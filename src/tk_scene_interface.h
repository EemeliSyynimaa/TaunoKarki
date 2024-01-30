#ifndef __TK_SCENE_INTERFACE_H__
#define __TK_SCENE_INTERFACE_H__

#include "tk_platform.h"

typedef void type_scene_init(void*);
typedef void type_scene_update(void*, struct game_input*, f32);
typedef void type_scene_render(void*);

struct scene_interface
{
    type_scene_init* init;
    type_scene_update* update;
    type_scene_render* render;
    void* data;
};

#endif
