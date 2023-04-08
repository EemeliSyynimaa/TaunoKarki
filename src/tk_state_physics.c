struct frame
{
    struct circle circles[MAX_CIRCLES];
    struct contact contacts[MAX_CONTACTS];
    struct line_segment lines[MAX_COLLISION_SEGMENTS];
    u32 num_circles;
    u32 num_lines;
    u32 number;
};

#define MAX_FRAMES 3600

struct state_physics_data
{
    struct game_state* base;
    struct frame frames[MAX_FRAMES];
    u32 frame_current;
    u32 frame_max;
    u32 frame_min;
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

            struct v2 acceleration = { 0.0f };

            if (i == 0)
            {
                // if (input->move_left.key_down)
                // {
                //     acceleration.x -= 1.0f;
                // }

                // if (input->move_right.key_down)
                {
                    acceleration.x += 1.0f;
                }

                // if (input->move_down.key_down)
                {
                    acceleration.y -= 1.0f;
                }

                // if (input->move_up.key_down)
                // {
                //     acceleration.y += 1.0f;
                // }

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

    // Todo: this looks horrible, refactor and clean

    for (u32 i = 0; i < num_circles; i++)
    {
        struct circle* circle = &circles[i];

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
                        if ((!circle->contact ||
                            contact.t < circle->contact->t) &&
                            (!other->contact || contact.t < other->contact->t))
                        {
                            if (!circle->contact && !other->contact)
                            {
                                circle->contact = &contacts[result++];
                                other->contact = circle->contact;
                            }
                            else
                            {
                                if (circle->contact)
                                {
                                    if (circle->contact->a &&
                                        circle->contact->a != circle)
                                    {
                                        circle->contact->a->contact = NULL;
                                    }

                                    if (circle->contact->b &&
                                        circle->contact->b != circle)
                                    {
                                        circle->contact->b->contact = NULL;
                                    }

                                    other->contact = circle->contact;
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

                                    circle->contact = other->contact;
                                }
                            }

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

                        // Todo: HERE RECURSIVELY CHCECK THAT EACH CONTACT IS
                        // NULLIFIED FOR ALL PARTIES
                    LOG("COLLISION: circle %d and line %d\n", i, j);
                    if (!circle->contact || contact.t < circle->contact->t)
                    {
                        if (circle->contact)
                        {
                            if (circle->contact->a &&
                                circle->contact->a != circle)
                            {
                                circle->contact->a->contact = NULL;
                            }

                            if (circle->contact->b &&
                                circle->contact->b != circle)
                            {
                                circle->contact->b->contact = NULL;
                            }
                        }
                        else
                        {
                            circle->contact = &contacts[result++];
                        }

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

void state_physics_init(void* data)
{
    struct state_physics_data* state = (struct state_physics_data*)data;
    struct frame* frame = &state->frames[state->frame_current];

    // Create lines
    frame->num_lines = 4;

    f32 width = 4.5f;
    f32 height = 4.5f;
    f32 spawn_area = 4.0f;

    // Top
    frame->lines[0].start = (struct v2){ -width, height };
    frame->lines[0].end = (struct v2){ width, height };
    // Bottom
    frame->lines[1].start = (struct v2){ -width, -height };
    frame->lines[1].end = (struct v2){ width, -height };
    // Left
    frame->lines[2].start = (struct v2){ -width, height };
    frame->lines[2].end = (struct v2){ -width, -height };
    // Right
    frame->lines[3].start = (struct v2){ width, height };
    frame->lines[3].end = (struct v2){ width, -height };

    // Create circles
    frame->num_circles = 6; // 1 controllable, 4 static, 10 dynamic

    // Make the first circle controllable
    u32 index = 0;
    struct circle* circle = &frame->circles[index++];
    circle->position.x = 3.75f;
    circle->position.y = -3.75f;
    circle->radius = 0.25f;
    circle->mass = 10.0f;
    circle->dynamic = true;

    // Todo: these are not really handled as static, they are  just dynamic
    // circles but with HUGE mass

    // Create static circles
    for (u32 i = 0; i < 4; i++)
    {
        circle = &frame->circles[index++];
        circle->position.x = ((f32)(i % 2) - 0.5f) * spawn_area;
        circle->position.y = ((f32)(i / 2) - 0.5f) * spawn_area;
        circle->radius = 0.35f;
        circle->mass = F32_MAX;
        circle->dynamic = false;
    }

    #if 0
    // Create dynamic circles
    for (u32 i = 0; i < 10; i++)
    {
        circle = &frame->circles[index++];
        circle->position.x = f32_random(-spawn_area, spawn_area);
        circle->position.y = f32_random(-spawn_area, spawn_area);
        circle->radius = 0.25f;
        circle->mass = 1.0f;
        circle->dynamic = true;
        circle->target.x = f32_random(-spawn_area, spawn_area);
        circle->target.y = f32_random(-spawn_area, spawn_area);
    }
    #else
    circle = &frame->circles[index++];
    circle->position.x = 4.25f;
    circle->position.y = -4.25f;
    circle->radius = 0.25f;
    circle->mass = 1.0f;
    circle->dynamic = true;
    circle->target.x = 4.0f;
    circle->target.y = -4.0f;
    #endif

    // Setup camera
    f32 size = 5.0f;
    f32 near = 0.1f;
    f32 far = 100.0f;

    struct camera* camera = &state->base->camera;
    camera->position = (struct v3){ 0.0f, 0.0f, 5.0f };
    camera->projection = m4_orthographic(-size, size, -size, size, near, far);
    camera->projection_inverse = m4_inverse(camera->projection);
    camera->view = m4_look_at(camera->position,
        (struct v3) { camera->position.xy, 0.0f },
        (struct v3) { 0.0f, 1.0f, 0.0f });
    camera->view_inverse = m4_inverse(camera->view);

    api.gl.glClearColor(0.25f, 0.0f, 0.0f, 0.0f);

    // Start as paused
    state->paused = true;
}

void physics_advance(void* data, struct game_input* input, f32 step)
{
    struct state_physics_data* state = (struct state_physics_data*)data;

    // Copy old frame to new
    struct frame* prev = &state->frames[state->frame_current];

    if (++state->frame_current >= MAX_FRAMES)
    {
        state->frame_current = 0;
    }

    struct frame* frame = &state->frames[state->frame_current];
    *frame = *prev;

    LOG("Frame: %u\n", ++frame->number);

    circles_velocities_update(frame->circles, frame->num_circles, input,
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

        u32 num_contacts = circles_collisions_check(frame->circles,
            frame->num_circles, frame->contacts, frame->lines,
            frame->num_lines);

        if (num_contacts)
        {
            circles_collisions_resolve(frame->contacts, num_contacts, step);
        }
        else
        {
            LOG("No contacts!\n");
            break;
        }
    }

    circles_positions_update(frame->circles, frame->num_circles);

    // Todo: frame_min should be updated as well
    state->frame_max = state->frame_current;
}

u32 state_frame_decrease(u32 frame_current, u32 frame_min, u32 frame_max)
{
    u32 result = frame_current;

    if (frame_current != frame_min)
    {
        result = --frame_current % MAX_FRAMES;

        LOG("Frame: %u\n", result);
    }

    return result;
}

u32 state_frame_increase(u32 frame_current, u32 frame_min, u32 frame_max)
{
    u32 result = frame_current;

    if (frame_current != frame_max)
    {
        result = ++frame_current % MAX_FRAMES;

        LOG("Frame: %u\n", result);
    }

    return result;
}

void state_physics_update(void* data, struct game_input* input, f32 step)
{
    struct state_physics_data* state = (struct state_physics_data*)data;

    state->paused = !input->pause;

    if (!state->paused)
    {
        physics_advance(state, input, step);
    }
    else
    {
        if (key_times_pressed(&input->move_left))
        {
            state->frame_current = state_frame_decrease(state->frame_current,
                state->frame_min, state->frame_max);
        }
        else if (key_times_pressed(&input->move_right))
        {
            state->frame_current = state_frame_increase(state->frame_current,
                state->frame_min, state->frame_max);
        }
        else if (input->move_up.key_down)
        {
            state->frame_current = state_frame_increase(state->frame_current,
                state->frame_min, state->frame_max);
        }
        else if (input->move_down.key_down)
        {
            state->frame_current = state_frame_decrease(state->frame_current,
                state->frame_min, state->frame_max);
        }

        if (input->physics_advance.key_down)
        {
            physics_advance(state, input, step);
        }

        if (key_times_pressed(&input->physics_advance_step))
        {
            physics_advance(state, input, step);
        }
    }
}

void state_physics_render(void* data)
{
    struct state_physics_data* state = (struct state_physics_data*)data;
    struct frame* frame = &state->frames[state->frame_current];
    circles_render(frame->circles, frame->num_circles, state->base,
        state->paused);
    lines_render(frame->lines, frame->num_lines, state->base);
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
