b32 tile_inside_level_bounds(struct level* level, struct v2 position)
{
    b32 result = false;

    s32 x = (s32)f32_round(position.x);
    s32 y = (s32)f32_round(position.y);

    result = x >= 0 && x <(s32) level->width && y >= 0 &&
        y < (s32)level->height;

    return result;
}

b32 tile_is_of_type(struct level* level, struct v2 position, u32 type)
{
    b32 result = false;

    if (tile_inside_level_bounds(level, position))
    {
        s32 x = (s32)f32_round(position.x);
        s32 y = (s32)f32_round(position.y);

        result = level->tile_types[y * level->width + x] == type;
    }

    return result;
}

u8 tile_type_get(struct level* level, struct v2 position)
{
    u8 result = 0;

    if (tile_inside_level_bounds(level, position))
    {
        s32 x = (s32)f32_round(position.x);
        s32 y = (s32)f32_round(position.y);

        result = level->tile_types[y * level->width + x];
    }

    return result;
}

b32 tile_is_free(struct level* level, struct v2 position)
{
    return tile_type_get(level, position) > TILE_WALL;
}

struct v2 tile_random_get(struct level* level, u32 type)
{
    struct v2 result = { 0 };

    u32 max_iterations = 1000;

    while (max_iterations--)
    {
        struct v2 position =
        {
            (f32)u32_random(0, level->width),
            (f32)u32_random(0, level->height)
        };

        if (tile_is_of_type(level, position, type))
        {
            result = position;

            break;
        }
    };

    return result;
}
