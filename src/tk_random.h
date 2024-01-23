#ifndef __TK_RANDOM_H__
#define __TK_RANDOM_H__

#include "tk_platform.h"
#include "tk_math.h"

// Todo: this is in global space, scary
struct random_number_generator
{
    u64 seed;
};


static u64 random(struct random_number_generator* rng)
{
    u64 result = rng->seed = rng->seed * 48271 % 0x7fffffff;

    return result;
}

static u32 u32_random(struct random_number_generator* rng, u32 min, u32 max)
{
    u32 result = random(rng) % (max - min) + min;

    return result;
}

static f32 f32_random(struct random_number_generator* rng, f32 min, f32 max)
{
    f32 result = 0.0f;
    f32 rand = u32_random(rng, 0, S32_MAX) / (f32)S32_MAX;
    result = min + rand * (max - min);

    return result;
}

#endif
