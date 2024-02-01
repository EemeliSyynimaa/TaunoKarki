#include "tk_platform.h"
#include "tk_math.h"
#include "tk_state_interface.h"
#include "tk_random.h"
#include "tk_memory.h"
#include "tk_config.h"
#include "tk_color.h"
#include "tk_cube_renderer.h"
#include "tk_sprite_renderer.h"
#include "tk_entity.h"
#include "tk_physics.h"
#include "tk_camera.h"
#include "tk_mesh.h"
#include "tk_particle.h"
#include "tk_input.h"
#include "tk_collision.h"
#include "tk_level.h"

// Todo: global for now
struct api api;

// Todo: where to store these?
struct gun_shot
{
    struct v2 position;
    f32 volume; // Todo: change name
};

struct game_state
{
    struct physics_world world;
    struct player player;
    struct enemy enemies[MAX_ENEMIES];
    struct particle_line particle_lines[MAX_PARTICLES];
    struct camera camera;
    struct mouse mouse;
    struct mesh sphere;
    struct mesh wall;
    struct mesh floor;
    struct mesh triangle;
    struct memory_block stack_temporary;
    struct memory_block stack_permanent;
    struct v2 wall_corners[MAX_WALL_CORNERS];
    struct line_segment wall_faces[MAX_WALL_FACES];
    struct cube_renderer cube_renderer;
    struct sprite_renderer sprite_renderer;
    struct particle_renderer particle_renderer;
    struct particle_system particle_system;
    struct gun_shot gun_shots[MAX_GUN_SHOTS];
    struct level level;
    struct level level_mask;
    struct collision_map cols;
    struct object_pool bullet_pool;
    struct object_pool item_pool;
    struct state_interface* state_current;

    struct mesh_render_info render_info_health_bar;
    struct mesh_render_info render_info_ammo_bar;
    struct mesh_render_info render_info_weapon_bar;

    b32 render_debug;
    b32 level_change;
    b32 level_cleared;
    b32 player_in_start_room;
    f32 accumulator;
    f32 level_clear_notify;
    u32 shader;
    u32 shader_simple;
    u32 shader_cube;
    u32 shader_sprite;
    u32 shader_particle;
    u32 texture_tileset;
    u32 texture_sphere;
    u32 texture_cube;
    u32 texture_sprite;
    u32 texture_particle;
    u32 free_particle_line;
    u32 num_enemies;
    u32 num_wall_corners;
    u32 num_wall_faces;
    u32 num_gun_shots;
    u32 ticks_per_second;
    u32 level_current;
    u32 random_seed;
};
