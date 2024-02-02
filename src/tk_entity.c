
void bullet_trail_create(struct game_state* state, struct v2 start,
    struct v2 end, struct v4 color_start, struct v4 color_end, f32 time)
{
    if (++state->free_bullet_trail == MAX_BULLET_TRAILS)
    {
        state->free_bullet_trail = 0;
    }

    struct bullet_trail* particle =
        &state->bullet_trails[state->free_bullet_trail];

    *particle = (struct bullet_trail){ 0 };
    particle->start = start;
    particle->end = end;
    particle->color_start = color_start;
    particle->color_end = color_end;
    particle->alive = true;
    particle->time_start = time;
    particle->time_current = particle->time_start;
}

void bullet_trails_update(struct game_state* state, struct game_input* input,
    f32 dt)
{
    for (u32 i = 0; i < MAX_BULLET_TRAILS; i++)
    {
        struct bullet_trail* particle = &state->bullet_trails[i];

        if (particle->alive)
        {
            if (particle->time_current <= 0.0f)
            {
                particle->alive = false;
            }
            else
            {
                particle->time_current -= dt;
                f32 t = 1.0f - particle->time_current / particle->time_start;

                particle->color_current = particle->color_start;

                particle->color_current.r = particle->color_start.r +
                    (particle->color_end.r - particle->color_start.r) * t;
                particle->color_current.g = particle->color_start.g +
                    (particle->color_end.g - particle->color_start.g) * t;
                particle->color_current.b = particle->color_start.b +
                    (particle->color_end.b - particle->color_start.b) * t;
                particle->color_current.a = particle->color_start.a +
                    (particle->color_end.a - particle->color_start.a) * t;
            }
        }
    }
}

void bullet_trails_render(struct game_state* state, struct camera* camera)
{
    // Todo: hax, but this fixes transparent blending
    f32 depth = 0.0f;

    struct mesh_render_info info = { 0 };
    info.color = colors[WHITE];
    info.mesh = &state->floor;
    info.texture = state->texture_tileset;
    info.shader = state->shader_simple;

    struct m4 vp = m4_mul_m4(camera->view, camera->projection);

    for (u32 i = 0; i < MAX_BULLET_TRAILS; i++)
    {
        struct bullet_trail* particle = &state->bullet_trails[i];

        if (particle->alive)
        {
            depth += 0.0001f;

            info.color = particle->color_current;

            line_render(&info, particle->start, particle->end,
                PLAYER_RADIUS + depth, PROJECTILE_RADIUS * 0.5f, vp);
        }
    }
}

struct item* item_create(struct physics_world* world,
    struct object_pool* item_pool, struct v2 position, u32 type)
{
    struct item* result = NULL;

    if (type > ITEM_NONE && type < ITEM_COUNT)
    {
        result = object_pool_get_next(item_pool);

        if (!result->alive)
        {
            entity_add_body(&result->header, world);
        }

        struct rigid_body* body = result->header.body;
        body->type = RIGID_BODY_DYNAMIC;
        body->position = position;
        body->trigger = true;
        result->alive = ITEM_ALIVE_TIME;
        result->type = type;
        body_add_circle_collider(body, v2_zero, ITEM_RADIUS, COLLISION_ITEM,
            COLLISION_PLAYER);

        result->cube.faces[0].texture = 4 + type - 1;

        u32 color = OLIVE;

        switch (type)
        {
            case ITEM_HEALTH:
            {
                color = WHITE;
            } break;
            case ITEM_WEAPON_LEVEL_UP:
            {
                color = LIME;
            } break;
        }

        for (u32 i = 0; i < 6; i++)
        {
            result->cube.faces[i].color = color;
        }
    }

    return result;
}

void gun_shot_register(struct game_state* state, struct v2 position, f32 volume)
{
    if (state->num_gun_shots < MAX_GUN_SHOTS)
    {
        struct gun_shot* shot = &state->gun_shots[state->num_gun_shots++];

        shot->position = position;
        shot->volume = volume;
    }
}

