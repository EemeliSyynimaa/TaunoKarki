#if 0
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
#endif
