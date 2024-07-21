struct tk_entity* tk_entity_get_free(struct tk_entity* entities,
    u32 num_entities)
{
    struct tk_entity* result = NULL;

    for (u32 i = 0; i < num_entities; i++)
    {
        if (!entities[i].flags & TK_ENTITY_ALIVE)
        {
            result = &entities[i];
            *result = (struct tk_entity){ 0 };
            result->flags |= TK_ENTITY_ALIVE;

            break;
        }
    }

    return result;
}

void tk_entity_apply_force(struct tk_entity* entity, struct v2 force)
{
    entity->force.x += force.x;
    entity->force.y += force.y;
}

b32 tk_collision_detect_circle_circle(
    struct v2 a_pos, struct v2 a_move, f32 a_radius,
    struct v2 b_pos, struct v2 b_move, f32 b_radius,
    struct tk_contact* contact)
{
    b32 result = false;

    // Todo: add early escapes
    // - amount of movement is less distance between circles minus radii

    // Reduce the velocity of b from the velocity of a
    struct v2 a_vel = v2_sub(a_move, b_move);

    // Calculate velocity direction
    struct v2 a_dir = v2_normalize(a_vel);

    // Calculate vector from a to b
    struct v2 ab = v2_direction(a_pos, b_pos);

    // There cannot be collision if the circles are moving to different
    // directions
    if (v2_dot(ab, a_dir) > 0.0f)
    {
        // Calculate closest point to b on line in velocity direction
        struct v2 d_pos = v2_add(a_pos, v2_mul_f32(a_dir, v2_dot(ab, a_dir)));

        // Calculate velocity
        f32 a_len = v2_length(a_vel);

        // Calculate distance from circle a to d
        f32 ad_len = v2_distance(a_pos, d_pos);

        // Calculate squared distance from b to d
        f32 bd_len = v2_distance_squared(b_pos, d_pos);

        // Calculate total squared radii of a and b
        f32 rad_total = f32_square(a_radius + b_radius);

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
                    contact->position = v2_add(a_pos,
                        v2_mul_f32(a_dir, ac_len));
                    contact->time_delta = ac_len / a_len;
                    // contact->a = a;
                    // contact->b = b;
                }
            }
        }
    }

    return result;
}

void physics_update_velocities(struct tk_entity* entities, u32 num_entities,
    f32 dt)
{
    for (u32 i = 0; i < num_entities; i++)
    {
        struct tk_entity* entity = &entities[i];

        if (!(entity->flags & TK_ENTITY_ALIVE))
        {
            continue;
        }

        if (entity->flags & TK_ENTITY_DYNAMIC)
        {
            if (entity->mass > 0.0f)
            {
                tk_entity_apply_force(entity,
                    v2_mul_f32(v2_negate(entity->velocity), entity->friction));

                struct v2 acceleration =
                {
                    entity->force.x / entity->mass,
                    entity->force.y / entity->mass
                };

                entity->velocity.x += acceleration.x * dt;
                entity->velocity.y += acceleration.y * dt;

                entity->move_delta.x = entity->velocity.x * dt;
                entity->move_delta.y = entity->velocity.y * dt;
            }

            entity->force = v2_zero;
        }
    }
}

u32 physics_check_contacts(struct tk_entity* entities, u32 num_entities,
    struct tk_contact* contacts, u32 max_contacts, u32 id)
{
    u32 num_contacts = 0;

    for (u32 i = 0; i < num_entities - 1; i++)
    {
        struct tk_entity* a = &entities[i];

        if (!(a->flags & TK_ENTITY_ALIVE) || !(a->flags & TK_ENTITY_DYNAMIC))
        {
            continue;
        }

        for (u32 j = i + 1; j < num_entities; j++)
        {
            struct tk_entity* b = &entities[j];

            if (!(b->flags & TK_ENTITY_ALIVE) || !b->flags & TK_ENTITY_DYNAMIC)
            {
                continue;
            }

            struct tk_contact contact = { 0 };
            b32 result = tk_collision_detect_circle_circle(a->position,
                a->move_delta, a->radius, b->position, b->move_delta,
                b->radius, &contact);

            if (result && num_contacts < max_contacts)
            {
                contact.entity_a = a;
                contact.entity_b = b;

                contacts[num_contacts++] = contact;

                LOG("[%u] contact: %s -> %s\n", id, a->name, b->name);
            }
        }
    }

    return num_contacts;
}

