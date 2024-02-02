// Todo: maybe rename these to world, floor or smth...
void level_generate(struct memory_block* stack, struct level* level,
    struct level* layout_mask)
{
    u32 width = layout_mask->width;
    u32 height = layout_mask->height;
    u32 start_x = (u32)layout_mask->start_pos.x;
    u32 start_y = (u32)layout_mask->start_pos.y;

    if (!width || !height)
    {
        LOG("Error: no level width or height set: %u x %u\n", width, height);
        return;
    }

    u64 size = width * height;
    u8* data = 0;

    data = stack_alloc(stack, size);
    memory_copy(layout_mask->tile_types, data, size);

    memory_set(level, sizeof(struct level), 0);

    // Generate randomized level
    // Each room will be numbered, starting from three
    u32 room_index = 3;

    for (u32 y = 0; y < height; y++)
    {
        for (u32 x = 0; x < width; x++)
        {
            u32 index = y * width + x;

            // Don't create room on an empty space (zeroed)
            if (!data[index])
            {
                continue;
            }

            while (true)
            {
                u32 action = u32_random(0, 2);

                // Continue previous room
                if (action == 0)
                {
                    u32 prev = index - 1;

                    // Skip check if previous room is an empty space
                    if (x == 0 || !data[prev])
                    {
                        continue;
                    }

                    data[index] = data[prev];
                }
                // Continue upper room
                else if (action == 1)
                {
                    u32 prev = index - width;

                    // Skip check if upper room is an empty space
                    if (y == 0 || !data[prev])
                    {
                        continue;
                    }

                    data[index] = data[prev];
                }
                else
                {
                    data[index] = room_index++;
                }

                break;
            }
        }
    }

    // Start room
    data[start_y * width + start_x] = 2;

    for (u32 y = 0; y < height; y++)
    {
        for (u32 x = 0; x < width; x++)
        {
            LOG("%d, ", data[y * width + x]);
        }

        LOG("\n");
    }

    // Room dimensions, always multiple of two!
    u32 room_width = 4;
    u32 room_height = 4;

    // Calculate level dimensions; +1 for outer wall
    level->width = height * room_height + 1;
    level->height = width * room_width + 1;

    // Create walls around rooms
    for (u32 y = 0; y < height; y++)
    {
        for (u32 x = 0; x < width; x++)
        {
            u32 room_index = y * width + x;
            u32 room_type = data[y * width + x];

            for (u32 j = 0; j < room_height; j++)
            {
                for (u32 k = 0; k < room_width; k++)
                {
                    u32 tile_x = room_width * x + k;
                    u32 tile_y = room_height * y + j;
                    u32 tile_type = room_type;
                    u32 tile_index = tile_y * level->width + tile_x;
                    s32 tile_left = tile_index - 1;
                    s32 tile_up = tile_index - level->width;
                    s32 room_left = room_index - 1;
                    s32 room_up = room_index - width;

                    if (j == 0 && k == 0 && tile_left >= 0 && tile_up >= 0 &&
                        level->tile_types[tile_left] == TILE_WALL &&
                        level->tile_types[tile_up] == TILE_WALL)
                    {
                        tile_type = TILE_WALL;
                    }
                    else if (room_type == TILE_NOTHING)
                    {
                        tile_type = TILE_NOTHING;

                        if (k == 0 && room_left >= 0 && x != 0 &&
                            data[room_left] != TILE_NOTHING)
                        {
                            tile_type = TILE_WALL;
                        }

                        if (j == 0 && room_up >= 0 && y != 0 &&
                            data[room_up] != TILE_NOTHING)
                        {
                            tile_type = TILE_WALL;
                        }
                    }
                    else if (tile_x == 0 || tile_y == 0 ||
                        (k == 0 && data[room_left] != room_type) ||
                        (j == 0 && data[room_up] != room_type))
                    {
                        tile_type = TILE_WALL;
                    }

                    level->tile_types[tile_index] = tile_type;
                }
            }
        }
    }

    // Set outer wall
    for (u32 x = 0, y = level->height - 1; x < level->width; x++)
    {
        u32 tile_index = y * level->width + x;
        u32 tile_prev = tile_index - level->width;
        u32 tile_type = level->tile_types[tile_prev] == TILE_NOTHING ?
            TILE_NOTHING : TILE_WALL;

        level->tile_types[tile_index] = tile_type;
    }

