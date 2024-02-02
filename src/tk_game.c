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
#include "tk_world.c"

#include "tk_enemy.c"
#include "tk_entity.c"

void collision_map_render(struct game_state* state, struct line_segment* cols,
    u32 num_cols, struct camera* camera)
{
    struct mesh_render_info info = { 0 };
    info.color = colors[RED];
    info.mesh = &state->floor;
    info.texture = state->texture_tileset;
    info.shader = state->shader_simple;

    struct m4 vp = m4_mul_m4(camera->view, camera->projection);

    for (u32 i = 0; i < num_cols; i++)
    {
        line_render(&info, cols[i].start, cols[i].end, WALL_SIZE + 0.01f,
            0.025f, vp);
    }
}

void level_init(struct game_state* state)
{
    // Clear everything
    object_pool_reset(&state->bullet_pool);
    object_pool_reset(&state->item_pool);

    memory_set(state->enemies,
        sizeof(struct enemy) * MAX_ENEMIES, 0);
    memory_set(state->bullet_trails,
        sizeof(struct bullet_trail) * MAX_BULLET_TRAILS, 0);
    memory_set(state->wall_corners,
        sizeof(struct v2) * MAX_WALL_CORNERS, 0);
    memory_set(state->wall_faces,
        sizeof(struct line_segment) * MAX_WALL_FACES, 0);
    memory_set(&state->cols, sizeof(struct collision_map), 0);
    memory_set(state->gun_shots,
        sizeof(struct gun_shot) * MAX_GUN_SHOTS, 0);

    state->num_enemies = 0;
    state->num_wall_corners = 0;
    state->num_wall_faces = 0;
    state->num_gun_shots = 0;
    state->free_bullet_trail = 0;

    // Inited once per level
    u32 enemies_min = state->level_current;
    u32 enemies_max =  MIN(enemies_min * 4, MAX_ENEMIES);
    state->num_enemies = u32_random(enemies_min, enemies_max);
    // state->num_enemies = 1;

    LOG("%u enemies\n", state->num_enemies);

    level_generate(&state->stack_temporary, &state->level, &state->level_mask);

    state->level.wall_info.mesh = &state->wall;
    state->level.wall_info.texture = state->texture_tileset;
    state->level.wall_info.shader = state->shader;
    state->level.wall_info.color = colors[WHITE];

    u32 color_enemy = cube_renderer_color_add(&state->cube_renderer,
        (struct v4){ 0.7f, 0.90f, 0.1f, 1.0f });
    u32 color_player = cube_renderer_color_add(&state->cube_renderer,
        (struct v4){ 1.0f, 0.4f, 0.9f, 1.0f });

    world_init(&state->world);

    for (u32 i = 0; i < state->num_enemies; i++)
    {
        struct enemy* enemy = &state->enemies[i];
        struct rigid_body* body = entity_add_body(&enemy->header,
            &state->world);
        body->type = RIGID_BODY_DYNAMIC;
        body->position = tile_random_get(&state->level, TILE_FLOOR);
        body->friction = FRICTION;
        enemy->header.type = ENTITY_ENEMY;
        enemy->alive = true;
        enemy->health = ENEMY_HEALTH_MAX;
        enemy->vision_cone_size = 0.2f * i;
        enemy->shooting = false;
        enemy->cube.faces[0].texture = 13;
        enemy->state = u32_random(0, 1) ? ENEMY_STATE_SLEEP :
            ENEMY_STATE_WANDER_AROUND;
        body_add_circle_collider(body, v2_zero, PLAYER_RADIUS, COLLISION_ENEMY,
            (COLLISION_ENEMY | COLLISION_PLAYER | COLLISION_WALL));
        body_add_rect_collider(body, v2_zero, PLAYER_RADIUS, PLAYER_RADIUS,
            COLLISION_ENEMY_HITBOX, COLLISION_NONE);

        LOG("Enemy %u is %s\n", i,
            enemy->state == ENEMY_STATE_SLEEP ? "sleeping" : "wandering");

        cube_data_color_update(&enemy->cube, color_enemy);

        enemy->weapon = weapon_create(u32_random(1, 3));
        enemy->weapon.projectile_damage *= 0.2f;
    }

    if (state->level_current == 1)
    {
        state->player.weapon = weapon_create(WEAPON_MACHINEGUN);
    }

    if (!state->player.alive)
    {
        struct rigid_body* body = entity_add_body(&state->player.header,
            &state->world);
        body->type = RIGID_BODY_DYNAMIC;
        body->position = state->level.start_pos;
        body->friction = FRICTION;
        state->player.header.type = ENTITY_PLAYER;
        state->player.alive = true;
        state->player.health = PLAYER_HEALTH_MAX;
        state->player.cube.faces[0].texture = 11;
        body_add_circle_collider(body, v2_zero, PLAYER_RADIUS, COLLISION_PLAYER,
            (COLLISION_ENEMY | COLLISION_ITEM | COLLISION_WALL));
        body_add_rect_collider(body, v2_zero, PLAYER_RADIUS, PLAYER_RADIUS,
            COLLISION_PLAYER_HITBOX, COLLISION_NONE);
    }

    cube_data_color_update(&state->player.cube, color_player);

    state->mouse.world = state->player.header.body->position;

    struct camera* camera = &state->camera_game;
    camera->position.xy = state->level.start_pos;
    camera->target.xy = state->level.start_pos;
    camera->target.z = 3.75f;
    camera->view = m4_translate(-camera->position.x, -camera->position.y,
        -camera->position.z);
    camera->view_inverse = m4_inverse(camera->view);

    collision_map_static_calculate(&state->level, state->cols.statics,
        MAX_STATICS, &state->cols.num_statics);

    // state->world.walls = state->cols.statics;
    // state->world.num_walls = state->cols.num_statics;

    world_wall_bodies_create(&state->world, state->cols.statics,
        state->cols.num_statics);

    LOG("Wall faces: %d/%d\n", state->cols.num_statics, MAX_STATICS);

    get_wall_corners_from_faces(state->wall_corners, MAX_WALL_CORNERS,
        &state->num_wall_corners, state->cols.statics,
        state->cols.num_statics);

    LOG("Wall corners: %d/%d\n", state->num_wall_corners, MAX_WALL_CORNERS);

    {
        struct m4 transform = m4_translate(state->level.start_pos.x,
            state->level.start_pos.y + 1.5f, 0.5f);
        struct m4 rotation = m4_rotate_z(0.0f);
        struct m4 scale = m4_scale_xyz(0.25f, 0.125f, 0.125f);
        struct m4 model = m4_mul_m4(scale, rotation);
        model = m4_mul_m4(model, transform);

        state->level.elevator_light.model = model;

        cube_data_color_update(&state->level.elevator_light, RED);
    }
}

#include "tk_state_game.c"
// #include "tk_state_physics.c"

b32 PHYSICS_DEBUG = false;

struct state_interface state_physics;
struct state_interface state_game;

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
        state->camera_game.width = (f32)init->screen_width;
        state->camera_game.height = (f32)init->screen_height;
        state->camera_gui.width = (f32)init->screen_width;
        state->camera_gui.height = (f32)init->screen_height;

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

        // state_physics = state_physics_create(state);
        state_game = state_game_create(state);

        state->state_current = PHYSICS_DEBUG ? &state_physics : &state_game;
        state->state_current->init(state->state_current->data);

        memory->initialized = true;
    }

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
            state->state_current->update(state->state_current->data, input,
                step);

            u32 num_keys = sizeof(input->keys)/sizeof(input->keys[0]);

            for (u32 i = 0; i < num_keys; i++)
            {
                input->keys[i].transitions = 0;
            }
        }

        api.gl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        state->state_current->render(state->state_current->data);
    }
    else
    {
        LOG("game_update: memory not initialized!\n");
    }
}
