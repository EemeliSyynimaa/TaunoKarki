#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include "scenemanager.h"
#include "assetmanager.h"

typedef struct tk_game_state_t
{
    uint32_t VAO;
    int32_t screen_width;
    int32_t screen_height;
    int32_t running;
    float step;
    float time_delta;
    float time_current;
    SceneManager scenes;
    AssetManager assets;
} tk_game_state_t;

#endif