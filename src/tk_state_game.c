// Todo: move data specific for this state from game state to here
struct state_game_data
{
    struct game_state* base;
};

b32 collision_detect_circle_circle(struct rigid_body* a, struct rigid_body* b,
    struct contact* contact)
{
    b32 result = false;

    // Todo: add early escapes
    // - amount of movement is less distance between circles minus radii

    // Reduce the velocity of b from the velocity of a
    struct v2 a_vel = v2_sub(a->move_delta, b->move_delta);

    // Calculate velocity direction
    struct v2 a_dir = v2_normalize(a_vel);

    // Calculate vector from a to b
    struct v2 ab = v2_direction(a->position, b->position);

    // There cannot be collision if the circles are moving to different
    // directions
    if (v2_dot(ab, a_dir) > 0.0f)
    {
        // Calculate closest point to b on line in velocity direction
        struct v2 d_pos = v2_add(a->position, v2_mul_f32(a_dir,
            v2_dot(ab, a_dir)));

        // Calculate velocity
        f32 a_len = v2_length(a_vel);

        // Calculate distance from circle a to d
        f32 ad_len = v2_distance(a->position, d_pos);

        // Calculate squared distance from b to d
        f32 bd_len = v2_distance_squared(b->position, d_pos);

        // Calculate total squared radii of a and b
        f32 rad_total = f32_square(a->radius + b->radius);

        // There cannot be collision if the length between b and d is equal or
        // greater than the total radii
        if (bd_len < rad_total)
        {
            // Calculate length from c to d
            f32 cd_len = f32_sqrt(rad_total - bd_len);

            // Calculate length from a to c
            f32 ac_len = ad_len - cd_len;

            // Collision occurs if the length from a to c is less than velocity
            if (ac_len < a_len)
            {
                result = true;

                // Store contact position
                if (contact)
                {
                    contact->position = v2_add(a->position,
                        v2_mul_f32(a_dir, ac_len));
                    contact->t = ac_len / a_len;
                    contact->a = a;
                    contact->b = b;
                }
            }
        }
    }

    return result;
}

struct v2 get_closest_point_on_line_segment(struct v2 point, struct v2 start,
    struct v2 end)
{
    struct v2 result = { 0.0f };
    struct v2 d = v2_normalize(v2_direction(start, end));
    struct v2 s_to_p = v2_sub(point, start);

    result = v2_add(start, v2_mul_f32(d, v2_dot(s_to_p, d)));

    f32 length_segment = v2_distance(start, end);
    f32 distance_start_closest = v2_distance(start, result);
    f32 distance_end_closest = v2_distance(end, result);

    if (distance_end_closest > length_segment ||
        distance_start_closest > length_segment)
    {
        if (distance_end_closest > distance_start_closest)
        {
            result = start;
        }
        else
        {
            result = end;
        }
    }

    return result;
}

struct v2 get_closest_point_on_line(struct v2 point, struct v2 start,
    struct v2 end)
{
    struct v2 result = { 0.0f };
    struct v2 d = v2_normalize(v2_direction(start, end));
    struct v2 s_to_p = v2_sub(point, start);

    result = v2_add(start, v2_mul_f32(d, v2_dot(s_to_p, d)));

    return result;
}

f32 get_distance_to_closest_point_on_line_segment(struct v2 point,
    struct line_segment line)
{
    f32 result = 0.0f;

    result = v2_distance(get_closest_point_on_line_segment(point, line.start,
        line.end), point);

    return result;
}

b32 collision_detect_circle_line(struct rigid_body* a, struct line_segment* b,
    struct contact* contact)
{
    b32 result = false;

    struct line_segment line = {
        a->position,
        v2_add(a->position, a->move_delta)
    };

    f32 r = a->radius;