    for (u32 y = 0, x = level->width - 1; y < level->height; y++)
    {
        u32 tile_index = y * level->width + x;
        u32 tile_prev = tile_index - 1;
        u32 tile_type = level->tile_types[tile_prev] == TILE_NOTHING ?
            TILE_NOTHING : TILE_WALL;

        level->tile_types[tile_index] = tile_type;
    }

    // Find doors
    for (u32 i = 3; i < room_index - 1; i++)
    {
        for (u32 j = i + 1; j < room_index; j++)
        {
            struct v2 tile_i = tile_random_get(level, i);
            struct v2 tile_j = tile_random_get(level, j);

            u32 walls[256] = { 0 };
            u32 wall_count = 0;

            // Find each wall block between rooms i and j
            for (u32 y = 1; y < level->height - 1; y++)
            {
                for (u32 x = 1; x < level->width - 1; x++)
                {
                    u32 tile_index = y * level->width + x;

                    if (level->tile_types[tile_index] == TILE_WALL)
                    {
                        u32 tile_prev = level->tile_types[tile_index - 1];
                        u32 tile_next = level->tile_types[tile_index + 1];
                        u32 tile_up   = level->tile_types[tile_index -
                            level->width];
                        u32 tile_down = level->tile_types[tile_index +
                            level->width];

                        if ((i == tile_prev && j == tile_next) ||
                            (i == tile_next && j == tile_prev) ||
                            (i == tile_up && j == tile_down) ||
                            (i == tile_down && j == tile_up))
                        {
                            walls[wall_count++] = tile_index;
                        }
                    }
                }
            }

            // Pick a random block and mark it as door
            if (wall_count)
            {
                u32 path_max_length = 20;
                u32 path_length = path_find(level, tile_i, tile_j, NULL, 0);

                if (!path_length || path_length > path_max_length)
                {
                    u32 num_doors_to_open = 1;

                    if (wall_count > 8)
                    {
                        num_doors_to_open = 3;
                    }
                    else if (wall_count > 4)
                    {
                        num_doors_to_open = 2;
                    }

                    u32 random_doors[3];

                    for (u32 k = 0; k < num_doors_to_open; k++)
                    {
                        u32 random_door = u32_random(0, wall_count - 1);
                        b32 door_picked_already = false;

                        for (u32 l = 0; l < k; l++)
                        {
                            s32 difference = random_door - random_doors[l];

                            if (difference <= 1 && difference >= -1)
                            {
                                door_picked_already = true;
                                break;
                            }
                        }

                        if (!door_picked_already)
                        {
                            random_doors[k] = random_door;

                            level->tile_types[walls[random_door]] = TILE_FLOOR;
                            level->tile_sprites[walls[random_door]] = 16;
                        }
                    }
                }
            }
        }
    }

    // Open start room door
    s32 room_center_x = (u32)(room_width * 0.5f);
    s32 room_center_y = (u32)(room_height * 0.5f);

    level->start_pos.x = start_x * room_width + (f32)room_center_x;
    level->start_pos.y = start_y * room_height + (f32)room_center_y;

    // Always pointing south
    u32 start_door_index = (start_y * room_height + room_center_y +
        -room_center_y) * level->width + start_x * room_width + room_center_x;

    level->tile_types[start_door_index] = TILE_FLOOR;
    level->tile_sprites[start_door_index] = 16;

    // Create floors
    for (u32 i = 0; i < level->width * level->height; i++)
    {
        u8 type = level->tile_types[i];

        if (!level->tile_sprites[i])
        {
            if (type == TILE_WALL)
            {
                level->tile_sprites[i] = 8;
            }
            else if (type == 2)
            {
                // Use specific tile for start room
                level->tile_types[i] = TILE_START;
                level->tile_sprites[i] = 17;
            }
            else if (type > 2)
            {
                level->tile_types[i] = TILE_FLOOR;
                level->tile_sprites[i] = 8 + type % 8;
            }
        }
        else if (level->tile_sprites[i] == 16)
        {
            level->tile_types[i] = TILE_DOOR;
        }
    }

    for (u32 y = 0; y < level->height; y++)
    {
        for (u32 x = 0; x < level->width; x++)
        {
            LOG("%d, ", level->tile_sprites[y * level->width + x]);
        }

        LOG("\n");
    }

    stack_free(stack);
}

void level_render(struct level* level, struct sprite_renderer* sprite_renderer,
    struct cube_renderer* cube_renderer, struct m4 projection, struct m4 view)
{
    // Todo: fix level rendering glitch (a wall block randomly drawn in a
    //       wrong place)

