struct mouse
{
    struct v2 world;
    struct v2 screen;
};


u32 key_times_pressed(struct key_state* state)
{
    u32 result = 0;

    result = state->transitions / 2;

    if (state->key_down && state->transitions % 2)
    {
        result++;
    }

    return result;
}

f32 time_elapsed_seconds(u64 ticks_start, u64 ticks_end)
{
    f32 result = (ticks_end - ticks_start) / 1000000000.0f;

    return result;
}
