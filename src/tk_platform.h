#include <stdint.h>
#include <stdbool.h>

#include <stdio.h>
#include <stdarg.h>

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef float       f32;
typedef double      f64;

typedef bool        b32;

#define KILOBYTES(X) ((X) * 1024)
#define MEGABYTES(X) (KILOBYTES(X) * 1024)
#define GIGABYTES(X) (MEGABYTES(X) * 1024)
#define TERABYTES(X) (GIGABYTES(X) * 1024)

#include "tk_opengl_api.h"
#include "tk_file_api.h"
#include "tk_time_api.h"

typedef void type_log(char*, ...);

struct key_state
{
    u32 transitions;
    b32 key_down;
};

struct game_input
{
    f32 delta_time;

    union 
    {
        struct key_state keys[13];

        struct
        {
            struct key_state move_up;
            struct key_state move_down;
            struct key_state move_right;
            struct key_state move_left;

            struct key_state reload;
            struct key_state shoot;

            struct key_state back;

            struct key_state weapon_slot_1;
            struct key_state weapon_slot_2;
            struct key_state weapon_slot_3;
            struct key_state weapon_slot_4;
            struct key_state weapon_slot_5;

            struct key_state weapon_pick;
        };
    };

    s32 mouse_x;
    s32 mouse_y;
    s32 mouse_wheel_delta;

    // Debug stuff
    b32 enable_debug_rendering;
    b32 pause;
};

struct api
{
    struct gl_api gl;
    struct file_functions file;
    struct time_functions time;
};

struct game_init
{
    struct api api;
    type_log* log;
    s32 screen_width;
    s32 screen_height;
    u64 init_time;
};

struct game_memory
{
    b32 initialized;
    void* base;
    u64 size;
};

type_log* _log;

#define LOG(...) _log(__VA_ARGS__);