void enemies_update(struct game_state* state, struct game_input* input, f32 dt)
{
    // Todo: clean this function...
    for (u32 i = 0; i < MAX_ENEMIES; i++)
    {
        struct enemy* enemy = &state->enemies[i];

        if (enemy->alive)
        {
            struct rigid_body* body = enemy->header.body;

            if (enemy->health < 0.0f)
            {
                enemy->alive = false;

                rigid_body_free(body);

                {
                    u32 count = 0;

                    for (u32 i = 0; i < state->num_enemies; i++)
                    {
                        count += state->enemies[i].alive;
                    }

                    if (count)
                    {
                        LOG("Enemies left %u of %u\n", count,
                            state->num_enemies);
                    }
                    else
                    {
                        state->level_cleared = true;
                        state->level_clear_notify = 2.5f;
                    }
                }

                item_create(&state->world, &state->item_pool,
                    body->position, ITEM_HEALTH + enemy->weapon.type);

                u32 random_item_count = u32_random(0, 3);

                for (u32 j = 0; j < random_item_count; j++)
                {
                    struct v2 position = body->position;

                    f32 offset_max = 0.25f;
                    f32 offset_x = f32_random(-offset_max, offset_max);
                    f32 offset_y = f32_random(-offset_max, offset_max);

                    position.x += offset_x;
                    position.y += offset_y;

                    u32 random_item_type = u32_random(0, 2);
                    random_item_type =
                        random_item_type ? ITEM_HEALTH : ITEM_WEAPON_LEVEL_UP;

                    item_create(&state->world, &state->item_pool, position,
                        random_item_type);
                }

                continue;
            }

            enemy->player_in_view = enemy_sees_player(&state->world, enemy,
                &state->player);

            enemy->gun_shot_heard = enemy_hears_gun_shot(state->gun_shots,
                state->num_gun_shots, enemy);

            if (enemy->player_in_view)
            {
                enemy->player_last_seen_position =
                    state->player.header.body->position;
                enemy->player_last_seen_direction = v2_normalize(
                    body->velocity);

                if (enemy->state != ENEMY_STATE_SHOOT &&
                    enemy->state != ENEMY_STATE_SLEEP)
                {
                    enemy_state_transition(enemy,
                        ENEMY_STATE_REACT_TO_PLAYER_SEEN, &state->level,
                        &state->cols);
                }
            }
            else if (enemy->gun_shot_heard)
            {
                // Todo: enemy should react only to the closest source of noise
                if (enemy->state == ENEMY_STATE_SLEEP)
                {
                    enemy_state_transition(enemy,
                        ENEMY_STATE_LOOK_FOR_PLAYER, &state->level,
                        &state->cols);
                }
                else if (enemy->state == ENEMY_STATE_RUSH_TO_TARGET)
                {
                    enemy->target = enemy->gun_shot_position;
                    enemy_calculate_path_to_target(&state->cols, &state->level,
                        enemy);
                }
                else if (enemy->state != ENEMY_STATE_SHOOT)
                {
                    enemy_state_transition(enemy,
                        ENEMY_STATE_REACT_TO_GUN_SHOT, &state->level,
                        &state->cols);
                }
            }
            else if (enemy->got_hit)
            {
                // Todo: react only if the source of bullet is not visible?
                if (enemy->state == ENEMY_STATE_SLEEP)
                {
                    enemy_state_transition(enemy,
                        ENEMY_STATE_LOOK_FOR_PLAYER, &state->level,
                        &state->cols);
                }
                else if (enemy->state != ENEMY_STATE_SHOOT)
                {
                    enemy_state_transition(enemy,
                        ENEMY_STATE_REACT_TO_BEING_SHOT_AT, &state->level,
                        &state->cols);
                }

                enemy->got_hit = false;
            }

            struct level* level = &state->level;
            struct collision_map* cols = &state->cols;

            switch (enemy->state)
            {
                case ENEMY_STATE_SLEEP:
                    break;
                case ENEMY_STATE_REACT_TO_PLAYER_SEEN:
                    enemy_state_react_to_player_seen_update(enemy, level, cols);
                    break;
                case ENEMY_STATE_REACT_TO_GUN_SHOT:
                    enemy_state_react_to_gun_shot_update(enemy, level, cols);
                    break;
                case ENEMY_STATE_SHOOT:
                    enemy_state_shoot_update(enemy, level, cols,
                        &state->bullet_pool, &state->world);
                    break;
                case ENEMY_STATE_RUSH_TO_TARGET:
                    enemy_state_rush_to_target_update(enemy, level, cols);
                    break;
                case ENEMY_STATE_LOOK_AROUND:
                    enemy_state_look_around_update(enemy, level, cols);
                    break;
                case ENEMY_STATE_WANDER_AROUND:
                    enemy_state_wander_around_update(enemy, level, cols);
                    break;
                case ENEMY_STATE_REACT_TO_BEING_SHOT_AT:
                    enemy_state_react_to_being_shot_at_update(enemy, level,
                        cols);
                    break;
                case ENEMY_STATE_LOOK_FOR_PLAYER:
                    enemy_state_look_for_player_update(enemy, level,
                        cols);
                    break;
            }

            enemy->state_timer_finished = false;

            if (enemy->state_timer > 0.0f)
            {
                enemy->state_timer -= dt;

                if (enemy->state_timer < 0.0f)
                {
                    enemy->state_timer_finished = true;
                }
            }

            if (enemy->path_length)
            {
                struct v2 current = enemy->path[enemy->path_index];

                f32 distance_to_current = v2_distance(body->position,
                    current);
                f32 epsilon = 0.25f;

                if (distance_to_current < epsilon)
                {
                    if (++enemy->path_index >= enemy->path_length)
                    {
                        enemy->path_length = 0;
                        enemy->acceleration = 0.0f;
                    }
                    else
                    {
                        enemy_look_towards_position(enemy,
                            enemy->path[enemy->path_index]);
                    }
                }
                else
                {
                    struct v2 direction = v2_normalize(v2_direction(
                        body->position, current));

                    f32 length = v2_length(direction);

                    if (length > 1.0f)
                    {
                        direction.x /= length;
                        direction.y /= length;
                    }

                    enemy->direction_move = direction;
                    enemy->direction_aim = direction;
                }
            }

            body->acceleration = v2_mul_f32(enemy->direction_move,
                enemy->acceleration);

            if (enemy->turn_amount)
            {
                f32 speed = enemy_turn_speed_get(enemy) * dt;
                f32 remaining = MIN(f32_abs(enemy->turn_amount), speed);

                if (enemy->turn_amount > 0)
                {
                    body->angle -= remaining;
                    enemy->turn_amount = MAX(enemy->turn_amount - remaining, 0);
                }
                else
                {
                    body->angle += remaining;
                    enemy->turn_amount = MIN(enemy->turn_amount + remaining, 0);
                }
            }

            struct v2 eye = { PLAYER_RADIUS + 0.0001f, 0.0f };

            enemy->eye_position = v2_rotate(eye, body->angle);
            enemy->eye_position.x += body->position.x;
            enemy->eye_position.y += body->position.y;

            {
                f32 vision_cone_update_speed = 3.0f;
                f32 vision_cone_size_min = 0.25f;
                f32 vision_cone_size_max = 1.0f;

                if (enemy->player_in_view)
                {
                    enemy->vision_cone_size -= dt * vision_cone_update_speed;
                    enemy->vision_cone_size = MAX(enemy->vision_cone_size,
                        vision_cone_size_min);
                }
                else
                {
                    enemy->vision_cone_size += dt * vision_cone_update_speed;
                    enemy->vision_cone_size = MIN(enemy->vision_cone_size,
                        vision_cone_size_max);
                }
            }

            // Todo: clean this, maybe move somewhere else
            {
                struct weapon* weapon = &enemy->weapon;

                if (enemy->trigger_release > 0.0f)
                {
                    enemy->trigger_release -= dt;

                    if (enemy->trigger_release <= 0.0f)
                    {
                        weapon->fired = false;
                    }
                }

                if (weapon->last_shot > 0.0f)
                {
                    weapon->last_shot -= dt;
                }

                if (weapon->reloading)
                {
                    if (weapon->last_shot <= 0.0f)
                    {
                        weapon->ammo++;

                        if (weapon->ammo == weapon->ammo_max)
                        {
                            weapon->reloading = false;
                        }
                        else
                        {
                            weapon->last_shot = weapon->reload_time;
                        }
                    }
                }
            }
        }
    }
}

