struct scene_physics scene_physics_init(struct game_state* game)
{
    struct scene_physics result = { 0 };

    result.renderer = renderer_init(game->shader_simple, m4_orthographic(0.0f,
        game->screen_width, 0.0f, game->screen_height, 0.0f, 1.0f), &api.gl);

    return result;
}

void scene_physics_update(struct scene_physics* data, struct game_input* input,
    f32 step)
{
    // Todo: temporary here to reset everything on hot reload
    memory_set(&data->entities, sizeof(data->entities), 0);
    data->num_entities = 0;

    struct tk_entity* circle = &data->entities[data->num_entities++];
    circle->x = 200.0f;
    circle->y = 200.0f;
    circle->radius = 50.0f;
    circle->color = colors[AQUA];
    circle->flags = TK_ENTITY_ALIVE | TK_ENTITY_RENDERABLE;
    circle->render_type = TK_RENDER_TYPE_CIRCLE;
    circle->num_fans = 32;

    struct tk_entity* aabb = &data->entities[data->num_entities++];
    aabb->x = 400.0f;
    aabb->y = 200.0f;
    aabb->half_width = 100.0f;
    aabb->half_height = 50.0f;
    aabb->color = colors[OLIVE];
    aabb->flags = TK_ENTITY_ALIVE | TK_ENTITY_RENDERABLE;
    aabb->render_type = TK_RENDER_TYPE_AABB;
}

void scene_physics_render(struct scene_physics* data)
{
    for (u32 i = 0; i < data->num_entities; i++)
    {
        struct tk_entity* entity = &data->entities[i];

        if (entity->flags & TK_ENTITY_ALIVE)
        {
            if (entity->flags & TK_ENTITY_RENDERABLE)
            {
                switch (entity->render_type)
                {
                    case TK_RENDER_TYPE_CIRCLE:
                    {
                        renderer_draw_circle(&data->renderer, entity->x,
                            entity->y, entity->radius, entity->color,
                            entity->num_fans);

                    } break;
                    case TK_RENDER_TYPE_AABB:
                    {
                        renderer_draw_aabb(&data->renderer, entity->x,
                            entity->y, entity->half_width, entity->half_height,
                            entity->color);
                    } break;
                }
            }
        }
    }
}
