#include "tk_platform.h"
#include "tk_opengl.c"
#include "tk_camera.c"

// Todo: possibly move this to tk_game.h
struct game_state
{
    b32 initialized;

    struct camera camera;
};

void game_init(struct game_memory* memory, struct game_init* init)
{
    struct game_state* state = (struct game_state*)memory;

    struct api api = init->api;

    if (!state->initialized)
    {
        opengl_init(api);

        state->initialized = true;

        // Todo: test multiple cameras!
        state->camera.width = (f32)init->screen_width;
        state->camera.height = (f32)init->screen_height;
    }
}

void game_update(struct game_memory* memory, struct game_input* input)
{
    struct game_state* state = (struct game_state*)memory;
}