void line_of_sight_render(struct game_state* state, struct camera* camera,
    struct v2 position, f32 angle_start, f32 angle_max, struct v4 color,
    b32 render_lines)
{
    struct v2 corners[MAX_WALL_CORNERS] = { 0 };
    u32 num_corners = 0;

    struct ray_cast_collision collision = { 0 };
    struct v2 dir_left = v2_direction_from_angle(angle_start + angle_max);
    struct v2 dir_right = v2_direction_from_angle(angle_start - angle_max);

    collision = world_raycast(&state->world, position, dir_right,
        COLLISION_ALL);
    corners[num_corners++] = collision.position;

    collision = world_raycast(&state->world, position, dir_left,
        COLLISION_ALL);
    corners[num_corners++] = collision.position;

    for (u32 i = 0; i < state->cols.num_dynamics &&
        num_corners < MAX_WALL_CORNERS; i++)
    {
        struct v2 temp = state->cols.dynamics[i].start;

        if (direction_in_range(v2_direction(position, temp), dir_left,
            dir_right))
        {
            corners[num_corners++] = temp;
        }
    }

    for (u32 i = 0; i < state->num_wall_corners &&
        num_corners < MAX_WALL_CORNERS; i++)
    {
        struct v2 temp = state->wall_corners[i];

        if (direction_in_range(v2_direction(position, temp), dir_left,
            dir_right))
        {
            corners[num_corners++] = temp;
        }
    }

    struct v2 finals[MAX_WALL_CORNERS] = { 0 };
    u32 num_finals = 0;

    struct mesh_render_info info = { 0 };
    info.color = colors[RED];
    info.mesh = &state->floor;
    info.texture = state->texture_tileset;
    info.shader = state->shader_simple;

    struct mesh_render_info info_triangle = { 0 };
    info_triangle.color = color;
    info_triangle.shader = state->shader_simple;

    struct m4 vp = m4_mul_m4(camera->view, camera->projection);

    for (u32 i = 0; i < num_corners; i++)
    {
        struct v2 direction = v2_direction(position, corners[i]);
        f32 angle = f32_atan(direction.y, direction.x);
        f32 t = 0.001f;

        collision = world_raycast(&state->world, position, direction,
            COLLISION_ALL);

        if (!collision.body)
        {
            LOG("Raycast returned zero! Pos: %f %f Dir: %f %f Len: %f\n",
                position.x, position.y, direction.x, direction.y,
                v2_length(direction));
        }

        struct ray_cast_collision collision_temp = { 0 };
        finals[num_finals++] = collision.position;

        if (render_lines)
        {
            line_render(&info, position, collision.position, 0.005f, 0.005f,
                vp);
        }

        collision_temp = world_raycast(&state->world, position,
            v2_direction_from_angle(angle + t), COLLISION_ALL);


        if (!collision_temp.body)
        {
            LOG("Raycast returned zero!\n");
        }

        if (v2_distance(collision_temp.position, collision.position) > 0.001f)
        {
            if (render_lines)
            {
                line_render(&info, position, collision_temp.position, 0.005f,
                    0.005f, vp);
            }

            finals[num_finals++] = collision_temp.position;
        }

        collision_temp = world_raycast(&state->world, position,
            v2_direction_from_angle(angle - t), COLLISION_ALL);

        if (!collision_temp.body)
        {
            LOG("Raycast returned zero!\n");
        }

        if (v2_distance(collision_temp.position, collision.position) > 0.001f)
        {
            if (render_lines)
            {
                line_render(&info, position, collision_temp.position, 0.005f,
                    0.005f, vp);
            }

            finals[num_finals++] = collision_temp.position;
        }
    }

    reorder_corners_ccw(finals, num_finals, position);

    for (u32 i = 0; i < num_finals-1; i++)
    {
        triangle_render(&info_triangle, position, finals[i], finals[i+1],
            0.0025f, vp);
    }
}

