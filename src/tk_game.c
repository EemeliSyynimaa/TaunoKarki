#include "tk_platform.h"

void game_init(struct game_memory* memory, struct game_init* init)
{
    struct api platform = init->api;

    platform.log("This is great!\n");
}

void game_update(struct game_memory* memory, struct game_input* input)
{
}
