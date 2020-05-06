#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <stdbool.h>

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

typedef struct key_state
{
    u32 transitions;
    b32 key_down;
} key_state;

typedef struct game_input
{
    f32 delta_time;

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

typedef struct game_memory
{
    void* base;
    u64 size;
} game_memory;

typedef u64 file_handle;

void file_open(file_handle* file, s8* path);
void file_close(file_handle* file);
void file_read(file_handle* file, s8* data, u64 bytes_max, u64* bytes_read);
void file_size_get(file_handle* file, u64* file_size);

void debug_log(s8* format, ...);


#endif