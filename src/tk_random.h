#if 0
// Todo: this is in global space, scary
struct random_number_generator
{
    u32* seed;
} _rng;


u32 random()
{
    u32 result = *_rng.seed = (u64)*_rng.seed * 48271 % 0x7fffffff;

    return result;
}

u32 u32_random(u32 min, u32 max)
{
    u32 result = random() % (max - min + 1) + min;

    return result;
}

f32 f32_random(f32 min, f32 max)
{
    f32 result = 0.0f;
    f32 rand = u32_random(0, S32_MAX) / (f32)S32_MAX;
    result = min + rand * (max - min);

    return result;
}
#endif
