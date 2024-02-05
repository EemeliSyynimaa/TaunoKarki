// Todo: move data specific for this state from game state to here
void scene_game_init(struct scene_game* data)
{
    struct game_state* game = data->base;

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

    struct camera* camera = &game->camera_game;
    camera->projection = m4_perspective(60.0f,
        (f32)camera->width / (f32)camera->height, 0.1f, 15.0f);
    camera->projection_inverse = m4_inverse(camera->projection);

    camera = &game->camera_gui;
    camera->projection = m4_orthographic(0.0f, camera->width, 0.0f,
        camera->height, 0.0f, 1.0f);
    camera->projection_inverse = m4_inverse(camera->projection);

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

void scene_game_update(struct scene_game* data, struct game_input* input,
    f32 step)
{
    struct game_state* game = data->base;

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
        bullet_trails_update(game, input, step);
        particle_system_update(&game->particle_system, step);

        world_update(&game->world, step);
    }

    struct v2 start_min = v2_sub_f32(game->level.start_pos, 2.0f);
    struct v2 start_max = v2_add_f32(game->level.start_pos, 2.0f);
    struct v2 plr_pos = game->player.header.body->position;

    game->player_in_start_room = plr_pos.x > start_min.x &&
        plr_pos.x < start_max.x && plr_pos.y > start_min.y &&
        plr_pos.y < start_max.y;

    struct camera* camera = &game->camera_game;
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

        struct v2 direction_to_mouse = v2_normalize(v2_direction(plr_pos,
            mouse->world));

        struct v2 target_pos = v2_average(mouse->world, plr_pos);

        f32 distance_to_target = v2_distance(target_pos, plr_pos);

        distance_to_target -= distance_to_activate;

        struct v2 target = plr_pos;

        if (distance_to_target > 0)
        {
            target.x = plr_pos.x + direction_to_mouse.x * distance_to_target;
            target.y = plr_pos.y + direction_to_mouse.y * distance_to_target;
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

    mouse->world = camera_view_to_world_pos(camera, (f32)input->mouse_x,
        (f32)input->mouse_y);

    // Todo: get the bodies
    // collision_map_dynamic_calculate(&game->cols, bodies, num_bodies);

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

void scene_game_render(struct scene_game* data)
{
    struct game_state* game = data->base;
    struct camera* camera_game = &game->camera_game;
    struct camera* camera_gui = &game->camera_gui;

    if (game->level_cleared)
    {
        cube_data_color_update(&game->level.elevator_light, LIME);
    }

    level_render(&game->level, &game->sprite_renderer, &game->cube_renderer,
        camera_game->projection, camera_game->view);
    player_render(game, camera_game, camera_gui);
    enemies_render(game, camera_game, camera_gui);
    items_render(&game->item_pool, &game->cube_renderer);
    particle_system_render(&game->particle_system, &game->particle_renderer);

    particle_renderer_sort(&game->particle_renderer);

    cube_renderer_flush(&game->cube_renderer, &camera_game->view,
        &camera_game->projection);
    sprite_renderer_flush(&game->sprite_renderer, &camera_game->view,
        &camera_game->projection);
    particle_renderer_flush(&game->particle_renderer, &camera_game->view,
        &camera_game->projection);

    bullet_trails_render(game, camera_game);

    if (game->render_debug)
    {
        collision_map_render(game, game->cols.statics, game->cols.num_statics,
            camera_game);
    }
}