    if (intersect_line_segment_to_line_segment(line, *b, NULL) ||
        get_distance_to_closest_point_on_line_segment(line.end, *b) < r ||
        get_distance_to_closest_point_on_line_segment(b->start, line) < r ||
        get_distance_to_closest_point_on_line_segment(b->end, line) < r )
    {
        struct v2 line_intersection = { 0 };

        intersect_line_to_line(line, *b, &line_intersection);

        struct v2 closest = get_closest_point_on_line(a->position, b->start,
            b->end);

        f32 distance_to_closest = v2_distance(a->position, closest);
        f32 distance_to_intersection = v2_distance(a->position,
            line_intersection);
        f32 distance_to_contact =
            a->radius * (distance_to_intersection / distance_to_closest);

        struct v2 i_to_a = v2_normalize(v2_direction(line_intersection,
            a->position));

        struct v2 c = v2_add(line_intersection, v2_mul_f32(i_to_a,
            distance_to_contact));

        contact->t = v2_distance(c, a->position) / v2_length(a->move_delta);
        contact->position = get_closest_point_on_line_segment(c, b->start,
            b->end);

        // Todo: collisions with corners are not perfect, the circle may get
        // stuck

        result = true;
    }

    return result;
}

u32 body_collisions_check(struct rigid_body bodies[], u32 num_bodies,
    struct contact contacts[], struct line_segment lines[], u32 num_lines)
{
    u32 result = 0;

    // Todo: this looks horrible, refactor and clean
    for (u32 i = 0; i < num_bodies; i++)
    {
        struct rigid_body* body = &bodies[i];

        if (!body->alive || body->bullet)
        {
            continue;
        }

        if (result < MAX_CONTACTS)
        {
            if (i < num_bodies - 1)
            {
                // Check collisions against other bodies
                for (u32 j = i + 1; j < num_bodies; j++)
                {
                    // Todo: we don't need to check the collisions if neither
                    // body is moving
                    struct rigid_body* other = &bodies[j];
                    struct contact contact = { 0 };

                    if (!other->alive || other->bullet)
                    {
                        continue;
                    }

                    if (collision_detect_circle_circle(body, other, &contact))
                    {
                        LOG("COLLISION: body %d and body %d\n", i, j);

                        if (body->trigger || other->trigger)
                        {
                            // Todo: implement collision callbacks
                            continue;
                        }

                        if ((!body->contact ||
                            contact.t < body->contact->t) &&
                            (!other->contact || contact.t < other->contact->t))
                        {
                            if (!body->contact && !other->contact)
                            {
                                body->contact = &contacts[result++];
                                other->contact = body->contact;
                            }
                            else
                            {
                                if (body->contact)
                                {
                                    if (body->contact->a &&
                                        body->contact->a != body)
                                    {
                                        body->contact->a->contact = NULL;
                                    }

                                    if (body->contact->b &&
                                        body->contact->b != body)
                                    {
                                        body->contact->b->contact = NULL;
                                    }

                                    other->contact = body->contact;
                                }
                                else
                                {
                                    if (other->contact->a &&
                                        other->contact->a != other)
                                    {
                                        other->contact->a->contact = NULL;
                                    }

                                    if (other->contact->b &&
                                        other->contact->b != other)
                                    {
                                        other->contact->b->contact = NULL;
                                    }

                                    body->contact = other->contact;
                                }
                            }

                            *body->contact = contact;
                        }
                    }
                }
            }

            // Todo: we don't need to check these collisions if the circle
            // is not moving

            // Check collisions against static walls
            for (u32 j = 0; j < num_lines; j++)
            {
                struct line_segment* other = &lines[j];
                struct contact contact = { 0 };

                if (collision_detect_circle_line(body, other, &contact))
                {
                    contact.a = body;
                    contact.b = NULL;
                    contact.line = other;

                    // Todo: HERE RECURSIVELY CHCECK THAT EACH CONTACT IS
                    // NULLIFIED FOR ALL PARTIES
                    LOG("COLLISION: body %d and line %d\n", i, j);
                    if (!body->contact || contact.t < body->contact->t)
                    {
                        if (body->contact)
                        {
                            if (body->contact->a &&
                                body->contact->a != body)
                            {
                                body->contact->a->contact = NULL;
                            }

                            if (body->contact->b &&
                                body->contact->b != body)
                            {
                                body->contact->b->contact = NULL;
                            }
                        }
                        else
                        {
                            body->contact = &contacts[result++];
                        }

                        *body->contact = contact;
                    }
                }
            }
        }
        else
        {
            LOG("Maximum number of contacts reached!\n");
            break;
        }
    }

    return result;
}