    struct sprite_data data;
    data.color = colors[WHITE];

    for (u32 y = 0; y < level->height; y++)
    {
        for (u32 x = 0; x < level->width; x++)
        {
            f32 top = y + TILE_WALL * 0.5f;
            f32 bottom = y - TILE_WALL * 0.5f;
            f32 left = x - TILE_WALL * 0.5f;
            f32 right = x + TILE_WALL * 0.5f;

            struct v2 corners[] =
            {
                { left, top },
                { right, top },
                { right, bottom },
                { left, bottom }
            };

            u64 tile_index = y * level->width + x;
            u8 tile_type = level->tile_types[tile_index];

            if (tile_type == TILE_WALL)
            {
                struct m4 transform = m4_translate((f32)x, (f32)y, 0.5f);
                struct m4 rotation = m4_rotate_z(0.0f);
                struct m4 scale = m4_scale_all(WALL_SIZE * 0.5f);

                struct m4 model = m4_mul_m4(scale, rotation);
                model = m4_mul_m4(model, transform);

                struct m4 mvp = m4_mul_m4(model, view);
                mvp = m4_mul_m4(mvp, projection);

                mesh_render(&level->wall_info, &mvp);
            }
            else if (tile_type != TILE_NOTHING)
            {
                struct m4 transform = m4_translate((f32)x, (f32)y, 0.0f);
                struct m4 rotation = m4_rotate_z(0.0f);
                struct m4 scale = m4_scale_all(WALL_SIZE * 0.5f);

                struct m4 model = m4_mul_m4(scale, rotation);
                model = m4_mul_m4(model, transform);

                data.model = model;
                data.texture = level->tile_sprites[tile_index];
                data.color = colors[WHITE];

                sprite_renderer_add(sprite_renderer, &data);
            }
        }
    }

    cube_renderer_add(cube_renderer, &level->elevator_light);
}

void level_mask_init(struct level* mask)
{
    // Inited once per game round e.g. until the player has died

    // Todo: now hard coded, randomize in the future
    mask->width = 8;
    mask->height = 8;
    mask->start_pos = (struct v2) { 1.0f, 1.0f };

    // Level mask mask is used to make each level structurally compatible.
    // The outer walls and start room location will be the same for each
    // level but the inner walls may differ.
    {
        u32 width = mask->width;
        u32 height = mask->height;
        u32 start_x = (u32)mask->start_pos.x;
        u32 start_y = (u32)mask->start_pos.y;

        u64 size = width * height;
        u8* data = mask->tile_types;

        memory_set(data, size, 1);

        {
            data[5 * width + 3] = 0;
            data[5 * width + 4] = 0;
            data[6 * width + 3] = 0;
            data[6 * width + 4] = 0;
        }

        // Reserve space for the starting room
        data[start_y * width + start_x] = 0;
    }
}

