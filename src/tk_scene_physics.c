struct scene_physics scene_physics_init(struct game_state* game)
{
    struct scene_physics result = { 0 };

    result.renderer.shader = game->shader_simple;
    result.renderer.view_projection = m4_orthographic(0.0f, game->screen_width,
        0.0f, game->screen_height, 0.0f, 1.0f);

    return result;
}

void scene_physics_update(struct scene_physics* data, struct game_input* input,
    f32 step)
{
}

void scene_physics_render(struct scene_physics* data)
{
    render_circle(&data->renderer, 200.0f, 200.0f, 50.0f, colors[TEAL], 32);
    render_aabb(&data->renderer, 400.0f, 200.0f, 100.0f, 50.0f, colors[OLIVE]);
}
