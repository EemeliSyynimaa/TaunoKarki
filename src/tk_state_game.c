// Todo: move data specific for this state from game state to here
struct state_game_data
{
    struct game_state* base;
};

void rigid_bodies_update(struct rigid_body bodies[], u32 num_bodies,
    struct level* level, f32 dt)
{
    for (u32 i = 0; i < num_bodies; i++)
    {
        struct rigid_body* body = &bodies[i];

        if (body->alive)
        {
            struct v2 move_delta = { 0.0f };

            body->acceleration.x += -body->velocity.x * body->friction;
            body->acceleration.y += -body->velocity.y * body->friction;

            move_delta.x = 0.5f * body->acceleration.x * f32_square(dt) +
                body->velocity.x * dt;
            move_delta.y = 0.5f * body->acceleration.y * f32_square(dt) +
                body->velocity.y * dt;

            body->velocity.x = body->velocity.x + body->acceleration.x * dt;
            body->velocity.y = body->velocity.y + body->acceleration.y * dt;

            if (!body->bullet)
            {
                check_tile_collisions(level, &body->position, &body->velocity,
                    move_delta, body->radius, 1);
            }
        }
    }
}

void state_game_init(void* data)
{
    struct state_game_data* state = (struct state_game_data*)data;
    struct game_state* game = state->base;

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

    struct camera* camera = &game->camera;
    camera->perspective = m4_perspective(60.0f,
        (f32)camera->screen_width / (f32)camera->screen_height, 0.1f, 15.0f);
    camera->perspective_inverse = m4_inverse(camera->perspective);
    camera->ortho = m4_orthographic(0.0f, camera->screen_width, 0.0f,
        camera->screen_height, 0.0f, 1.0f);
    camera->ortho_inverse = m4_inverse(camera->ortho);

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

void state_game_update(void* data, struct game_input* input, f32 step)
{
    struct state_game_data* state = (struct state_game_data*)data;
    struct game_state* game = state->base;

    game->render_debug = input->enable_debug_rendering;

    if (game->level_clear_notify <= 0.0f)
    {
        game->cube_renderer.num_cubes = 0;
        game->sprite_renderer.num_sprites = 0;
        game->particle_renderer.num_particles = 0;

        player_update(game, input, step);
        enemies_update(game, input, step);
        bullets_update(game, input, step);
        items_update(game, input, step);
        particle_lines_update(game, input, step);
        particle_system_update(&game->particle_system, step);

        rigid_bodies_update(game->world.bodies, MAX_BODIES, &game->level, step);
    }

    struct v2 start_min = v2_sub_f32(game->level.start_pos, 2.0f);
    struct v2 start_max = v2_add_f32(game->level.start_pos, 2.0f);
    struct v2 plr_pos = game->player.body->position;

    game->player_in_start_room = plr_pos.x > start_min.x &&
        plr_pos.x < start_max.x && plr_pos.y > start_min.y &&
        plr_pos.y < start_max.y;

    struct camera* camera = &game->camera;
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

        struct v2 direction_to_mouse = v2_normalize(v2_direction(
            game->player.body->position, mouse->world));

        struct v2 target_pos = v2_average(mouse->world,
            game->player.body->position);

        f32 distance_to_target = v2_distance(target_pos,
            game->player.body->position);

        distance_to_target -= distance_to_activate;

        struct v2 target = game->player.body->position;

        if (distance_to_target > 0)
        {
            target.x = game->player.body->position.x +
                direction_to_mouse.x * distance_to_target;
            target.y = game->player.body->position.y +
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

    collision_map_dynamic_calculate(game);

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

void state_game_render(void* data)
{
    struct state_game_data* state = (struct state_game_data*)data;
    struct game_state* game = state->base;
    struct camera* camera = &game->camera;

    level_render(state->base, &game->level);
    player_render(state->base);
    enemies_render(state->base);
    items_render(state->base);
    particle_system_render(&game->particle_system, &game->particle_renderer);

    particle_renderer_sort(&game->particle_renderer);

    cube_renderer_flush(&game->cube_renderer, &camera->view,
        &camera->perspective);
    sprite_renderer_flush(&game->sprite_renderer, &camera->view,
        &camera->perspective);
    particle_renderer_flush(&game->particle_renderer, &camera->view,
        &camera->perspective);

    particle_lines_render(state->base);

    if (game->render_debug)
    {
        collision_map_render(state->base, state->base->cols.statics,
            state->base->cols.num_statics);
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
