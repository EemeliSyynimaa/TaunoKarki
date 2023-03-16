// Todo: move data specific for this state from game state to here
struct state_game_data
{
    struct game_state* base;
};

void state_game_init(struct state_game_data* data)
{
    struct game_state* state = data->base;

    cube_renderer_init(&state->cube_renderer, state->shader_cube,
        state->texture_cube);
    sprite_renderer_init(&state->sprite_renderer,
        state->shader_sprite, state->texture_sprite);
    particle_renderer_init(&state->particle_renderer,
        state->shader_particle, state->texture_particle);

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

    particle_emitter_create(&state->particle_system, &config, false);

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

    particle_emitter_create(&state->particle_system, &config, true);

    struct camera* camera = &state->camera;
    camera->projection = m4_perspective(60.0f,
        (f32)camera->screen_width / (f32)camera->screen_height, 0.1f, 15.0f);
    camera->projection_inverse = m4_inverse(camera->projection);
    state->render_debug = false;

    u32 num_colors = sizeof(colors) / sizeof(struct v4);

    for (u32 i = 0; i < num_colors; i++)
    {
        cube_renderer_color_add(&state->cube_renderer, colors[i]);
    }

    state->level_current = 1;

    level_mask_init(&state->level_mask);
    level_init(state);

    api.gl.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

void state_game_update(struct state_game_data* data, struct game_input* input,
    f32 step)
{
    struct game_state* state = data->base;

    if (state->level_clear_notify <= 0.0f)
    {
        state->cube_renderer.num_cubes = 0;
        state->sprite_renderer.num_sprites = 0;
        state->particle_renderer.num_particles = 0;

        player_update(state, input, step);
        enemies_update(state, input, step);
        bullets_update(state, input, step);
        items_update(state, input, step);
        particle_lines_update(state, input, step);
        particle_system_update(&state->particle_system, step);
    }

    struct v2 start_min = v2_sub_f32(state->level.start_pos, 2.0f);
    struct v2 start_max = v2_add_f32(state->level.start_pos, 2.0f);
    struct v2 plr_pos = state->player.body.position;

    state->player_in_start_room = plr_pos.x > start_min.x &&
        plr_pos.x < start_max.x && plr_pos.y > start_min.y &&
        plr_pos.y < start_max.y;

    struct camera* camera = &state->camera;
    struct mouse* mouse = &state->mouse;

    if (state->level_clear_notify > 0.0f)
    {
        camera->target.x = state->level.start_pos.x;
        camera->target.y = state->level.start_pos.y + 1.0f;
        camera->target.z = 3.75f;

        state->level_clear_notify -= step;
    }
    else if (state->player_in_start_room)
    {
        camera->target.xy = state->level.start_pos;
        camera->target.z = 3.75f;
    }
    else
    {
        f32 distance_to_activate = 0.0f;

        struct v2 direction_to_mouse = v2_normalize(v2_direction(
            state->player.body.position, mouse->world));

        struct v2 target_pos = v2_average(mouse->world,
            state->player.body.position);

        f32 distance_to_target = v2_distance(target_pos,
            state->player.body.position);

        distance_to_target -= distance_to_activate;

        struct v2 target = state->player.body.position;

        if (distance_to_target > 0)
        {
            target.x = state->player.body.position.x +
                direction_to_mouse.x * distance_to_target;
            target.y = state->player.body.position.y +
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

    collision_map_dynamic_calculate(state);

    state->num_gun_shots = 0;

    if (state->level_change)
    {
        state->level_cleared = false;
        state->level_change = false;
        state->accumulator = 0;
        state->level_current++;
        level_init(state);
    }
}

void state_game_render(struct state_game_data* data)
{
    struct game_state* state = data->base;
    struct camera* camera = &state->camera;

    level_render(data->base, &state->level);
    player_render(data->base);
    enemies_render(data->base);
    items_render(data->base);
    particle_system_render(&state->particle_system,
        &state->particle_renderer);

    particle_renderer_sort(&state->particle_renderer);

    cube_renderer_flush(&state->cube_renderer, &camera->view,
        &camera->projection);
    sprite_renderer_flush(&state->sprite_renderer, &camera->view,
        &camera->projection);
    particle_renderer_flush(&state->particle_renderer, &camera->view,
        &camera->projection);

    particle_lines_render(data->base);

    if (state->render_debug)
    {
        collision_map_render(data->base);
    }
}

struct state_interface state_game_create(struct game_state* state)
{
    struct state_interface result = { 0 };
    result.init = state_game_init;
    result.update = state_game_update;
    result.render = state_game_render;
    result.data = stack_alloc(&state->stack, sizeof(struct state_game_data));

    memory_set(result.data, sizeof(struct state_game_data), 0);

    struct game_state** base = (struct game_state**)result.data;

    *base = state;

    return result;
}