void enemies_render(struct game_state* state, struct camera* camera_game,
    struct camera* camera_gui)
{
    for (u32 i = 0; i < MAX_ENEMIES; i++)
    {
        struct enemy* enemy = &state->enemies[i];

        if (enemy->alive)
        {
            switch (enemy->state)
            {
                case ENEMY_STATE_SHOOT:
                {
                    enemy->cube.faces[0].texture = 15;
                } break;
                case ENEMY_STATE_WANDER_AROUND:
                case ENEMY_STATE_LOOK_AROUND:
                {
                    enemy->cube.faces[0].texture = 13;
                } break;
                case ENEMY_STATE_SLEEP:
                {
                    enemy->cube.faces[0].texture = 12;
                } break;
                case ENEMY_STATE_RUSH_TO_TARGET:
                case ENEMY_STATE_REACT_TO_PLAYER_SEEN:
                case ENEMY_STATE_REACT_TO_GUN_SHOT:
                case ENEMY_STATE_REACT_TO_BEING_SHOT_AT:
                case ENEMY_STATE_LOOK_FOR_PLAYER:
                default:
                {
                    enemy->cube.faces[0].texture = 14;
                } break;
            }

            struct rigid_body* body = enemy->header.body;

            struct m4 transform = m4_translate(body->position.x,
                body->position.y, PLAYER_RADIUS);
            struct m4 rotation = m4_rotate_z(body->angle);
            struct m4 scale = m4_scale_xyz(PLAYER_RADIUS, PLAYER_RADIUS, 0.25f);
            struct m4 model = m4_mul_m4(scale, rotation);
            model = m4_mul_m4(model, transform);

            enemy->cube.model = model;

            cube_renderer_add(&state->cube_renderer, &enemy->cube);

            // if (state->render_debug)
            // {
            //     // Todo: calculate temporarily here
            //     struct v2 eye = { PLAYER_RADIUS + 0.0001f, 0.0f };
            //     eye = v2_rotate(eye, body->angle);
            //     eye = v2_add(eye, body->position);

            //     line_of_sight_render(state, eye, body->angle,
            //         enemy->vision_cone_size * ENEMY_LINE_OF_SIGHT_HALF,
            //         enemy->player_in_view ? colors[PURPLE] : colors[TEAL],
            //         true);
            // }

            struct v2 screen_pos = camera_world_to_view_pos(camera_game,
                body->position.x, body->position.y + 0.5f, 0.5f);

            health_bar_render(&state->render_info_health_bar, camera_gui,
                screen_pos, enemy->health, ENEMY_HEALTH_MAX);
            ammo_bar_render(&state->render_info_ammo_bar, camera_gui,
                screen_pos, (f32)enemy->weapon.ammo,
                (f32)enemy->weapon.ammo_max);

#if 0
            // Render velocity vector
            if (state->render_debug)
            {
                f32 max_speed = 7.0f;
                f32 length = v2_length(body->velocity) / max_speed;
                f32 angle = f32_atan(body->velocity.x,
                    body->velocity.y);

                transform = m4_translate(
                    body->position.x + body->velocity.x /
                        max_speed,
                    body->position.y + body->velocity.y /
                        max_speed,
                    0.012f);

                rotation = m4_rotate_z(-angle);
                scale = m4_scale_xyz(0.05f, length, 0.01f);

                model = m4_mul_m4(scale, rotation);
                model = m4_mul_m4(model, transform);

                struct m4 mvp = m4_mul_m4(model, state->camera.view);
                mvp = m4_mul_m4(mvp, state->camera.perspective);

                mesh_render(&state->floor, &mvp, state->texture_tileset,
                    state->shader_simple, colors[GREY]);
            }

            // Render aim vector
            if (state->render_debug)
            {
                f32 length = 1.0f;
                f32 angle = f32_atan(enemy->direction_aim.x,
                    enemy->direction_aim.y);

                transform = m4_translate(
                    body->position.x + enemy->direction_aim.x / length,
                    body->position.y + enemy->direction_aim.y / length,
                    0.011f);

                rotation = m4_rotate_z(-angle);
                scale = m4_scale_xyz(0.025f, length, 0.01f);

                model = m4_mul_m4(scale, rotation);
                model = m4_mul_m4(model, transform);

                struct m4 mvp = m4_mul_m4(model, state->camera.view);
                mvp = m4_mul_m4(mvp, state->camera.perspective);

                mesh_render(&state->floor, &mvp, state->texture_tileset,
                    state->shader_simple, colors[YELLOW]);
            }

            // Render look vector
            if (state->render_debug)
            {
                f32 length = 1.0f;
                f32 angle = f32_atan(enemy->direction_look.x,
                    enemy->direction_look.y);

                transform = m4_translate(
                    body->position.x + enemy->direction_look.x / length,
                    body->position.y + enemy->direction_look.y / length,
                    0.010f);

                rotation = m4_rotate_z(-angle);
                scale = m4_scale_xyz(0.025f, length, 0.01f);

                model = m4_mul_m4(scale, rotation);
                model = m4_mul_m4(model, transform);

                struct m4 mvp = m4_mul_m4(model, state->camera.view);
                mvp = m4_mul_m4(mvp, state->camera.perspective);

                mesh_render(&state->floor, &mvp, state->texture_tileset,
                    state->shader_simple, colors[BLUE]);
            }

            // Render path
            if (state->render_debug)
            {
                if (enemy->path_length)
                {
                    struct v2 current = body->position;

                    for (u32 j = enemy->path_index; j < enemy->path_length; j++)
                    {
                        struct v2 next = enemy->path[j];

                        line_render(state, current, next, colors[i],
                            0.005f, 0.02f);

                        current = next;
                    }
                }
            }
#endif
        }
    }
}