void physics_resolve_contacts(struct tk_contact* contacts, u32 num_contacts,
    f32 dt)
{
    f32 t = 1.0f;

    for (u32 i = 0; i < num_contacts; i++)
    {
        t = MIN(t, contacts[i].time_delta);
    }

    LOG("Time delta is %f\n", t);

    for (u32 i = 0; i < num_contacts; i++)
    {
        struct tk_contact* contact = &contacts[i];
        struct tk_entity* a = contact->entity_a;
        struct tk_entity* b = contact->entity_b;

        f32 t_remaining = 1.0f - MAX(t, contact->time_delta);

        struct v2 n = v2_normalize(v2_direction(b->position, a->position));

        f32 a1 = v2_dot(a->velocity, n);
        f32 a2 = v2_dot(b->velocity, n);

        f32 p = (2.0f * (a1 - a2)) / (a->mass + b->mass);

        a->velocity.x = a->velocity.x - p * b->mass * n.x;
        a->velocity.y = a->velocity.y - p * b->mass * n.y;

        b->velocity.x = b->velocity.x + p * a->mass * n.x;
        b->velocity.y = b->velocity.y + p * a->mass * n.y;

        a->move_delta = v2_mul_f32(a->move_delta, contact->time_delta);
        a->position = v2_add(a->position, a->move_delta);
        a->move_delta = v2_mul_f32(a->velocity, t_remaining * dt);

        b->move_delta = v2_mul_f32(b->move_delta, contact->time_delta);
        b->position = v2_add(b->position, b->move_delta);
        b->move_delta = v2_mul_f32(b->velocity, t_remaining * dt);
    }
}

void physics_move_entities(struct tk_entity* entities, u32 num_entities)
{
    for (u32 i = 0; i < num_entities; i++)
    {
        struct tk_entity* entity = &entities[i];

        if (!(entity->flags & TK_ENTITY_ALIVE))
        {
            continue;
        }

        if (entity->flags & TK_ENTITY_DYNAMIC)
        {
            entity->position.x += entity->move_delta.x;
            entity->position.y += entity->move_delta.y;
        }
    }
}

void physics_advance(struct scene_physics* data, struct game_input* input,
    f32 dt)
{
    // Copy old frame to new
    struct physics_frame* prev = &data->frames[data->frame_current];

    if (++data->frame_current >= MAX_PHYSICS_FRAMES)
    {
        data->frame_current = 0;
    }

    struct physics_frame* frame = &data->frames[data->frame_current];
    *frame = *prev;

    frame->id++;

    // LOG("Frame: %u\n", frame->id);

    // Note: this is nowhere near optimal code. Priority is to make it
    // functional and then clean it up, optimize if needed

    // Apply forces
    for (u32 i = 0; i < frame->num_entities; i++)
    {
        struct tk_entity* entity = &frame->entities[i];

        if (!(entity->flags & TK_ENTITY_ALIVE))
        {
            continue;
        }

        if (entity->flags & TK_ENTITY_DYNAMIC &&
            entity->flags & TK_ENTITY_PLAYER_CONTROLLED)
        {
            struct v2 dir = { 0.0f};

            if (input->move_right.key_down)
            {
                dir.x += 1.0f;
            }

            if (input->move_left.key_down)
            {
                dir.x -= 1.0f;
            }

            if (input->move_up.key_down)
            {
                dir.y += 1.0f;
            }

            if (input->move_down.key_down)
            {
                dir.y -= 1.0f;
            }

            dir = v2_normalize(dir);

            tk_entity_apply_force(entity,
                v2_mul_f32(dir, entity->acceleration));
        }
    }

    physics_update_velocities(frame->entities, frame->num_entities, dt);

    u32 max_iterations = 100;

    for (u32 i = 0; i < max_iterations; i++)
    {
        frame->num_contacts = physics_check_contacts(frame->entities,
            frame->num_entities, frame->contacts, TK_MAX_CONTACTS, frame->id);

        if (frame->num_contacts > 0)
        {
            LOG("[%u] %u iteration\n", frame->id, i + 1);

            physics_resolve_contacts(frame->contacts, frame->num_contacts, dt);
        }
    }

    physics_move_entities(frame->entities, frame->num_entities);

    // Todo: frame_min should be updated as well
    data->frame_max = data->frame_current;
}

u32 state_frame_decrease(u32 frame_current, u32 frame_min, u32 frame_max)
{
    u32 result = frame_current;

    if (frame_current != frame_min)
    {
        result = --frame_current % MAX_PHYSICS_FRAMES;

        LOG("Frame: %u\n", result);
    }

    return result;
}

u32 state_frame_increase(u32 frame_current, u32 frame_min, u32 frame_max)
{
    u32 result = frame_current;

    if (frame_current != frame_max)
    {
        result = ++frame_current % MAX_PHYSICS_FRAMES;
    }

    return result;
}

// Todo: TESTING ONLY, MEGA HAZARD
void text_copy(char* src, char* dst)
{
    while (*src)
    {
        *dst++ = *src++;
    };
}

