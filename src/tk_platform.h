#ifndef TK_PLATFORM_H
#define TK_PLATFORM_H

#include <stdint.h>

int32_t tk_current_time_get();

typedef struct
{
    int32_t player_move_up;
    int32_t player_move_down;
    int32_t player_move_right;
    int32_t player_move_left;
    int32_t player_reload;
    int32_t player_shoot;

    int32_t menu_escape;
    int32_t menu_up;
    int32_t menu_down;
    int32_t menu_confirm;

    int32_t mouse_x;
    int32_t mouse_y;
} tk_state_player_input_t;

typedef void tk_sound_sample;

int32_t tk_sound_play(tk_sound_sample* sample, int32_t channel = -1, 
    int32_t loops = 0);

int32_t tk_sound_is_playing(int32_t channel);

#endif