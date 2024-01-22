#include "tk_platform.h"
#include "tk_camera.c"

// Todo: possibly move this to tk_game.h
struct game_state
{
    b32 initialized;
    f32 accumulator;

    struct camera camera;
    struct api api;
};

void game_init(struct game_memory* memory, struct game_init* init)
{
    struct game_state* state = (struct game_state*)memory;

    struct api api = state->api = init->api;

    if (!state->initialized)
    {
        // Init opengl
        s32 version_major = 0;
        s32 version_minor = 0;
        s32 uniform_blocks_max_vertex = 0;
        s32 uniform_blocks_max_geometry = 0;
        s32 uniform_blocks_max_fragment = 0;
        s32 uniform_blocks_max_combined = 0;
        s32 uniform_buffer_max_bindings = 0;
        s32 uniform_block_max_size = 0;
        s32 vertex_attribs_max = 0;

        api.gl.glGetIntegerv(GL_MAJOR_VERSION, &version_major);
        api.gl.glGetIntegerv(GL_MINOR_VERSION, &version_minor);
        api.gl.glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS,
            &uniform_blocks_max_vertex);
        api.gl.glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS,
            &uniform_blocks_max_geometry);
        api.gl.glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS,
            &uniform_blocks_max_fragment);
        api.gl.glGetIntegerv(GL_MAX_COMBINED_UNIFORM_BLOCKS,
            &uniform_blocks_max_combined);
        api.gl.glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS,
            &uniform_buffer_max_bindings);
        api.gl.glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE,
            &uniform_block_max_size);
        api.gl.glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &vertex_attribs_max);

        api.log("OpenGL %i.%i\n", version_major, version_minor);
        api.log("Uniform blocks max vertex: %d\n", uniform_blocks_max_vertex);
        api.log("Uniform blocks max gemoetry: %d\n",
            uniform_blocks_max_geometry);
        api.log("Uniform blocks max fragment: %d\n",
            uniform_blocks_max_fragment);
        api.log("Uniform blocks max combined: %d\n",
            uniform_blocks_max_combined);
        api.log("Uniform buffer max bindings: %d\n",
            uniform_buffer_max_bindings);
        api.log("Uniform block max size: %d\n", uniform_block_max_size);
        api.log("Vertex attribs max: %d\n", vertex_attribs_max);

        api.gl.glEnable(GL_DEPTH_TEST);
        api.gl.glEnable(GL_BLEND);
        api.gl.glEnable(GL_CULL_FACE);
        api.gl.glDepthFunc(GL_LESS);
        api.gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        api.gl.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        state->initialized = true;

        // Todo: test multiple cameras!
        state->camera.width = (f32)init->screen_width;
        state->camera.height = (f32)init->screen_height;
    }
}

void game_update(struct game_memory* memory, struct game_input* input)
{
    struct game_state* state = (struct game_state*)memory;

    f32 step = 1.0f / 120.0f;

    state->accumulator += input->delta_time;

    while (state->accumulator >= step)
    {
        state->accumulator -= step;

        // Todo: update current state

        u32 num_keys = sizeof(input->keys)/sizeof(input->keys[0]);

        for (u32 i = 0; i < num_keys; i++)
        {
            input->keys[i].transitions = 0;
        }
    }

    state->api.gl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Todo: render current state
}
