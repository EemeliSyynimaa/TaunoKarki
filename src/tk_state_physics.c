struct state_physics_data
{
    struct game_state* base;
    struct circle circles[MAX_CIRCLES];
    struct contact contacts[MAX_CONTACTS];
    struct line_segment lines[MAX_COLLISION_SEGMENTS];
    u32 num_circles;
    u32 num_lines;
    b32 paused;
};

void lines_render(struct line_segment lines[], u32 num_lines,
    struct game_state* state)
{
    for (u32 i = 0; i < num_lines; i++)
    {
        line_render(state, lines[i].start, lines[i].end, colors[GREY], 1.25f,
            0.0125f);
    }
}

b32 collision_detect_circle_circle(struct circle* a, struct circle* b,
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

b32 collision_detect_circle_line(struct circle* a, struct line_segment* b,
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

void circles_velocities_update(struct circle circles[], u32 num_circles,
    struct game_input* input, f32 dt)
{
    for (u32 i = 0; i < num_circles; i++)
    {
        struct circle* circle = &circles[i];

        if (circle->dynamic)
        {
            f32 friction = FRICTION;
            f32 speed = 5.0f;

            struct v2 acceleration = { 0.0f };

            if (i == 0)
            {
                if (input->move_left.key_down)
                {
                    acceleration.x -= 1.0f;
                }

                if (input->move_right.key_down)
                {
                    acceleration.x += 1.0f;
                }

                if (input->move_down.key_down)
                {
                    acceleration.y -= 1.0f;
                }

                if (input->move_up.key_down)
                {
                    acceleration.y += 1.0f;
                }

                acceleration = v2_normalize(acceleration);
                acceleration = v2_mul_f32(acceleration,
                    PLAYER_ACCELERATION * 1.0f);
            }
            else
            {
                acceleration = v2_mul_f32(v2_direction(circle->position,
                    circle->target), 2.0f);
            }

            acceleration.x -= circle->velocity.x * friction;
            acceleration.y -= circle->velocity.y * friction;

            circle->velocity.x = circle->velocity.x + acceleration.x * dt;
            circle->velocity.y = circle->velocity.y + acceleration.y * dt;

            circle->move_delta.x = 0.5f * circle->acceleration.x * dt * dt +
                circle->velocity.x * dt;
            circle->move_delta.y = 0.5f * circle->acceleration.y * dt * dt +
                circle->velocity.y * dt;
        }
    }
}

u32 circles_collisions_check(struct circle circles[], u32 num_circles,
    struct contact contacts[], struct line_segment lines[], u32 num_lines)
{
    u32 result = 0;

    for (u32 i = 0; i < num_circles; i++)
    {
        struct circle* circle = &circles[i];
        circle->contact = NULL;

        if (result < MAX_CONTACTS)
        {
            if (i < num_circles - 1)
            {
                // Check collisions against other circles
                for (u32 j = i + 1; j < num_circles; j++)
                {
                    // Todo: we don't need to check the collisions if neither
                    // circle is moving
                    struct circle* other = &circles[j];
                    struct contact contact = { 0 };

                    if (collision_detect_circle_circle(circle, other, &contact))
                    {
                        LOG("COLLISION: circle %d and circle %d\n", i, j);
                        if (!circle->contact)
                        {
                            circle->contact =
                                &contacts[result++];
                            *circle->contact = contact;
                        }
                        else if (contact.t < circle->contact->t)
                        {
                            *circle->contact = contact;
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

                if (collision_detect_circle_line(circle, other, &contact))
                {
                    contact.a = circle;
                    contact.b = NULL;
                    contact.line = other;

                    LOG("COLLISION: circle %d and line %d\n", i, j);
                    if (!circle->contact)
                    {
                        circle->contact =
                            &contacts[result++];
                        *circle->contact = contact;
                    }
                    else if (contact.t < circle->contact->t)
                    {
                        *circle->contact = contact;
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

void circles_collisions_resolve(struct contact contacts[], u32 num_contacts,
    f32 dt)
{
    for (u32 i = 0; i < num_contacts; i++)
    {
        struct contact* contact = &contacts[i];
        // Todo: this is a bit of haxy way to move the remaining (1 - t)
        // with a new velocity
        f32 t_remaining = 1.0f - contact->t;

        struct circle* a = contact->a;
        struct circle* b = contact->b;
        struct line_segment* line = contact->line;

        // Todo: needs state, get rid of it
        // Render contact position
        // {
        //     struct m4 transform = m4_translate(contact->position.x,
        //         contact->position.y, 1.5f);
        //     struct m4 rotation = m4_identity();
        //     struct m4 scale = m4_scale_all(0.05f);

        //     struct m4 model = m4_mul_m4(scale, rotation);
        //     model = m4_mul_m4(model, transform);

        //     struct m4 mvp = m4_mul_m4(model, state->camera.view);
        //     mvp = m4_mul_m4(mvp, state->camera.projection);

        //     mesh_render(&state->sphere, &mvp, state->texture_sphere,
        //         state->shader_simple, colors[RED]);
        // }

        // Circle - circle
        if (a && b)
        {
#if 1
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

#else
            a->position = v2_add(a->position,
                v2_mul_f32(a->move_delta, contact->t));
            b->position = v2_add(b->position,
                v2_mul_f32(b->move_delta, contact->t));

            struct v2 n = v2_normalize(v2_direction(a->position, b->position));

            f32 vdot_a = v2_dot(a->velocity, n);
            f32 vdot_b = v2_dot(b->velocity, n);

            a->velocity = v2_sub(a->velocity, v2_mul_f32(n, vdot_a));
            b->velocity = v2_sub(b->velocity, v2_mul_f32(n, vdot_b));

            f32 mvdot_a = v2_dot(a->move_delta, n);
            f32 mvdot_b = v2_dot(b->move_delta, n);

            a->move_delta = v2_mul_f32(
                v2_sub(a->move_delta, v2_mul_f32(n, mvdot_a)), t_remaining);
            b->move_delta = v2_mul_f32(
                v2_sub(b->move_delta, v2_mul_f32(n, mvdot_b)), t_remaining);
#endif
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
    }
}

void circles_positions_update(struct circle circles[], u32 num_circles)
{
    for (u32 i = 0; i < num_circles; i++)
    {
        struct circle* circle = &circles[i];

        circle->position.x += circle->move_delta.x;
        circle->position.y += circle->move_delta.y;
    }
}

void circles_render(struct circle circles[], u32 num_circles,
    struct game_state* state, b32 paused)
{
    for (u32 i = 0; i < num_circles; i++)
    {
        struct circle* circle = &circles[i];
        struct m4 transform = m4_translate(circle->position.x,
            circle->position.y, 1.25f);
        struct m4 rotation = m4_identity();
        struct m4 scale = m4_scale_all(circle->radius);

        struct m4 model = m4_mul_m4(scale, rotation);
        model = m4_mul_m4(model, transform);

        struct m4 mvp = m4_mul_m4(model, state->camera.view);
        mvp = m4_mul_m4(mvp, state->camera.projection);

        u32 color = (i == 0) ? AQUA : (circle->dynamic ? WHITE : GREY);

        mesh_render(&state->sphere, &mvp, state->texture_sphere,
            state->shader_simple, colors[color]);

        if (circle->dynamic)
        {
            if (paused)
            {
                // Render movement vector
                line_render(state, circle->position, v2_add(circle->position,
                    circle->move_delta), colors[RED], 1.5f, 0.01f);
            }
            else
            {
                if (i)
                {
                    // Render line to target
                    line_render(state, circle->position, circle->target,
                        colors[RED], 1.1, 0.0125f);
                }
                else
                {
                    // Render velocity vector
                    f32 max_speed = 7.0f;
                    f32 length = v2_length(circle->velocity) / max_speed;
                    f32 angle = f32_atan(circle->velocity.x,
                        circle->velocity.y);

                    transform = m4_translate(
                        circle->position.x + circle->velocity.x / max_speed,
                        circle->position.y + circle->velocity.y / max_speed,
                        1.5f);

                    rotation = m4_rotate_z(-angle);
                    scale = m4_scale_xyz(0.025f, length, 1.5f);

                    model = m4_mul_m4(scale, rotation);
                    model = m4_mul_m4(model, transform);

                    struct m4 mvp = m4_mul_m4(model, state->camera.view);
                    mvp = m4_mul_m4(mvp, state->camera.projection);

                    mesh_render(&state->floor, &mvp, state->texture_tileset,
                        state->shader_simple, colors[GREY]);
                }
            }
        }
    }
}

void state_physics_init(struct state_physics_data* data)
{
    // Create lines
    data->num_lines = 4;

    f32 width = 4.5f;
    f32 height = 4.5f;
    f32 spawn_area = 4.0f;

    // Top
    data->lines[0].start = (struct v2){ -width, height };
    data->lines[0].end = (struct v2){ width, height };
    // Bottom
    data->lines[1].start = (struct v2){ -width, -height };
    data->lines[1].end = (struct v2){ width, -height };
    // Left
    data->lines[2].start = (struct v2){ -width, height };
    data->lines[2].end = (struct v2){ -width, -height };
    // Right
    data->lines[3].start = (struct v2){ width, height };
    data->lines[3].end = (struct v2){ width, -height };

    // Create circles
    data->num_circles = 15; // 1 controllable, 4 static, 10 dynamic

    // Make the first circle controllable
    u32 index = 0;
    struct circle* circle = &data->circles[index++];
    circle->position.x = 0.0f;
    circle->position.y = 0.0f;
    circle->radius = 0.25f;
    circle->mass = 10.0f;
    circle->dynamic = true;

    // Todo: these are not really handled as static, they are  just dynamic
    // circles but with HUGE mass

    // Create static circles
    for (u32 i = 0; i < 4; i++)
    {
        circle = &data->circles[index++];
        circle->position.x = ((f32)(i % 2) - 0.5f) * spawn_area;
        circle->position.y = ((f32)(i / 2) - 0.5f) * spawn_area;
        circle->radius = 0.35f;
        circle->mass = F32_MAX;
        circle->dynamic = false;
    }

    // Create dynamic circles
    for (u32 i = 0; i < 10; i++)
    {
        circle = &data->circles[index++];
        circle->position.x = f32_random(-spawn_area, spawn_area);
        circle->position.y = f32_random(-spawn_area, spawn_area);
        circle->radius = 0.25f;
        circle->mass = 1.0f;
        circle->dynamic = true;
        circle->target.x = f32_random(-spawn_area, spawn_area);
        circle->target.y = f32_random(-spawn_area, spawn_area);
    }

    // Setup camera
    f32 size = 5.0f;
    f32 near = 0.1f;
    f32 far = 100.0f;

    struct camera* camera = &data->base->camera;
    camera->position = (struct v3){ 0.0f, 0.0f, 5.0f };
    camera->projection = m4_orthographic(-size, size, -size, size, near, far);
    camera->projection_inverse = m4_inverse(camera->projection);
    camera->view = m4_look_at(camera->position,
        (struct v3) { camera->position.xy, 0.0f },
        (struct v3) { 0.0f, 1.0f, 0.0f });
    camera->view_inverse = m4_inverse(camera->view);

    api.gl.glClearColor(0.25f, 0.0f, 0.0f, 0.0f);
}

void state_physics_update(struct state_physics_data* data,
    struct game_input* input, f32 step)
{
    static f32 particle_test = 0;
    static u32 frame_number = 0;

    data->paused = input->pause;

    if (!data->paused)
    {
        LOG("Frame: %i\n", frame_number++);

        circles_velocities_update(data->circles, data->num_circles, input,
            step);

        f32 max_iterations = 10;

        for (u32 i = 0; i < max_iterations; i++)
        {
            LOG("Checking collisions, iteration %d\n", i + 1);

            // Todo: sometimes, for some reasons, a collision
            // between two objects happens again in the following
            // iteration. This shouldn't occur as the collision
            // is already resolved and the velocities and positions
            // are updated! Investigate

            // Todo: sometimes collisions are handled poorly
            // when multiple circles are touching...?

            u32 num_contacts = circles_collisions_check(data->circles,
                data->num_circles, data->contacts, data->lines,
                data->num_lines);

            if (num_contacts)
            {
                circles_collisions_resolve(data->contacts, num_contacts, step);
            }
            else
            {
                LOG("No contacts!\n");
                break;
            }
        }

        circles_positions_update(data->circles, data->num_circles);
    }
}

void state_physics_render(struct state_physics_data* data)
{
    circles_render(data->circles, data->num_circles, data->base, data->paused);
    lines_render(data->lines, data->num_lines, data->base);
}

struct state_interface state_physics_create(struct game_state* state)
{
    struct state_interface result = { 0 };
    result.init = state_physics_init;
    result.update = state_physics_update;
    result.render = state_physics_render;
    result.data = stack_alloc(&state->stack, sizeof(struct state_physics_data));

    memory_set(result.data, sizeof(struct state_physics_data), 0);

    struct game_state** base = (struct game_state**)result.data;

    *base = state;

    return result;
}
