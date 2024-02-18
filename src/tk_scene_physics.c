struct scene_physics scene_physics_init(struct game_state* game)
{
    struct scene_physics result = { 0 };

    result.renderer = renderer_init(game->shader_simple, m4_orthographic(0.0f,
        10.0f, 0.0f, 10.0f, 0.0f, 1.0f), &api.gl);

    struct tk_entity* circle = &result.entities[result.num_entities++];
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

    struct tk_entity* aabb = &result.entities[result.num_entities++];
    aabb->position.x = 4.0f;
    aabb->position.y = 2.0f;
    aabb->half_width = 1.0f;
    aabb->half_height = 0.5f;
    aabb->color = colors[OLIVE];
    aabb->flags = TK_ENTITY_ALIVE | TK_ENTITY_RENDERABLE;
    aabb->render_type = TK_RENDER_TYPE_AABB;

    return result;
}

void scene_physics_update(struct scene_physics* data, struct game_input* input,
    f32 dt)
{
    for (u32 i = 0; i < data->num_entities; i++)
    {
        struct tk_entity* entity = &data->entities[i];

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

            acceleration = v2_mul_f32(dir, entity->acceleration);
        }

        if (entity->flags & TK_ENTITY_DYNAMIC)
        {
            acceleration.x -= entity->velocity.x * entity->friction;
            acceleration.y -= entity->velocity.y * entity->friction;

            entity->position.x += (entity->velocity.x + 0.5f *
                acceleration.x * dt) * dt;
            entity->position.y += (entity->velocity.y + 0.5f *
                acceleration.y * dt) * dt;

            entity->velocity.x += acceleration.x * dt;
            entity->velocity.y += acceleration.y * dt;
        }
    }
}

void scene_physics_render(struct scene_physics* data)
{
    for (u32 i = 0; i < data->num_entities; i++)
    {
        struct tk_entity* entity = &data->entities[i];

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
