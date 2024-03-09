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

    LOG("Frame: %u\n", ++frame->id);

    for (u32 i = 0; i < frame->num_entities; i++)
    {
        struct tk_entity* entity = &frame->entities[i];

        if (!(entity->flags & TK_ENTITY_ALIVE))
        {
            continue;
        }

        struct v2 acceleration = { 0.0f };

        if (entity->flags & TK_ENTITY_PLAYER_CONTROLLED)
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

                entity->position.x += entity->velocity.x * dt;
                entity->position.y += entity->velocity.y * dt;
            }

            entity->force = v2_zero;
        }
    }

    // circles_velocities_update(frame->circles, frame->num_circles, input,
    //     step);

    // f32 max_iterations = 10;

    // for (u32 i = 0; i < max_iterations; i++)
    // {
    //     LOG("Checking collisions, iteration %d\n", i + 1);

    //     // Todo: sometimes, for some reasons, a collision
    //     // between two objects happens again in the following
    //     // iteration. This shouldn't occur as the collision
    //     // is already resolved and the velocities and positions
    //     // are updated! Investigate

    //     // Todo: sometimes collisions are handled poorly
    //     // when multiple circles are touching...?

    //     u32 num_contacts = circles_collisions_check(frame->circles,
    //         frame->num_circles, frame->contacts, frame->lines,
    //         frame->num_lines);

    //     if (num_contacts)
    //     {
    //         circles_collisions_resolve(frame->contacts, num_contacts, step);
    //     }
    //     else
    //     {
    //         LOG("No contacts!\n");
    //         break;
    //     }
    // }

    // circles_positions_update(frame->circles, frame->num_circles);

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

        LOG("Frame: %u\n", result);
    }

    return result;
}

void scene_physics_init(struct game_state* game, struct scene_physics* data)
{
    memory_set(data, sizeof(struct scene_physics), 0);

    data->renderer = renderer_init(game->shader_simple, m4_orthographic(0.0f,
        10.0f, 0.0f, 10.0f, 0.0f, 1.0f), &api.gl);
    data->paused = true;

    // Initialize first frame
    struct physics_frame* frame = &data->frames[0];

    frame->num_entities = ARRAY_SIZE(frame->entities);

    struct tk_entity* circle = tk_entity_get_free(frame->entities,
        frame->num_entities);
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

    circle = tk_entity_get_free(frame->entities, frame->num_entities);
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
            switch (entity->render_type)
            {
                case TK_RENDER_TYPE_CIRCLE:
                {
                    renderer_draw_circle(&data->renderer, entity->position.x,
                        entity->position.y, entity->radius, entity->color,
                        entity->num_fans);
                } break;
                case TK_RENDER_TYPE_AABB:
                {
                    renderer_draw_aabb(&data->renderer, entity->position.x,
                        entity->position.y, entity->half_width,
                        entity->half_height, entity->color);
                } break;
            }
        }
    }
}
