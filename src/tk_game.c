#include "tk_game.h"

#include "tk_resources.c"
#include "tk_camera.c"

// Todo: create mesh_renderer that can be configured to work with particles,
// cubes, sprites and more
#include "tk_primitive_renderer.c"
#include "tk_sprite_renderer.c"
#include "tk_cube_renderer.c"
#include "tk_particle.c"

#include "tk_level.c"
#include "tk_collision.c"
#include "tk_physics.c"
#include "tk_input.c"

#include "tk_pathfind.c"
#include "tk_weapon.c"

#include "tk_enemy.c"
#include "tk_world.c"
#include "tk_entity.c"

#include "tk_renderer.c"

#include "tk_scene_game.c"
#include "tk_scene_physics.c"

void game_init(struct game_memory* memory, struct game_init* init)
{
    struct game_state* state = (struct game_state*)memory->base;

    // Init globals that use the game memory each time the game init is called
    _log = *init->log;
    _rng.seed = &state->random_seed;
    api = init->api;

    if (!memory->initialized)
    {
        // Init OpenGL
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

        LOG("OpenGL %i.%i\n", version_major, version_minor);
        LOG("Uniform blocks max vertex: %d\n", uniform_blocks_max_vertex);
        LOG("Uniform blocks max gemoetry: %d\n", uniform_blocks_max_geometry);
        LOG("Uniform blocks max fragment: %d\n", uniform_blocks_max_fragment);
        LOG("Uniform blocks max combined: %d\n", uniform_blocks_max_combined);
        LOG("Uniform buffer max bindings: %d\n", uniform_buffer_max_bindings);
        LOG("Uniform block max size: %d\n", uniform_block_max_size);
        LOG("Vertex attribs max: %d\n", vertex_attribs_max);

        api.gl.glEnable(GL_DEPTH_TEST);
        api.gl.glEnable(GL_BLEND);
        api.gl.glEnable(GL_CULL_FACE);
        api.gl.glDepthFunc(GL_LESS);
        api.gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        api.gl.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        // Init screen size
        state->screen_width = (f32)init->screen_width;
        state->screen_height = (f32)init->screen_height;

        // Init cameras
        state->camera_game.width = state->screen_width;
        state->camera_game.height = state->screen_height;
        state->camera_gui.width = state->screen_width;
        state->camera_gui.height = state->screen_height;

        // Init permanent stack allocator
        state->stack_permanent.base = (s8*)state + sizeof(struct game_state);
        state->stack_permanent.current = state->stack_permanent.base;
        state->stack_permanent.size = MEGABYTES(256);

        // Init temporary stack allocator
        state->stack_temporary.base = (s8*)state + sizeof(struct game_state) +
            state->stack_permanent.size;
        state->stack_temporary.current = state->stack_temporary.base;
        state->stack_temporary.size = MEGABYTES(256);

        // Init random seed
        state->random_seed = (u32)init->init_time;

        // Init resources
        state->shader = program_create(&state->stack_temporary,
            "assets/shaders/vertex.glsl",
            "assets/shaders/fragment.glsl");

        state->shader_simple = program_create(&state->stack_temporary,
            "assets/shaders/vertex.glsl",
            "assets/shaders/fragment_simple.glsl");

        state->shader_cube = program_create(&state->stack_temporary,
            "assets/shaders/vertex_cube.glsl",
            "assets/shaders/fragment_cube.glsl");

        state->shader_sprite = program_create(&state->stack_temporary,
            "assets/shaders/vertex_sprite.glsl",
            "assets/shaders/fragment_sprite.glsl");

        state->shader_particle = program_create(&state->stack_temporary,
            "assets/shaders/vertex_particle.glsl",
            "assets/shaders/fragment_particle.glsl");

        state->texture_tileset = texture_create(&state->stack_temporary,
            "assets/textures/tileset.tga");
        state->texture_sphere = texture_create(&state->stack_temporary,
            "assets/textures/sphere.tga");
        state->texture_cube = texture_array_create(&state->stack_temporary,
            "assets/textures/cube.tga", 4, 4);
        state->texture_sprite = texture_array_create(&state->stack_temporary,
            "assets/textures/tileset.tga", 8, 8);
        state->texture_particle = texture_array_create(&state->stack_temporary,
            "assets/textures/particles.tga", 8, 8);

        mesh_create(&state->stack_temporary, "assets/meshes/sphere.mesh",
            &state->sphere);
        mesh_create(&state->stack_temporary, "assets/meshes/wall.mesh",
            &state->wall);
        mesh_create(&state->stack_temporary, "assets/meshes/floor.mesh",
            &state->floor);
        mesh_create(&state->stack_temporary, "assets/meshes/triangle.mesh",
            &state->triangle);

        state->render_info_health_bar = (struct mesh_render_info)
        {
            colors[RED], &state->floor, state->texture_tileset,
            state->shader_simple
        };
        state->render_info_ammo_bar = (struct mesh_render_info)
        {
            colors[YELLOW], &state->floor, state->texture_tileset,
            state->shader_simple
        };
        state->render_info_weapon_bar = (struct mesh_render_info)
        {
            colors[LIME], &state->floor, state->texture_tileset,
            state->shader_simple
        };

        object_pool_init(&state->bullet_pool, sizeof(struct bullet),
            MAX_BULLETS, &state->stack_permanent);
        object_pool_init(&state->item_pool, sizeof(struct item),
            MAX_ITEMS, &state->stack_permanent);

        state->scene_game.base = state;
        scene_game_init(&state->scene_game);

        memory->initialized = true;
    }

    state->scene_physics = scene_physics_init(state);
    state->current_scene = SCENE_PHYSICS;

    if (!memory->initialized)
    {
        LOG("game_init: end of init, memory not initalized!\n");
    }
}

void game_update(struct game_memory* memory, struct game_input* input)
{
    if (memory->initialized)
    {
        struct game_state* state = (struct game_state*)memory->base;

        f32 step = 1.0f / 120.0f;

        state->accumulator += input->delta_time;

        while (state->accumulator >= step)
        {
            state->accumulator -= step;

            switch (state->current_scene)
            {
                case SCENE_GAME:
                {
                    scene_game_update(&state->scene_game, input, step);
                } break;
                case SCENE_PHYSICS:
                {
                    scene_physics_update(&state->scene_physics, input, step);
                } break;
            }

            u32 num_keys = sizeof(input->keys)/sizeof(input->keys[0]);

            for (u32 i = 0; i < num_keys; i++)
            {
                input->keys[i].transitions = 0;
            }
        }

        api.gl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        switch (state->current_scene)
        {
            case SCENE_GAME:
            {
                scene_game_render(&state->scene_game);
            } break;
            case SCENE_PHYSICS:
            {
                scene_physics_render(&state->scene_physics);
            } break;
        }
    }
    else
    {
        LOG("game_update: memory not initialized!\n");
    }
}