void scene_physics_init(struct game_state* game, struct scene_physics* data)
{
    memory_set(data, sizeof(struct scene_physics), 0);

    data->renderer = renderer_init(game->shader_simple, m4_orthographic(0.0f,
        10.0f, 0.0f, 10.0f, 0.0f, 1.0f), &api.gl);
    data->paused = false;

    // Initialize first frame
    struct physics_frame* frame = &data->frames[0];

    frame->num_entities = ARRAY_SIZE(frame->entities);

    struct tk_entity* circle = tk_entity_get_free(frame->entities,
        frame->num_entities);
    text_copy("A", circle->name);
    circle->position.x = 2.0f;
    circle->position.y = 2.0f;
    circle->radius = 0.5f;
    circle->color = colors[AQUA];
    circle->flags = TK_ENTITY_ALIVE | TK_ENTITY_RENDERABLE | TK_ENTITY_DYNAMIC |
        TK_ENTITY_PLAYER_CONTROLLED;
    circle->render_type = TK_RENDER_TYPE_CIRCLE;
    circle->num_fans = 32;
    circle->friction = 10.0f;
    circle->acceleration = 40.0f;
    circle->mass = 1.0f;

    for (u32 y = 0; y < 4; y++)
    {
        for (u32 x = 0; x < 6; x++)
        {
            circle = tk_entity_get_free(frame->entities, frame->num_entities);
            text_copy("", circle->name);
            circle->position.x = 1.0f + 0.5f * x;
            circle->position.y = 5.0f + 0.5f * y;
            circle->radius = 0.25f;
            circle->color = colors[AQUA];
            circle->flags = TK_ENTITY_ALIVE | TK_ENTITY_RENDERABLE |
                TK_ENTITY_DYNAMIC;
            circle->render_type = TK_RENDER_TYPE_CIRCLE;
            circle->num_fans = 32;
            circle->friction = 2.0f;
            circle->acceleration = 0.0f;
            circle->mass = 5.0f;
        }
    }

    circle = tk_entity_get_free(frame->entities, frame->num_entities);
    text_copy("B", circle->name);
    circle->position.x = 6.0f;
    circle->position.y = 2.0f;
    circle->velocity.x = -10.0f;
    circle->radius = 0.5f;
    circle->color = colors[AQUA];
    circle->flags = TK_ENTITY_ALIVE | TK_ENTITY_RENDERABLE | TK_ENTITY_DYNAMIC;
    circle->render_type = TK_RENDER_TYPE_CIRCLE;
    circle->num_fans = 32;
    circle->friction = 2.0f;
    circle->acceleration = 0.0f;
    circle->mass = 1.0f;

    struct tk_entity* aabb = tk_entity_get_free(frame->entities,
        frame->num_entities);
    text_copy("RECT", aabb->name);
    aabb->position.x = 4.0f;
    aabb->position.y = 2.0f;
    aabb->half_width = 1.0f;
    aabb->half_height = 0.5f;
    aabb->color = colors[OLIVE];
    aabb->flags = TK_ENTITY_ALIVE | TK_ENTITY_RENDERABLE;
    aabb->render_type = TK_RENDER_TYPE_AABB;
}

void scene_physics_update(struct scene_physics* data, struct game_input* input,
    f32 dt)
{
    if (key_times_pressed(&input->physics_pause))
    {
        data->paused = !data->paused;
    }

    if (!data->paused)
    {
        physics_advance(data, input, dt);
    }
    else
    {
        if (key_times_pressed(&input->move_left))
        {
            data->frame_current = state_frame_decrease(data->frame_current,
                data->frame_min, data->frame_max);
        }
        else if (key_times_pressed(&input->move_right))
        {
            data->frame_current = state_frame_increase(data->frame_current,
                data->frame_min, data->frame_max);
        }
        else if (input->move_up.key_down)
        {
            data->frame_current = state_frame_increase(data->frame_current,
                data->frame_min, data->frame_max);
        }
        else if (input->move_down.key_down)
        {
            data->frame_current = state_frame_decrease(data->frame_current,
                data->frame_min, data->frame_max);
        }

        if (input->physics_advance.key_down)
        {
            physics_advance(data, input, dt);
        }

        if (key_times_pressed(&input->physics_advance_step))
        {
            physics_advance(data, input, dt);
        }
    }
}

void scene_physics_render(struct scene_physics* data)
{
    struct physics_frame* frame = &data->frames[data->frame_current];

    for (u32 i = 0; i < frame->num_entities; i++)
    {
        struct tk_entity* entity = &frame->entities[i];

        if (!(entity->flags & TK_ENTITY_ALIVE))
        {
            continue;
        }

        if (entity->flags & TK_ENTITY_RENDERABLE)
        {
            struct v4 color = entity->color;

            if (entity->flags & TK_ENTITY_DYNAMIC && entity->in_contact)
            {
                color = colors[RED];
            }

            switch (entity->render_type)
            {
                case TK_RENDER_TYPE_CIRCLE:
                {
                    renderer_draw_circle(&data->renderer, entity->position.x,
                        entity->position.y, entity->radius, color,
                        entity->num_fans);
                } break;
                case TK_RENDER_TYPE_AABB:
                {
                    renderer_draw_aabb(&data->renderer, entity->position.x,
                        entity->position.y, entity->half_width,
                        entity->half_height, color);
                } break;
            }
        }
    }
}
