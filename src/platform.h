#ifndef TK_PLATFORM_H
#define TK_PLATFORM_H

#include <stdint.h>

int32_t tk_current_time_get();

typedef void tk_sound_sample;

int32_t tk_sound_play(tk_sound_sample* sample, int32_t channel = -1, 
    int32_t loops = 0);

int32_t tk_sound_is_playing(int32_t channel);

// #include <stdint.h>

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;

typedef int8_t      u8;
typedef int16_t     u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef float       f32;
typedef double      f64;

typedef int32_t     b32;

typedef struct key_state
{
    u32 transitions;
    b32 key_down;
} key_state;

typedef struct game_input
{
    f32 delta_time;
    f32 current_time;

    union 
    {
        key_state keys[7];

        struct
        {
            key_state move_up;
            key_state move_down;
            key_state move_right;
            key_state move_left;

            key_state reload;
            key_state shoot;

            key_state back;
        };
    };

    s32 mouse_x;
    s32 mouse_y;
} game_input;

void init_game(s32 screen_width, s32 screen_height);
void update_game(game_input* input);

#endif