void level_init(struct game_state* state)
{
    // Clear everything
    object_pool_reset(&state->bullet_pool);
    object_pool_reset(&state->item_pool);

    memory_set(state->enemies,
        sizeof(struct enemy) * MAX_ENEMIES, 0);
    memory_set(state->bullet_trails,
        sizeof(struct bullet_trail) * MAX_BULLET_TRAILS, 0);
    memory_set(state->wall_corners,
        sizeof(struct v2) * MAX_WALL_CORNERS, 0);
    memory_set(state->wall_faces,
        sizeof(struct line_segment) * MAX_WALL_FACES, 0);
    memory_set(&state->cols, sizeof(struct collision_map), 0);
    memory_set(state->gun_shots,
        sizeof(struct gun_shot) * MAX_GUN_SHOTS, 0);

    state->num_enemies = 0;
    state->num_wall_corners = 0;
    state->num_wall_faces = 0;
    state->num_gun_shots = 0;
    state->free_bullet_trail = 0;

    // Inited once per level
    u32 enemies_min = state->level_current;
    u32 enemies_max =  MIN(enemies_min * 4, MAX_ENEMIES);
    state->num_enemies = u32_random(enemies_min, enemies_max);
    // state->num_enemies = 1;

    LOG("%u enemies\n", state->num_enemies);

    level_generate(&state->stack_temporary, &state->level, &state->level_mask);

    state->level.wall_info.mesh = &state->wall;
    state->level.wall_info.texture = state->texture_tileset;
    state->level.wall_info.shader = state->shader;
    state->level.wall_info.color = colors[WHITE];

    u32 color_enemy = cube_renderer_color_add(&state->cube_renderer,
        (struct v4){ 0.7f, 0.90f, 0.1f, 1.0f });
    u32 color_player = cube_renderer_color_add(&state->cube_renderer,
        (struct v4){ 1.0f, 0.4f, 0.9f, 1.0f });

    world_init(&state->world);

    for (u32 i = 0; i < state->num_enemies; i++)
    {
        struct enemy* enemy = &state->enemies[i];
        struct rigid_body* body = entity_add_body(&enemy->header,
            &state->world);
        body->type = RIGID_BODY_DYNAMIC;
        body->position = tile_random_get(&state->level, TILE_FLOOR);
        body->friction = FRICTION;
        enemy->header.type = ENTITY_ENEMY;
        enemy->alive = true;
        enemy->health = ENEMY_HEALTH_MAX;
        enemy->vision_cone_size = 0.2f * i;
        enemy->shooting = false;
        enemy->cube.faces[0].texture = 13;
        enemy->state = u32_random(0, 1) ? ENEMY_STATE_SLEEP :
            ENEMY_STATE_WANDER_AROUND;
        body_add_circle_collider(body, v2_zero, PLAYER_RADIUS, COLLISION_ENEMY,
            (COLLISION_ENEMY | COLLISION_PLAYER | COLLISION_WALL));
        body_add_rect_collider(body, v2_zero, PLAYER_RADIUS, PLAYER_RADIUS,
            COLLISION_ENEMY_HITBOX, COLLISION_NONE);

        LOG("Enemy %u is %s\n", i,
            enemy->state == ENEMY_STATE_SLEEP ? "sleeping" : "wandering");

        cube_data_color_update(&enemy->cube, color_enemy);

        enemy->weapon = weapon_create(u32_random(1, 3));
        enemy->weapon.projectile_damage *= 0.2f;
    }

    if (state->level_current == 1)
    {
        state->player.weapon = weapon_create(WEAPON_MACHINEGUN);
    }

    if (!state->player.alive)
    {
        struct rigid_body* body = entity_add_body(&state->player.header,
            &state->world);
        body->type = RIGID_BODY_DYNAMIC;
        body->position = state->level.start_pos;
        body->friction = FRICTION;
        state->player.header.type = ENTITY_PLAYER;
        state->player.alive = true;
        state->player.health = PLAYER_HEALTH_MAX;
        state->player.cube.faces[0].texture = 11;
        body_add_circle_collider(body, v2_zero, PLAYER_RADIUS, COLLISION_PLAYER,
            (COLLISION_ENEMY | COLLISION_ITEM | COLLISION_WALL));
        body_add_rect_collider(body, v2_zero, PLAYER_RADIUS, PLAYER_RADIUS,
            COLLISION_PLAYER_HITBOX, COLLISION_NONE);
    }

    cube_data_color_update(&state->player.cube, color_player);

    state->mouse.world = state->player.header.body->position;

    struct camera* camera = &state->camera_game;
    camera->position.xy = state->level.start_pos;
    camera->target.xy = state->level.start_pos;
    camera->target.z = 3.75f;
    camera->view = m4_translate(-camera->position.x, -camera->position.y,
        -camera->position.z);
    camera->view_inverse = m4_inverse(camera->view);

    collision_map_static_calculate(&state->level, state->cols.statics,
        MAX_STATICS, &state->cols.num_statics);

    // state->world.walls = state->cols.statics;
    // state->world.num_walls = state->cols.num_statics;

    world_wall_bodies_create(&state->world, state->cols.statics,
        state->cols.num_statics);

    LOG("Wall faces: %d/%d\n", state->cols.num_statics, MAX_STATICS);

    get_wall_corners_from_faces(state->wall_corners, MAX_WALL_CORNERS,
        &state->num_wall_corners, state->cols.statics,
        state->cols.num_statics);

    LOG("Wall corners: %d/%d\n", state->num_wall_corners, MAX_WALL_CORNERS);

    {
        struct m4 transform = m4_translate(state->level.start_pos.x,
            state->level.start_pos.y + 1.5f, 0.5f);
        struct m4 rotation = m4_rotate_z(0.0f);
        struct m4 scale = m4_scale_xyz(0.25f, 0.125f, 0.125f);
        struct m4 model = m4_mul_m4(scale, rotation);
        model = m4_mul_m4(model, transform);

        state->level.elevator_light.model = model;

        cube_data_color_update(&state->level.elevator_light, RED);
    }
}