void bullets_update(struct game_state* state, struct game_input* input, f32 dt)
{
    struct bullet* bullets = state->bullet_pool.data;

    for (u32 i = 0; i < state->bullet_pool.count; i++)
    {
        struct bullet* bullet = &bullets[i];

        if (bullet->alive)
        {
            struct rigid_body* body = bullet->header.body;
            // struct v2 move_delta =
            // {
            //     body->velocity.x * dt,
            //     body->velocity.y * dt
            // };

            // f32 distance_target = v2_length(move_delta);
            // u32 flags = COLLISION_WALL;

            // if (bullet->player_owned)
            // {
            //     flags |= COLLISION_ENEMY;
            // }
            // else
            // {
            //     flags |= COLLISION_PLAYER;
            // }

            // f32 distance = ray_cast_direction(&state->cols,
            //     body->position, v2_normalize(move_delta), NULL, flags);

            // f32 fraction = 1.0f;

            // if (distance < distance_target)
            // {
            //     fraction = distance / distance_target;
            // }

            // body->position.x += move_delta.x * fraction;
            // body->position.y += move_delta.y * fraction;

            if (false/*distance < distance_target*/)
            {
                rigid_body_free(body);

                bullet->alive = false;

                f32 angle = f32_atan(-body->velocity.y,
                    -body->velocity.x);
                f32 spread = 0.75f;
                f32 color = f32_random(0.25f, 0.75f);

                struct particle_emitter_config config;
                config.position.xy = body->position;
                config.position.z = 0.5f;
                config.permanent = false;
                config.type = PARTICLE_EMITTER_POINT;
                config.spawn_radius_min = 0.125f;
                config.spawn_radius_max = 0.25f;
                config.move_away_from_spawn = false;
                config.rate = 0.025f;
                config.max_particles = 5;
                config.velocity_min = 1.25f;
                config.velocity_max = 2.25f;
                config.velocity_angular_min = -2.5f;
                config.velocity_angular_max = 2.5f;
                config.scale_start = 0.0125;
                config.scale_end = 0.05f;
                config.color_start = (struct v4){ color, color, color, 1.0f };
                config.color_end = (struct v4){ .xyz = colors[GREY].xyz, 0.0f };
                config.opacity_start = 1.0f;
                config.opacity_end = 0.25f;
                config.time_min = 0.125f;
                config.time_max = 0.25f;
                config.texture = GFX_CIRCLE_FILLED;
                config.direction_min = angle - spread;
                config.direction_max = angle + spread;
                config.lifetime = 0.1f;

                f32 target_size = PLAYER_RADIUS + PROJECTILE_RADIUS;

                // Todo: we might not need to do this if the ray cast system
                // could differentiate whether the ray hit a wall or an entity
                if (bullet->player_owned)
                {
                    for (u32 j = 0; j < state->num_enemies; j++)
                    {
                        struct enemy* enemy = &state->enemies[j];
                        struct line_segment segments[4] = { 0 };

                        get_body_rectangle(body, target_size,
                            target_size, segments);

                        struct v2 corners[] =
                        {
                            segments[0].start,
                            segments[1].start,
                            segments[2].start,
                            segments[3].start
                        };

                        if (enemy->alive && collision_point_to_obb(
                            body->position, corners))
                        {
                            bullet->alive = false;
                            enemy->health -= bullet->damage;
                            enemy->got_hit = true;
                            enemy->hit_direction = v2_flip(v2_normalize(
                                body->velocity));

                            config.color_start = colors[RED];
                            config.color_end = colors[RED];

                            break;
                        }
                    }
                }
                else
                {
                    struct player* player = &state->player;
                    struct line_segment segments[4] = { 0 };

                    get_body_rectangle(player->header.body, target_size,
                        target_size, segments);

                    struct v2 corners[] =
                    {
                        segments[0].start,
                        segments[1].start,
                        segments[2].start,
                        segments[3].start
                    };

                    if (player->alive && collision_point_to_obb(
                        body->position, corners))
                    {
                        bullet->alive = false;
                        player->health -= bullet->damage;

                        config.color_start = colors[RED];
                        config.color_end = colors[RED];
                    }
                }

                particle_emitter_create(&state->particle_system, &config, true);
            }

            bullet_trail_create(state, bullet->start, body->position,
                (struct v4){ .xyz = bullet->color.xyz, 0.75f },
                (struct v4){ .xyz = bullet->color.xyz, 0.0f },
                0.45f);

            bullet->start = body->position;
        }
    }
}