void body_collisions_resolve(struct contact contacts[], u32 num_contacts,
    f32 dt)
{
    for (u32 i = 0; i < num_contacts; i++)
    {
        struct contact* contact = &contacts[i];
        // Todo: this is a bit of haxy way to move the remaining (1 - t)
        // with a new velocity
        f32 t_remaining = 1.0f - contact->t;

        struct rigid_body* a = contact->a;
        struct rigid_body* b = contact->b;
        struct line_segment* line = contact->line;

        // Circle - circle
        if (a && b)
        {
            // Todo: when circle cannot move, if it's in a corner for example,
            // it should be regarded as static!

            // Elastic collisions
            // Calculate new velocities
            struct v2 n = v2_normalize(v2_direction(b->position, a->position));

            f32 a1 = v2_dot(a->velocity, n);
            f32 a2 = v2_dot(b->velocity, n);

            f32 p = (2.0f * (a1 - a2)) / (a->mass + b->mass);

            a->velocity.x = a->velocity.x - p * b->mass * n.x;
            a->velocity.y = a->velocity.y - p * b->mass * n.y;

            b->velocity.x = b->velocity.x + p * a->mass * n.x;
            b->velocity.y = b->velocity.y + p * a->mass * n.y;

            a->move_delta = v2_mul_f32(a->move_delta, contact->t);
            a->position = v2_add(a->position, a->move_delta);
            a->move_delta = v2_mul_f32(a->velocity, t_remaining * dt);

            b->move_delta = v2_mul_f32(b->move_delta, contact->t);
            b->position = v2_add(b->position, b->move_delta);
            b->move_delta = v2_mul_f32(b->velocity, t_remaining * dt);
        }
        // Circle - line
        else if (a && line)
        {
            a->position = v2_add(a->position,
                v2_mul_f32(a->move_delta, contact->t));

            struct v2 n = v2_normalize(v2_direction(a->position,
                contact->position));

            f32 vdot_a = v2_dot(a->velocity, n);

            a->velocity = v2_sub(a->velocity, v2_mul_f32(n, vdot_a));

            f32 mvdot_a = v2_dot(a->move_delta, n);

            a->move_delta = v2_mul_f32(
                v2_sub(a->move_delta, v2_mul_f32(n, mvdot_a)), t_remaining);

        }
        // No collision??
        else
        {
            LOG("Error: no collidees in collision?\n");
        }

        if (contact->a)
        {
            contact->a->contact = NULL;
        }

        if (contact->b)
        {
            contact->b->contact = NULL;
        }
    }
}

void body_velocities_update(struct rigid_body bodies[], u32 num_bodies, f32 dt)
{
    for (u32 i = 0; i < num_bodies; i++)
    {
        struct rigid_body* body = &bodies[i];

        if (body->alive)
        {
            body->acceleration.x += -body->velocity.x * body->friction;
            body->acceleration.y += -body->velocity.y * body->friction;

            body->move_delta.x = 0.5f * body->acceleration.x * f32_square(dt) +
                body->velocity.x * dt;
            body->move_delta.y = 0.5f * body->acceleration.y * f32_square(dt) +
                body->velocity.y * dt;

            body->velocity.x = body->velocity.x + body->acceleration.x * dt;
            body->velocity.y = body->velocity.y + body->acceleration.y * dt;
        }
    }
}

void body_positions_update(struct rigid_body bodies[], u32 num_bodies)
{
    for (u32 i = 0; i < num_bodies; i++)
    {
        struct rigid_body* body = &bodies[i];

        if (body->alive)
        {
            body->position.x += body->move_delta.x;
            body->position.y += body->move_delta.y;
        }
    }
}

