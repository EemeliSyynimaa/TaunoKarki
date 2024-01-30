#include "tk_platform.h"
#include "tk_memory.h"
#include "tk_random.h"

#include "tk_camera.c"
#include "tk_resources.c"

#include "tk_scene_interface.h"
#include "tk_scene_physics.c"

struct game_state
{
    struct memory_block stack_permanent;
    struct memory_block stack_temporary;

    struct camera camera;
    struct random_number_generator rng;
    struct api api;

    b32 initialized;
    f32 accumulator;

    u32 shader_basic;
    u32 shader_simple;
    u32 shader_cube;
    u32 shader_sprite;
    u32 shader_particle;

    struct mesh mesh_sphere;
    struct mesh mesh_wall;
    struct mesh mesh_floor;
    struct mesh mesh_triangle;

    struct scene_interface scene_physics;
    struct scene_interface* scene_current;
};

void game_init(struct game_memory* memory, struct game_init* init)
{
    struct game_state* state = (struct game_state*)memory->base;

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

        // Init camera
        // Todo: test multiple cameras!
        state->camera.width = (f32)init->screen_width;
        state->camera.height = (f32)init->screen_height;

        // Init permanent stack allocator
        state->stack_permanent = stack_init(
            (u8*)state + sizeof(struct game_state), MEGABYTES(512));

        // Init temporary stack allocator (from the permanent stack)
        state->stack_temporary = stack_init(
            stack_alloc(&state->stack_permanent, MEGABYTES(256)),
            MEGABYTES(256));

        // Init random seed
        state->rng.seed = init->init_time;

        // Init resources
        state->shader_basic = program_create(&state->stack_temporary, &api,
            "assets/shaders/vertex.glsl",
            "assets/shaders/fragment.glsl");

        state->shader_simple = program_create(&state->stack_temporary, &api,
            "assets/shaders/vertex.glsl",
            "assets/shaders/fragment_simple.glsl");

        state->shader_cube = program_create(&state->stack_temporary, &api,
            "assets/shaders/vertex_cube.glsl",
            "assets/shaders/fragment_cube.glsl");

        state->shader_sprite = program_create(&state->stack_temporary, &api,
            "assets/shaders/vertex_sprite.glsl",
            "assets/shaders/fragment_sprite.glsl");

        state->shader_particle = program_create(&state->stack_temporary, &api,
            "assets/shaders/vertex_particle.glsl",
            "assets/shaders/fragment_particle.glsl");

        mesh_create(&state->stack_temporary, &api, "assets/meshes/sphere.mesh",
            &state->mesh_sphere);
        mesh_create(&state->stack_temporary, &api, "assets/meshes/wall.mesh",
            &state->mesh_wall);
        mesh_create(&state->stack_temporary, &api, "assets/meshes/floor.mesh",
            &state->mesh_floor);
        mesh_create(&state->stack_temporary, &api,
            "assets/meshes/triangle.mesh", &state->mesh_triangle);

        state->scene_physics = scene_physics_create(&state->stack_permanent);

        state->scene_current = &state->scene_physics;
        state->scene_current->init(state->scene_current->data);

        state->initialized = true;
    }
}

void game_update(struct game_memory* memory, struct game_input* input)
{
    struct game_state* state = (struct game_state*)memory->base;

    f32 step = 1.0f / 120.0f;

    state->accumulator += input->delta_time;

    while (state->accumulator >= step)
    {
        state->accumulator -= step;

        state->scene_current->update(state->scene_current->data, input, step);

        u32 num_keys = sizeof(input->keys)/sizeof(input->keys[0]);

        for (u32 i = 0; i < num_keys; i++)
        {
            input->keys[i].transitions = 0;
        }
    }

    state->api.gl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    state->scene_current->render(state->scene_current->data);
}