void bullets_render(struct game_state* state, struct camera* camera)
{
    struct mesh_render_info info = { 0 };
    info.color = colors[WHITE];
    info.mesh = &state->sphere;
    info.texture = state->texture_sphere;
    info.shader = state->shader;

    struct bullet* bullets = state->bullet_pool.data;

    for (u32 i = 0; i < state->bullet_pool.count; i++)
    {
        struct bullet* bullet = &bullets[i];

        if (bullet->alive)
        {
            struct rigid_body* body = bullet->header.body;

            struct m4 transform = m4_translate(body->position.x,
                body->position.y, PLAYER_RADIUS);
            struct m4 rotation = m4_rotate_z(body->angle);
            struct m4 scale = m4_scale_all(PROJECTILE_RADIUS);

            struct m4 model = m4_mul_m4(scale, rotation);
            model = m4_mul_m4(model, transform);

            struct m4 mvp = m4_mul_m4(model, camera->view);
            mvp = m4_mul_m4(mvp, camera->projection);

            mesh_render(&info, &mvp);
        }
    }
}

void player_update(struct game_state* state, struct game_input* input, f32 dt)
{
    struct player* player = &state->player;

    player->alive = player->health > 0.0f;

    if (player->alive)
    {
        struct rigid_body* body = player->header.body;
        struct v2 direction = { 0.0f };

        struct v2 dir = v2_sub(state->mouse.world, body->position);
        dir = v2_normalize(dir);

        body->angle = f32_atan(dir.y, dir.x);

        if (input->move_left.key_down)
        {
            direction.x -= 1.0f;
        }

        if (input->move_right.key_down)
        {
            direction.x += 1.0f;
        }

        if (input->move_down.key_down)
        {
            direction.y -= 1.0f;
        }

        if (input->move_up.key_down)
        {
            direction.y += 1.0f;
        }

        direction = v2_normalize(direction);

        body->acceleration.x = direction.x * PLAYER_ACCELERATION;
        body->acceleration.y = direction.y * PLAYER_ACCELERATION;

        // Todo: this falls behind here as the position is updated after the
        // eye
        struct v2 eye = { PLAYER_RADIUS + 0.0001f, 0.0f };

        player->eye_position = v2_rotate(eye, body->angle);
        player->eye_position.x += body->position.x;
        player->eye_position.y += body->position.y;

        struct weapon* weapon = &player->weapon;

        if (weapon->last_shot > 0.0f)
        {
            weapon->last_shot -= dt;
        }

        if (player->item_picked)
        {
            switch (player->item_picked)
            {
                case ITEM_HEALTH:
                {
                    player->health += ITEAM_HEALTH_AMOUNT;
                    player->health = MIN(player->health, PLAYER_HEALTH_MAX);
                } break;
                case ITEM_PISTOL:
                case ITEM_MACHINEGUN:
                case ITEM_SHOTGUN:
                {
                    u32 weapon_type = player->item_picked - 1;

                    if (weapon->type != weapon_type)
                    {
                        *weapon = weapon_create(player->item_picked - 1);
                        weapon->ammo = 0;
                        weapon_reload(weapon, true);
                    }

                } break;
                case ITEM_WEAPON_LEVEL_UP:
                {
                    if (weapon_level_up(weapon))
                    {
                        weapon_reload(weapon, true);
                    }
                }
            }

            player->item_picked = ITEM_NONE;
        }

        weapon->direction = dir;
        weapon->position = body->position;
        weapon->velocity = body->velocity;

        if (key_times_pressed(&input->reload))
        {
            weapon_reload(weapon, false);
        }

        if (state->level_cleared)
        {
            if (key_times_pressed(&input->weapon_pick) &&
                tile_type_get(&state->level, body->position) ==
                TILE_START)
            {
                LOG("Changing level...\n");
                state->level_change = true;
            }
        }

        if (input->shoot.key_down && !state->player_in_start_room)
        {
            if (weapon_shoot(&state->bullet_pool, &state->world, weapon, true))
            {
                gun_shot_register(state, weapon->position, 10.0f);
            }
        }
        else
        {
            weapon->fired = false;
        }

        if (weapon->reloading)
        {
            if (weapon->last_shot <= 0.0f)
            {
                weapon->ammo++;

                if (weapon->ammo == weapon->ammo_max)
                {
                    weapon->reloading = false;
                }
                else
                {
                    weapon->last_shot = weapon->reload_time;
                }
            }
        }

        struct particle_emitter* emitter = &state->particle_system.emitters[0];

        // Todo: testing, remove when enough
        if (v2_length(body->velocity) > 1.0f)
        {
            struct particle_emitter_config* config = &emitter->config;
            config->position.xy = body->position;

            f32 angle = f32_atan(-body->velocity.y, -body->velocity.x);
            f32 spread = 0.45f;

            config->direction_min = angle - spread;
            config->direction_max = angle + spread;

            emitter->active = true;
        }
        else
        {
            emitter->active = false;
        }
    }
}