void state_game_init(void* data)
{
    struct state_game_data* state = (struct state_game_data*)data;
    struct game_state* game = state->base;

    cube_renderer_init(&game->cube_renderer, game->shader_cube,
        game->texture_cube);
    sprite_renderer_init(&game->sprite_renderer,
        game->shader_sprite, game->texture_sprite);
    particle_renderer_init(&game->particle_renderer,
        game->shader_particle, game->texture_particle);

    struct particle_emitter_config config;
    config.position = (struct v3){ 3.0f, 3.0f, 0.125f };
    config.type = PARTICLE_EMITTER_CIRCLE;
    config.permanent = true;
    config.spawn_radius_min = 0.125f;
    config.spawn_radius_max = 0.25f;
    config.move_away_from_spawn = true;
    config.rate = 0.015f;
    config.max_particles = 256;
    config.velocity_min = 0.425f;
    config.velocity_max = 1.75f;
    config.velocity_angular_min = -2.5f;
    config.velocity_angular_max = 2.5f;
    config.scale_start = 0.05;
    config.scale_end = 0.15f;
    config.color_start = colors[WHITE];
    config.color_end = colors[RED];
    config.opacity_start = 1.0f;
    config.opacity_end = 0.0f;
    config.time_min = 0.5f;
    config.time_max = 1.5f;
    config.texture = GFX_STAR_FILLED;
    config.direction_min = 0.0f;
    config.direction_max = F64_PI * 2.0f;
    config.lifetime = INDEFINITE;

    particle_emitter_create(&game->particle_system, &config, false);

    config.position = (struct v3){ 14.0f, 15.0f, 0.125f };
    config.type = PARTICLE_EMITTER_CIRCLE;
    config.spawn_radius_min = 0.125f;
    config.spawn_radius_max = 0.25f;
    config.move_away_from_spawn = true;
    config.rate = 0.015f;
    config.max_particles = 256;
    config.velocity_min = 0.425f;
    config.velocity_max = 1.75f;
    config.velocity_angular_min = -2.5f;
    config.velocity_angular_max = 2.5f;
    config.scale_start = 0.05;
    config.scale_end = 0.15f;
    config.color_start = colors[WHITE];
    config.color_end = colors[RED];
    config.opacity_start = 1.0f;
    config.opacity_end = 0.0f;
    config.time_min = 0.5f;
    config.time_max = 1.5f;
    config.texture = GFX_STAR_FILLED;
    config.direction_min = 0.0f;
    config.direction_max = F64_PI * 2.0f;
    config.lifetime = 1.0f;

    particle_emitter_create(&game->particle_system, &config, true);

    struct camera* camera = &game->camera;
    camera->perspective = m4_perspective(60.0f,
        (f32)camera->screen_width / (f32)camera->screen_height, 0.1f, 15.0f);
    camera->perspective_inverse = m4_inverse(camera->perspective);
    camera->ortho = m4_orthographic(0.0f, camera->screen_width, 0.0f,
        camera->screen_height, 0.0f, 1.0f);
    camera->ortho_inverse = m4_inverse(camera->ortho);

    game->render_debug = false;

    u32 num_colors = sizeof(colors) / sizeof(struct v4);

    for (u32 i = 0; i < num_colors; i++)
    {
        cube_renderer_color_add(&game->cube_renderer, colors[i]);
    }

    game->level_current = 1;

    level_mask_init(&game->level_mask);
    level_init(game);

    api.gl.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

void state_game_update(void* data, struct game_input* input, f32 step)
{
    struct state_game_data* state = (struct state_game_data*)data;
    struct game_state* game = state->base;

    game->render_debug = input->enable_debug_rendering;

    if (game->level_clear_notify <= 0.0f)
    {
        game->cube_renderer.num_cubes = 0;
        game->sprite_renderer.num_sprites = 0;
        game->particle_renderer.num_particles = 0;

        player_update(game, input, step);
        enemies_update(game, input, step);
        bullets_update(game, input, step);
        items_update(&game->item_pool, &game->player, input, step);
        particle_lines_update(game, input, step);
        particle_system_update(&game->particle_system, step);

        struct physics_world* world = &game->world;

        body_velocities_update(world->bodies, MAX_BODIES, step);

        u32 max_iterations = 10;

        for (u32 i = 0; i < max_iterations; i++)
        {
            // LOG("Checking collisions, iteration %d\n", i + 1);

            // Todo: sometimes, for some reasons, a collision
            // between two objects happens again in the following
            // iteration. This shouldn't occur as the collision
            // is already resolved and the velocities and positions
            // are updated! Investigate

            // Todo: sometimes collisions are handled poorly
            // when multiple bodies are touching...?

            u32 num_contacts = body_collisions_check(world->bodies, MAX_BODIES,
                world->contacts, world->walls, world->num_walls);

            if (num_contacts)
            {
                body_collisions_resolve(world->contacts, num_contacts, step);
            }
            else
            {
                // LOG("No contacts!\n");
                break;
            }
        }

        body_positions_update(game->world.bodies, MAX_BODIES);
    }

    struct v2 start_min = v2_sub_f32(game->level.start_pos, 2.0f);
    struct v2 start_max = v2_add_f32(game->level.start_pos, 2.0f);
    struct v2 plr_pos = game->player.body->position;

    game->player_in_start_room = plr_pos.x > start_min.x &&
        plr_pos.x < start_max.x && plr_pos.y > start_min.y &&
        plr_pos.y < start_max.y;

    struct camera* camera = &game->camera;
    struct mouse* mouse = &game->mouse;

    if (game->level_clear_notify > 0.0f)
    {
        camera->target.x = game->level.start_pos.x;
        camera->target.y = game->level.start_pos.y + 1.0f;
        camera->target.z = 3.75f;

        game->level_clear_notify -= step;
    }
    else if (game->player_in_start_room)
    {
        camera->target.xy = game->level.start_pos;
        camera->target.z = 3.75f;
    }
    else
    {
        f32 distance_to_activate = 0.0f;

        struct v2 direction_to_mouse = v2_normalize(v2_direction(
            game->player.body->position, mouse->world));

        struct v2 target_pos = v2_average(mouse->world,
            game->player.body->position);

        f32 distance_to_target = v2_distance(target_pos,
            game->player.body->position);

        distance_to_target -= distance_to_activate;

        struct v2 target = game->player.body->position;

        if (distance_to_target > 0)
        {
            target.x = game->player.body->position.x +
                direction_to_mouse.x * distance_to_target;
            target.y = game->player.body->position.y +
                direction_to_mouse.y * distance_to_target;
        }

        camera->target.xy = target;
        camera->target.z = 10.0f;
    }

    {
        struct v3 dir = v3_direction(camera->position,
            camera->target);

        camera->position.x += dir.x * CAMERA_ACCELERATION * step;
        camera->position.y += dir.y * CAMERA_ACCELERATION * step;
        camera->position.z += dir.z * CAMERA_ACCELERATION * step;

        struct v3 up = { 0.0f, 1.0f, 0.0f };

        camera->view = m4_look_at(camera->position,
            (struct v3) { camera->position.xy, 0.0f }, up);
        camera->view_inverse = m4_inverse(camera->view);
    }

    mouse->world = calculate_world_pos((f32)input->mouse_x,
        (f32)input->mouse_y, camera);

    collision_map_dynamic_calculate(&game->cols, &game->player, game->enemies,
        game->num_enemies);

    game->num_gun_shots = 0;

    if (game->level_change)
    {
        game->level_cleared = false;
        game->level_change = false;
        game->accumulator = 0;
        game->level_current++;
        level_init(game);
    }
}

void state_game_render(void* data)
{
    struct state_game_data* state = (struct state_game_data*)data;
    struct game_state* game = state->base;
    struct camera* camera = &game->camera;

    if (game->level_cleared)
    {
        cube_data_color_update(&game->level.elevator_light, LIME);
    }

    level_render(&game->level, &game->sprite_renderer, &game->cube_renderer,
        camera->perspective, camera->view);
    player_render(game);
    enemies_render(game);
    items_render(&game->item_pool, &game->cube_renderer);
    particle_system_render(&game->particle_system, &game->particle_renderer);

    particle_renderer_sort(&game->particle_renderer);

    cube_renderer_flush(&game->cube_renderer, &camera->view,
        &camera->perspective);
    sprite_renderer_flush(&game->sprite_renderer, &camera->view,
        &camera->perspective);
    particle_renderer_flush(&game->particle_renderer, &camera->view,
        &camera->perspective);

    particle_lines_render(game);

    if (game->render_debug)
    {
        collision_map_render(game, game->cols.statics, game->cols.num_statics);
    }
}

struct state_interface state_game_create(struct game_state* state)
{
    struct state_interface result = { 0 };
    result.init = state_game_init;
    result.update = state_game_update;
    result.render = state_game_render;
    result.data = stack_alloc(&state->stack_permanent,
        sizeof(struct state_game_data));

    memory_set(result.data, sizeof(struct state_game_data), 0);

    struct game_state** base = (struct game_state**)result.data;

    *base = state;

    return result;
}