void player_render(struct game_state* state, struct camera* camera_game,
    struct camera* camera_gui)
{
    struct player* player = &state->player;

    if (player->alive)
    {
        struct rigid_body* body = player->header.body;

        struct m4 transform = m4_translate(body->position.x,
            body->position.y, PLAYER_RADIUS);
        struct m4 rotation = m4_rotate_z(body->angle);
        struct m4 scale = m4_scale_xyz(PLAYER_RADIUS, PLAYER_RADIUS, 0.25f);
        struct m4 model = m4_mul_m4(scale, rotation);
        model = m4_mul_m4(model, transform);

        player->cube.model = model;

        cube_renderer_add(&state->cube_renderer, &player->cube);

        if (state->render_debug)
        {
            struct v4 color = colors[LIME];
            color.a = 0.5f;

            // Todo: calculate temporarily here
            struct v2 eye = { PLAYER_RADIUS + 0.0001f, 0.0f };
            eye = v2_rotate(eye, body->angle);
            eye = v2_add(eye, body->position);

            line_of_sight_render(state, camera_game, eye, body->angle,
                ENEMY_LINE_OF_SIGHT_HALF, color, true);
        }

#if 0
        // Render velocity vector
        if (state->render_debug)
        {
            f32 max_speed = 7.0f;
            f32 length = v2_length(player->body->velocity) / max_speed;
            f32 angle = f32_atan(player->body->velocity.x,
                player->body->velocity.y);

            transform = m4_translate(
                player->body->position.x + player->body->velocity.x / max_speed,
                player->body->position.y + player->body->velocity.y / max_speed,
                0.01f);

            rotation = m4_rotate_z(-angle);
            scale = m4_scale_xyz(0.05f, length, 0.01f);

            model = m4_mul_m4(scale, rotation);
            model = m4_mul_m4(model, transform);

            struct m4 mvp = m4_mul_m4(model, state->camera.view);
            mvp = m4_mul_m4(mvp, state->camera.perspective);

            mesh_render(&state->floor, &mvp, state->texture_tileset,
                state->shader_simple, colors[GREY]);
        }

        // Render aim vector
        if (state->render_debug)
        {
            struct v2 vec =
            {
                state->mouse.world.x - player->body->position.x,
                (state->mouse.world.y - PLAYER_RADIUS) -
                    player->body->position.y
            };

            f32 length = v2_length(vec) * 0.5f;
            f32 angle = f32_atan(vec.x, vec.y);

            struct v2 direction = v2_normalize(vec);

            transform = m4_translate(
                player->body->position.x + direction.x * length,
                player->body->position.y + direction.y * length, PLAYER_RADIUS);
            rotation = m4_rotate_z(-angle);
            scale = m4_scale_xyz(0.01f, length, 0.01f);

            model = m4_mul_m4(scale, rotation);
            model = m4_mul_m4(model, transform);

            struct m4 mvp = m4_mul_m4(model, state->camera.view);
            mvp = m4_mul_m4(mvp, state->camera.perspective);

            mesh_render(&state->floor, &mvp, state->texture_tileset,
                state->shader_simple, colors[RED]);
        }
#endif
        struct v2 screen_pos = camera_world_to_view_pos(camera_game,
            body->position.x, body->position.y + 0.5f, 0.5f);

        struct weapon* weapon = &player->weapon;
        health_bar_render(&state->render_info_health_bar, camera_gui,
            screen_pos, player->health, PLAYER_HEALTH_MAX);
        ammo_bar_render(&state->render_info_ammo_bar, camera_gui,
            screen_pos, (f32)weapon->ammo, (f32)weapon->ammo_max);
        weapon_level_bar_render(&state->render_info_weapon_bar, camera_gui,
            screen_pos, weapon->level, WEAPON_LEVEL_MAX);
    }
}

void items_update(struct object_pool* item_pool, struct player* player,
    struct game_input* input, f32 dt)
{
    struct item* items = item_pool->data;

    for (u32 i = 0; i < item_pool->count; i++)
    {
        struct item* item = &items[i];

        if (item->alive)
        {
            struct rigid_body* body = item->header.body;

            body->angle -= (f32)F64_PI * dt;
            item->alive -= dt;

            if (item->alive < 0.0f)
            {
                item->alive = 0.0f;

                rigid_body_free(body);
            }
            else if (item->alive < ITEM_FLASH_TIME)
            {
                if (item->flash_timer <= 0.0f)
                {
                    item->flash_timer = ITEM_FLASH_SPEED;
                    item->flash_hide = !item->flash_hide;
                }
                else
                {
                    item->flash_timer -= dt;
                }
            }

            if (player->alive && collision_circle_to_circle(
                body->position, ITEM_RADIUS, player->header.body->position,
                PLAYER_RADIUS))
            {
                if (item->type < ITEM_SHOTGUN || item->type > ITEM_PISTOL ||
                    key_times_pressed(&input->weapon_pick))
                {
                    player->item_picked = item->type;
                    item->alive = 0;

                    rigid_body_free(body);
                }
            }
        }
    }
}

void items_render(struct object_pool* item_pool, struct cube_renderer* renderer)
{
    struct item* items = item_pool->data;

    for (u32 i = 0; i < item_pool->count; i++)
    {
        struct item* item = &items[i];

        if (item->alive && !item->flash_hide)
        {
            struct rigid_body* body = item->header.body;
            f32 t = f32_sin(body->angle) * 0.25f;

            struct m4 transform = m4_translate(body->position.x,
                body->position.y, ITEM_RADIUS);
            struct m4 rotation = m4_rotate_z(body->angle);
            struct m4 scale = m4_scale_all(ITEM_RADIUS + t * ITEM_RADIUS);
            struct m4 model = m4_mul_m4(scale, rotation);

            model = m4_mul_m4(model, transform);

            item->cube.model = model;

            cube_renderer_add(renderer, &item->cube);
        }
    }
}
