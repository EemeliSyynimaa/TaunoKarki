#include "tk_platform.h"
#include "tk_math.h"

#include <string.h>

struct memory_block
{
    u64 size;
    s8* base;
    s8* current;
};

void* stack_alloc(struct memory_block* block, u64 size)
{
    // Todo: add alignment
    u64 bytes_left = (block->base + block->size) - block->current;
    u64 bytes_needed = size + sizeof(u64);

    if (bytes_needed > bytes_left)
    {
        LOG("Not enough memory\n");

        return 0;
    }

    s8* result = block->current;

    block->current += size;

    *((u64*)block->current) = size;

    block->current += sizeof(u64);

    return result;
}

void* stack_free(struct memory_block* block)
{
    if (block->current > block->base)
    {
        block->current -= sizeof(u64);

        u64 size = *((u64*)block->current);

        block->current -= size;
    }
    else
    {
        LOG("Nothing to free in memory\n");
    }

    return block->current;
}

#define MAX_SPRITES 1024

struct sprite_vertex_data
{
    struct v3 position;
    struct v3 normal;
    struct v2 uv;
};

struct sprite_data
{
    struct m4 model;
    struct v4 color;
    u32 texture;
};

struct sprite_renderer
{
    struct sprite_data sprites[MAX_SPRITES];
    u32 vao;
    u32 vbo_vertices;
    u32 vbo_sprites;
    u32 ibo;
    u32 num_indices;
    u32 num_sprites;
    u32 shader;
    u32 texture;
    b32 initialized;
};

#define MAX_CUBES 1024
#define MAX_CUBE_COLORS 2048

struct cube_vertex_data
{
    struct v3 position;
    struct v3 normal;
    struct v2 uv;
};

struct cube_face
{
    u8 texture;
    u8 rotation;
    u8 color;
};

struct cube_data
{
    struct m4 model;
    struct cube_face faces[6];
};

struct cube_renderer
{
    struct cube_data cubes[MAX_CUBES];
    struct v4 colors[MAX_CUBE_COLORS];
    u32 vao;
    u32 vbo_vertices;
    u32 vbo_cubes;
    u32 ibo;
    u32 ubo;
    u32 num_indices;
    u32 num_cubes;
    u32 num_colors;
    u32 shader;
    u32 texture;
    b32 update_color_data;
    b32 initialized;
};

struct particle_data
{
    struct v2 velocity;
    struct v4 color;
    struct v2 tl;
    struct v2 tr;
    struct v2 br;
    struct v2 bl;
    f32 time;
};

#define MAX_PARTICLES 8192

struct particle_storage
{
    struct particle_data data[MAX_PARTICLES];
    u32 num_particles;
};

void particle_emitter_circle(struct particle_storage* storage, u32 count,
    struct v2 position, f32 velocity, f32 size, struct v4 color, f32 time)
{
    if (count > MAX_PARTICLES - storage->num_particles)
    {
        storage->num_particles = 0;
    }

    f32 angle = 0.0f;
    f32 angle_inc = F64_PI * 2.0f / count;
    f32 size_half = size * 0.5f;

    for (u32 i = 0; i < count; i++, angle += angle_inc)
    {
        struct particle_data data;
        data.tl = (struct v2){ position.x - size_half, position.y + size_half };
        data.tr = (struct v2){ position.x + size_half, position.y + size_half };
        data.br = (struct v2){ position.x + size_half, position.y - size_half };
        data.bl = (struct v2){ position.x - size_half, position.y - size_half };
        data.velocity = v2_mul_f32(v2_direction_from_angle(angle), velocity);
        data.color = color;
        data.time = time;
        storage->data[storage->num_particles++] = data;
    }

    LOG("Particles: %d of %d\n", storage->num_particles, MAX_PARTICLES);
}

struct particle_line
{
    struct v2 start;
    struct v2 end;
    struct v4 color_start;
    struct v4 color_end;
    struct v4 color_current;
    f32 time_start;
    f32 time_current;
    b32 alive;
};

struct particle_sphere
{
    struct v2 position;
    struct v2 velocity;
    struct v4 color;
    f32 radius_start;
    f32 radius_end;
    f32 radius_current;
    f32 time_start;
    f32 time_current;
    b32 alive;
};

struct rigid_body
{
    struct v2 position;
    struct v2 velocity;
    f32 angle;
};

struct weapon
{
    struct v2 position;
    struct v2 velocity;
    struct v2 direction;
    u32 type;
    u32 level;
    u32 ammo;
    u32 ammo_max;
    f32 last_shot;
    f32 fire_rate;
    f32 reload_time;
    f32 spread;
    f32 projectile_size;
    f32 projectile_speed;
    f32 projectile_damage;
    b32 fired;
    b32 reloading;
};

struct player
{
    struct rigid_body body;
    struct v2 eye_position;
    struct weapon weapon;
    struct cube_data cube;
    f32 health;
    b32 alive;
    u32 item_picked;
};

struct bullet
{
    struct rigid_body body;
    struct v2 start;
    f32 damage;
    b32 alive;
    b32 player_owned;
};

struct item
{
    struct rigid_body body;
    struct cube_data cube;
    u32 type;
    f32 alive;
    f32 flash_timer;
    b32 flash_hide;
};

enum
{
    ENEMY_STATE_SHOOT,
    ENEMY_STATE_WANDER_AROUND,
    ENEMY_STATE_RUSH_TO_TARGET,
    ENEMY_STATE_SLEEP,
    ENEMY_STATE_REACT_TO_PLAYER_SEEN,
    ENEMY_STATE_REACT_TO_GUN_SHOT,
    ENEMY_STATE_REACT_TO_BEING_SHOT_AT,
    ENEMY_STATE_LOOK_AROUND,
    ENEMY_STATE_LOOK_FOR_PLAYER
};

char enemy_state_str[][256] =
{
    "SHOOT",
    "WANDER AROUND",
    "RUSH TO TARGET",
    "SLEEP",
    "REACT_TO_PLAYER_SEEN",
    "REACT_TO_GUN_SHOT",
    "REACT_TO_BEING_SHOT_AT",
    "LOOK_AROUND",
    "LOOK_FOR_PLAYER"
};

#define MAX_PATH 256

struct enemy
{
    struct rigid_body body;
    struct v2 path[MAX_PATH];
    struct v2 direction_aim;
    struct v2 direction_look;
    struct v2 direction_move;
    struct v2 eye_position;
    struct v2 player_last_seen_position;
    struct v2 player_last_seen_direction;
    struct v2 gun_shot_position;
    struct v2 hit_direction;
    struct v2 target;
    struct weapon weapon;
    struct cube_data cube;
    u32 state;
    u32 path_index;
    u32 path_length;
    u32 turns_left; // For looking around
    f32 health;
    f32 trigger_release;
    f32 vision_cone_size;
    f32 acceleration;
    f32 state_timer;
    f32 turn_amount;
    b32 player_in_view;
    b32 gun_shot_heard;
    b32 got_hit;
    b32 alive;
    b32 shooting;
    b32 state_timer_finished;
};

#define MAX_LEVEL_SIZE 256

struct level
{
    u8 tile_sprites[MAX_LEVEL_SIZE*MAX_LEVEL_SIZE];
    u8 tile_types[MAX_LEVEL_SIZE*MAX_LEVEL_SIZE];
    u32 width;
    u32 height;
    struct v2 start_pos;
    struct cube_data elevator_light;
};

struct camera
{
    struct m4 projection;
    struct m4 view;
    struct m4 projection_inverse;
    struct m4 view_inverse;
    struct v3 position;
    struct v3 target;
    f32 screen_width;
    f32 screen_height;
};

struct mouse
{
    struct v2 world;
    struct v2 screen;
};

struct vertex
{
    struct v3 position;
    struct v2 uv;
    struct v3 normal;
    struct v4 color;
};

struct mesh
{
    u32 vao;
    u32 vbo;
    u32 ibo;
    u32 num_indices;
};

#define MAX_BULLETS 64
#define MAX_ENEMIES 64
#define MAX_ITEMS 256
#define MAX_WALL_CORNERS 512
#define MAX_WALL_FACES 512
#define MAX_COLLISION_SEGMENTS 1024
#define MAX_GUN_SHOTS 64

enum
{
    WEAPON_NONE = 0,
    WEAPON_SHOTGUN = 1,
    WEAPON_MACHINEGUN = 2,
    WEAPON_PISTOL = 3,
    WEAPON_COUNT = 4
};

struct line_segment
{
    struct v2 start;
    struct v2 end;
    u32 type;
};

struct gun_shot
{
    struct v2 position;
    f32 volume; // Todo: change name
};

struct game_state
{
    struct player player;
    struct bullet bullets[MAX_BULLETS];
    struct enemy enemies[MAX_ENEMIES];
    struct item items[MAX_ITEMS];
    struct particle_line particle_lines[MAX_PARTICLES];
    struct particle_sphere particle_spheres[MAX_PARTICLES];
    struct camera camera;
    struct mouse mouse;
    struct mesh sphere;
    struct mesh wall;
    struct mesh floor;
    struct mesh triangle;
    struct memory_block stack;
    struct v2 wall_corners[MAX_WALL_CORNERS];
    struct line_segment wall_faces[MAX_WALL_FACES];
    struct line_segment cols_static[MAX_COLLISION_SEGMENTS];
    struct line_segment cols_dynamic[MAX_COLLISION_SEGMENTS];
    struct cube_renderer cube_renderer;
    struct sprite_renderer sprite_renderer;
    struct gun_shot gun_shots[MAX_GUN_SHOTS];
    struct level level;
    struct level level_mask;
    struct particle_storage particles;
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
    u32 texture_tileset;
    u32 texture_sphere;
    u32 texture_cube;
    u32 texture_sprite;
    u32 free_bullet;
    u32 free_item;
    u32 free_particle_line;
    u32 free_particle_sphere;
    u32 num_enemies;
    u32 num_wall_corners;
    u32 num_wall_faces;
    u32 num_cols_static;
    u32 num_cols_dynamic;
    u32 num_gun_shots;
    u32 random_seed;
    u32 ticks_per_second;
    u32 level_current;
};

// Todo: global for now
struct api api;

u32 key_times_pressed(struct key_state* state)
{
    u32 result = 0;

    result = state->transitions / 2;

    if (state->key_down && state->transitions % 2)
    {
        result++;
    }

    return result;
}

f32 time_elapsed_seconds(u64 ticks_start, u64 ticks_end)
{
    f32 result = (ticks_end - ticks_start) / 1000000000.0f;

    return result;
}

u32 random_number_generate(struct game_state* state)
{  
    u32 result = state->random_seed =
        (u64)state->random_seed * 48271 % 0x7fffffff;

    return result;
}

u32 u32_random_number_get(struct game_state* state, u32 min, u32 max)
{
    u32 result = random_number_generate(state) % (max - min + 1) + min;

    return result;
}

f32 f32_random_number_get(struct game_state* state, f32 min, f32 max)
{
    f32 result = 0.0f;
    f32 rand = u32_random_number_get(state, 0, S32_MAX) / (f32)S32_MAX;
    result = min + rand * (max - min);

    return result;
}

f32 PLAYER_ACCELERATION = 40.0f;
f32 PLAYER_RADIUS       = 0.25f;
f32 PLAYER_HEALTH_MAX   = 100.0f;

f32 ENEMY_ACCELERATION           = 35.0f;
f32 ENEMY_LINE_OF_SIGHT_HALF     = F64_PI * 0.25f; // 45 degrees
f32 ENEMY_LINE_OF_SIGHT_DISTANCE = 8.0f;
f32 ENEMY_REACTION_TIME_MIN      = 0.25f;
f32 ENEMY_REACTION_TIME_MAX      = 0.75f;
f32 ENEMY_GUN_FIRE_HEAR_DISTANCE = 10.0f;
f32 ENEMY_HEALTH_MAX             = 100.0f;
f32 ENEMY_LOOK_AROUND_DELAY_MIN  = 0.5f;
f32 ENEMY_LOOK_AROUND_DELAY_MAX  = 2.0f;

f32 ITEM_RADIUS = 0.1;
f32 ITEM_ALIVE_TIME = 10.0f;
f32 ITEM_FLASH_TIME = 2.0f;
f32 ITEM_FLASH_SPEED = 0.125f;

f32 CAMERA_ACCELERATION = 15.0f;

enum
{
    ITEM_NONE = 0,
    ITEM_HEALTH = 1,
    ITEM_SHOTGUN = 2,
    ITEM_MACHINEGUN = 3,
    ITEM_PISTOL = 4,
    ITEM_WEAPON_LEVEL_UP = 5,
    ITEM_COUNT = 6
};

u32 ITEAM_HEALTH_AMOUNT = 25.0f;

f32 PROJECTILE_RADIUS = 0.035f;
f32 PROJECTILE_SPEED  = 100.0f;

f32 FRICTION  = 10.0f;
f32 WALL_SIZE = 1.0f;

u32 TILE_NOTHING = 0;
u32 TILE_WALL    = 1;
u32 TILE_FLOOR   = 2;
u32 TILE_DOOR    = 3;
u32 TILE_START   = 4;

u32 COLLISION_STATIC  = 1;
u32 COLLISION_PLAYER  = 2;
u32 COLLISION_ENEMY   = 4;
u32 COLLISION_BULLET  = 8;
u32 COLLISION_DYNAMIC = 14;
u32 COLLISION_ALL     = 255;

u32 WEAPON_LEVEL_MAX = 10;

// Todo: clean this, not very nice
struct v4 colors[] =
{
    { 0.0,  0.0,  0.0,  1.0 },
    { 0.0,  0.0,  0.5,  1.0 },
    { 0.0,  0.0,  1.0,  1.0 },
    { 0.0,  0.5,  0.0,  1.0 },
    { 0.0,  0.5,  0.5,  1.0 },
    { 0.0,  1.0,  0.0,  1.0 },
    { 0.0,  1.0,  1.0,  1.0 },
    { 0.5,  0.0,  0.0,  1.0 },
    { 0.5,  0.0,  0.5,  1.0 },
    { 0.5,  0.5,  0.0,  1.0 },
    { 0.5,  0.5,  0.5,  1.0 },
    { 0.75, 0.75, 0.75, 1.0 },
    { 1.0,  0.0,  0.0,  1.0 },
    { 1.0,  0.0,  1.0,  1.0 },
    { 1.0,  1.0,  0.0,  1.0 },
    { 1.0,  1.0,  1.0,  1.0 }
};

enum
{
    BLACK,
    NAVY,
    BLUE,
    GREEN,
    TEAL,
    LIME,
    AQUA,
    MAROON,
    PURPLE,
    OLIVE,
    GREY,
    SILVER,
    RED,
    FUCHSIA,
    YELLOW,
    WHITE
};

b32 tile_inside_level_bounds(struct level* level, struct v2 position)
{
    b32 result = false;

    s32 x = f32_round(position.x);
    s32 y = f32_round(position.y);

    result = x >= 0 && x < level->width && y >= 0 && y < level->height;

    return result;
}

b32 tile_is_of_type(struct level* level, struct v2 position, u32 type)
{
    b32 result = false;

    if (tile_inside_level_bounds(level, position))
    {
        s32 x = f32_round(position.x);
        s32 y = f32_round(position.y);

        result = level->tile_types[y * level->width + x] == type;
    }

    return result;
}

u8 tile_type_get(struct level* level, struct v2 position)
{
    u8 result = 0;

    if (tile_inside_level_bounds(level, position))
    {
        s32 x = f32_round(position.x);
        s32 y = f32_round(position.y);

        result = level->tile_types[y * level->width + x];
    }

    return result;
}

b32 tile_is_free(struct level* level, struct v2 position)
{
    return tile_type_get(level, position) > TILE_WALL;
}

struct v2 tile_random_get(struct game_state* state, struct level* level,
    u32 type)
{
    struct v2 result = { 0 };

    u32 max_iterations = 1000;

    while (max_iterations--)
    {
        struct v2 position =
        {
            u32_random_number_get(state, 0, level->width),
            u32_random_number_get(state, 0, level->height)
        };

        if (tile_is_of_type(level, position, type))
        {
            result = position;

            break;
        }
    };

    return result;
}

struct node
{
    // Todo: use integers instead?
    struct v2 position;
    struct node* parent;
    f32 g;
    f32 h;
    f32 f;
    b32 in_use;
};

// Todo: this really depends on the level size
#define MAX_NODES 1024

struct node* node_insert(struct node* node, struct node nodes[], u32 num_nodes)
{
    struct node* result = NULL;

    for (u32 i = 0; i < num_nodes; i++)
    {
        if (!nodes[i].in_use)
        {
            nodes[i] = *node;
            result = &nodes[i];
            break;
        }
    }

    return result;
}

struct node* lowest_rank_find(struct node nodes[], u32 num_nodes)
{
    struct node* result = NULL;

    for (u32 i = 0; i < num_nodes; i++)
    {
        if (nodes[i].in_use && (!result || nodes[i].f < result->f))
        {
            result = &nodes[i];
        }
    }

    return result;
}

b32 nodes_equal(struct node* a, struct node* b)
{
    return v2_equals(a->position, b->position);
}

struct node* node_find(struct v2* node, struct node nodes[], u32 num_nodes)
{
    struct node* result = NULL;

    for (u32 i = 0; i < num_nodes; i++)
    {
        if (nodes[i].in_use && v2_equals(*node, nodes[i].position))
        {
            result = &nodes[i];
            break;
        }
    }

    return result;
}

b32 neighbor_check(struct level* level, f32 x, f32 y, struct v2 neighbors[],
    u32* index)
{
    b32 result = false;

    struct v2 neighbor = { x, y };

    if (tile_is_free(level, neighbor))
    {
        neighbors[(*index)++] = neighbor;
        result = true;
    }

    return result;
}

u32 neighbors_get(struct level* level, struct node* node, struct v2 neighbors[])
{
    u32 result = 0;

    f32 x = node->position.x;
    f32 y = node->position.y;

    b32 left  = neighbor_check(level, x - 1.0f, y, neighbors, &result);
    b32 right = neighbor_check(level, x + 1.0f, y, neighbors, &result);
    b32 up    = neighbor_check(level, x, y + 1.0f, neighbors, &result);
    b32 down  = neighbor_check(level, x, y - 1.0f, neighbors, &result);

    if (left && up)
    {
        neighbor_check(level, x - 1.0f, y + 1.0f, neighbors, &result);
    }

    if (left && down)
    {
        neighbor_check(level, x - 1.0f, y - 1.0f, neighbors, &result);
    }

    if (right && up)
    {
        neighbor_check(level, x + 1.0f, y + 1.0f, neighbors, &result);
    }

    if (right && down)
    {
        neighbor_check(level, x + 1.0f, y - 1.0f, neighbors, &result);
    }

    return result;
}

f32 cost_calculate(struct v2 a, struct v2 b)
{
    f32 result = 0.0f;

    if (a.x != b.x && a.y != b.y)
    {
        result = f32_sqrt(2.0f);
    }
    else
    {
        result = 1.0f;
    }

    return result;
}

f32 heuristic_calculate(struct v2 a, struct v2 b)
{
    f32 result = 0.0f;

    f32 dx = f32_abs(a.x - b.x);
    f32 dy = f32_abs(a.y - b.y);
    f32 cost = 1.0f;
    f32 cost_diagonal = f32_sqrt(2.0f);

    result = cost * (dx + dy) + (cost_diagonal - 2.0f * cost) * MIN(dx, dy);

    return result;
}

void node_add_to_path(struct node* node, struct v2 path[], u32* index)
{
    if (!node->parent)
    {
        return;
    }

    // Todo: this overflows stack sometimes
    node_add_to_path(node->parent, path, index);

    (*index)++;

    if (path)
    {
        path[*index] = node->position;
    }
}

u32 path_find(struct level* level, struct v2 start, struct v2 goal,
    struct v2 path[], u32 path_size)
{
    u32 result = 0;

    if (!tile_is_free(level, start) || !tile_is_free(level, goal))
    {
        return result;
    }

    struct node open[MAX_NODES] = { 0 };
    struct node closed[MAX_NODES] = { 0 };

    start.x = f32_round(start.x);
    start.y = f32_round(start.y);
    goal.x  = f32_round(goal.x);
    goal.y  = f32_round(goal.y);

    struct node node_start = { start, NULL, 0.0f, 0.0f, 0.0f, true };
    struct node node_goal =  { goal, NULL, 0.0f, 0.0f, 0.0f, true };

    node_insert(&node_start, open, MAX_NODES);

    struct node* lowest = NULL;

    while ((lowest = lowest_rank_find(open, MAX_NODES)) &&
        !nodes_equal(lowest, &node_goal))
    {
        struct node* current = node_insert(lowest, closed, MAX_NODES);

        if (!current)
        {
            // Todo: assert here?
            LOG("CLOSED SET FULL!\n");
            break;
        }

        lowest->in_use = false;

        struct v2 neighbors[8];

        u32 num_neighbors = neighbors_get(level, current, neighbors);

        for (u32 i = 0; i < num_neighbors; i++)
        {
            struct v2* neighbor = &neighbors[i];
            struct node* neighbor_node = NULL;
            f32 cost = current->g + cost_calculate(current->position,
                *neighbor);
            f32 heuristic = heuristic_calculate(*neighbor, goal);

            if ((neighbor_node = node_find(neighbor, open, MAX_NODES)) &&
                cost < neighbor_node->g)
            {
                neighbor_node->in_use = false;
            }

            if ((neighbor_node = node_find(neighbor, closed, MAX_NODES)) &&
                cost < neighbor_node->g)
            {
                neighbor_node->in_use = false;
            }

            if (!node_find(neighbor, open, MAX_NODES) &&
                !node_find(neighbor, closed, MAX_NODES))
            {
                struct node new =
                {
                    *neighbor, current, cost, heuristic, cost + heuristic, true
                };

                // Todo: assert here?
                if (!node_insert(&new, open, MAX_NODES))
                {
                    LOG("OPEN SET FULL!\n");
                    break;
                }
            }
        }
    }

    if (lowest)
    {
        node_add_to_path(lowest, path, &result);
        result++;
    }

    return result;
}

void memory_set(void* data, u64 size, u8 value)
{
    for (u32 i = 0; i < size; i++)
    {
        *((u8*)data + i) = value;
    }
}

void memory_copy(void* src, void* dest, u64 size)
{
    for (u32 i = 0; i < size; i++)
    {
        *((u8*)dest + i) = *((u8*)src + i);
    }
}

struct ray_cast_collision
{
    struct v2 position;
    struct v2 wall_start;
    struct v2 wall_end;
    // Todo: add wall normal? Could be nice
    f32 ray_length;
};

void get_body_rectangle(struct rigid_body body, f32 width_half,
    f32 height_half, struct line_segment* segments)
{
    struct v2 corners[4] =
    {
        { -width_half,  height_half },
        {  width_half,  height_half },
        {  width_half, -height_half },
        { -width_half, -height_half }
    };

    for (u32 i = 0; i < 4; i++)
    {
        corners[i] = v2_rotate(corners[i], body.angle);
        corners[i].x += body.position.x;
        corners[i].y += body.position.y;
    }

    for (u32 i = 0; i < 4; i++)
    {
        segments[i].start = corners[i];
        segments[i].end   = corners[(i+1) % 4];
    }
}

b32 intersect_ray_to_line_segment(struct v2 start, struct v2 direction,
    struct line_segment line_segment, struct v2* collision)
{
    b32 result = false;
    struct v2 p = start;
    struct v2 r = direction;
    struct v2 q = line_segment.start;
    struct v2 s = v2_direction(line_segment.start, line_segment.end);
    struct v2 qp = { q.x - p.x, q.y - p.y };
    f32 r_x_s = v2_cross(r, s);

    if (r_x_s == 0.0f)
    {
        // Todo: implement if necessary
    }
    else
    {
        f32 qp_x_s = v2_cross(qp, s);
        f32 qp_x_r = v2_cross(qp, r);

        f32 t = qp_x_s / r_x_s;
        f32 u = qp_x_r / r_x_s;

        if (t > 0.0f && u > 0.0f && u < 1.0f)
        {
            collision->x = p.x + t * r.x;
            collision->y = p.y + t * r.y;

            result = true;
        }
    }

    return result;
}

f32 ray_cast_to_direction(struct v2 start, struct v2 direction,
    struct line_segment cols[], u32 num_cols, struct v2* collision,
    f32 max_length, u32 flags)
{
    f32 result = max_length;

    for (u32 i = 0; i < num_cols; i++)
    {
        struct v2 position = { 0 };

        if (cols[i].type & flags &&
            intersect_ray_to_line_segment(start, direction, cols[i], &position)
            && v2_distance(start, position) < result)
        {
            if (collision)
            {
                *collision = position;
            }

            result = v2_distance(start, position);
        }
    }

    return result;
}

f32 ray_cast_direction(struct game_state* state, struct v2 position,
    struct v2 direction, struct v2* collision, u32 flags)
{
    f32 result = F32_MAX;

    if (flags & COLLISION_STATIC)
    {
        result = ray_cast_to_direction(position, direction, state->cols_static,
            state->num_cols_static, collision, result, flags);
    }

    if (flags & COLLISION_DYNAMIC)
    {
        result = ray_cast_to_direction(position, direction, state->cols_dynamic,
            state->num_cols_dynamic, collision, result, flags);
    }

    return result;
}

f32 ray_cast_position(struct game_state* state, struct v2 start, struct v2 end,
    struct v2* collision, u32 flags)
{
    f32 result = ray_cast_direction(state, start, v2_direction(start, end),
        collision, flags);
    f32 distance = v2_distance(start, end) - 0.1f;

    if (result < distance)
    {
        result = 0.0f;
    }

    return result;
}

f32 ray_cast_body(struct game_state* state, struct v2 start,
    struct rigid_body body, struct v2* collision, u32 flags)
{
    f32 result = 0.0f;

    struct line_segment segments[4] = { 0 };

    get_body_rectangle(body, PLAYER_RADIUS, PLAYER_RADIUS, segments);

    for (u32 i = 0; i < 4; i++)
    {
        segments[i].type = flags;
    }

    struct v2 direction = v2_direction(start, body.position);

    f32 target = ray_cast_to_direction(start, direction, segments, 4, NULL,
        F32_MAX, flags) - 0.1f;

    result = ray_cast_direction(state, start, direction, collision, flags);

    if (result < target)
    {
        result = 0.0f;
    }

    return result;
}

void path_trim(struct game_state* state, struct v2 obj_start, struct v2 path[],
    u32* path_size)
{
    if (!(*path_size))
    {
        return;
    }

    struct v2 result[MAX_PATH] = { 0 };
    struct v2 start = obj_start;

    u32 result_index = 0;
    u32 path_index = 0;

    while (path_index < (*path_size - 1))
    {
        struct v2 end = path[path_index + 1];

        // Ray cast to each tile corner
        f32 wall_half = WALL_SIZE * 0.5f;

        struct v2 tl = { end.x - wall_half, end.y + wall_half };
        struct v2 bl = { end.x - wall_half, end.y - wall_half };
        struct v2 br = { end.x + wall_half, end.y - wall_half };
        struct v2 tr = { end.x + wall_half, end.y + wall_half };

        if (!ray_cast_position(state, start, tl, NULL, COLLISION_STATIC) ||
            !ray_cast_position(state, start, bl, NULL, COLLISION_STATIC) ||
            !ray_cast_position(state, start, br, NULL, COLLISION_STATIC) ||
            !ray_cast_position(state, start, tr, NULL, COLLISION_STATIC))
        {
            start = result[result_index++] = path[path_index];
        }
        else
        {
            path_index++;
        }
    }

    // Always insert the end node
    result[result_index++] = path[*path_size - 1];

    *path_size = result_index;
    memory_copy(&result, path, *path_size * sizeof(struct v2));
}

struct v2 calculate_world_pos(f32 pos_x, f32 pos_y, struct camera* camera)
{
    // Todo: doesn't work with orthographic projection
    struct v2 result = { 0.0f };

    struct v4 ndc =
    {
        pos_x / (camera->screen_width * 0.5f) - 1.0f,
        (pos_y / (camera->screen_height * 0.5f) - 1.0f) * -1.0f
    };

    struct v4 clip = { ndc.x, ndc.y, -1.0f, 1.0f };
    struct v4 view = m4_mul_v4(camera->projection_inverse, clip);
    view.z = -1.0f;
    view.w = 0.0f;

    struct v4 world = m4_mul_v4(camera->view_inverse, view);

    struct v3 temp = { world.x, world.y, world.z };
    temp = v3_normalize(temp);

    f32 c = f32_abs(camera->position.z / temp.z);

    result.x = temp.x * c + camera->position.x;
    result.y = temp.y * c + camera->position.y;

    return result;
}

struct v2 calculate_screen_pos(f32 pos_x, f32 pos_y, f32 pos_z,
    struct camera* camera)
{
    struct v2 result = { 0.0f };
    struct v4 world = { pos_x, pos_y, pos_z, 1.0f };

    struct v4 view = m4_mul_v4(camera->view, world);
    struct v4 clip = m4_mul_v4(camera->projection, view);
    struct v2 ndc = { clip.x / clip.w, clip.y / clip.w };

    result.x = (ndc.x + 1.0f) * camera->screen_width * 0.5f;
    result.y = (ndc.y + 1.0f) * camera->screen_height * 0.5f;

    return result;
}

bool gl_check_error(char* t)
{
    bool result = true;

    GLenum error = api.gl.glGetError();

    switch (error)
    {
        case GL_NO_ERROR:
            // LOG("glGetError(): NO ERRORS (%s)\n", t);
            result = false;
            break;
        case GL_INVALID_ENUM:
            LOG("glGetError(): INVALID ENUM (%s)\n", t);
            break;
        case GL_INVALID_VALUE:
            LOG("glGetError(): INVALID VALUE (%s)\n", t);
            break;
        case GL_INVALID_OPERATION:
            LOG("glGetError(): INVALID OPERATION (%s)\n", t);
            break;
        case GL_STACK_OVERFLOW:
            LOG("glGetError(): STACK OVERFLOW (%s)\n", t);
            break;
        case GL_STACK_UNDERFLOW:
            LOG("glGetError(): STACK UNDERFLOW (%s)\n", t);
            break;
        case GL_OUT_OF_MEMORY:
            LOG("glGetError(): OUT OF MEMORY (%s)\n", t);
            break;
        default:
            LOG("glGetError(): UNKNOWN ERROR (%s)\n", t);
            break;
    };

    return result;
}

void generate_vertex_array(struct mesh* mesh, struct vertex* vertices,
    u32 num_vertices, u32* indices)
{
    LOG("Vertices: %d Indices: %d\n", num_vertices, mesh->num_indices);
    api.gl.glGenVertexArrays(1, &mesh->vao);
    api.gl.glBindVertexArray(mesh->vao);

    api.gl.glGenBuffers(1, &mesh->vbo);
    api.gl.glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    api.gl.glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(struct vertex),
        vertices, GL_DYNAMIC_DRAW);

    api.gl.glGenBuffers(1, &mesh->ibo);
    api.gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
    api.gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        mesh->num_indices * sizeof(u32), indices, GL_DYNAMIC_DRAW);

    api.gl.glEnableVertexAttribArray(0);
    api.gl.glEnableVertexAttribArray(1);
    api.gl.glEnableVertexAttribArray(2);
    api.gl.glEnableVertexAttribArray(3);

    // Todo: implement offsetof
    api.gl.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)0);
    api.gl.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)12);
    api.gl.glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)20);
    api.gl.glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)32);
}

void sprite_renderer_init(struct sprite_renderer* renderer, u32 shader,
    u32 texture)
{
    renderer->shader = shader;
    renderer->texture = texture;

    struct sprite_vertex_data vertices[] =
    {
        // Top right
        {
            { 1.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f },
            { 0.0f, 1.0f }
        },
        // Top left
        {
            { -1.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f },
            { 0.0f, 0.0f }
        },
        // Bottom left
        {
            { -1.0f, -1.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f },
            { 1.0f, 0.0f }
        },
        // Bottom right
        {
            { 1.0f, -1.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f },
            { 1.0f, 1.0f }
        }
    };

    u32 indices[] =
    {
        0, 1, 2, 0, 2, 3
    };

    renderer->num_indices = 6;

    api.gl.glGenVertexArrays(1, &renderer->vao);
    api.gl.glBindVertexArray(renderer->vao);

    api.gl.glGenBuffers(1, &renderer->vbo_vertices);
    api.gl.glGenBuffers(1, &renderer->vbo_sprites);
    api.gl.glGenBuffers(1, &renderer->ibo);

    api.gl.glEnableVertexAttribArray(0);
    api.gl.glEnableVertexAttribArray(1);
    api.gl.glEnableVertexAttribArray(2);
    api.gl.glEnableVertexAttribArray(3);
    api.gl.glEnableVertexAttribArray(4);
    api.gl.glEnableVertexAttribArray(5);
    api.gl.glEnableVertexAttribArray(6);
    api.gl.glEnableVertexAttribArray(7);
    api.gl.glEnableVertexAttribArray(8);

    api.gl.glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_vertices);
    api.gl.glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
        GL_STATIC_DRAW);
    api.gl.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct sprite_vertex_data), (void*)0);
    api.gl.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct sprite_vertex_data), (void*)12);
    api.gl.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
        sizeof(struct sprite_vertex_data), (void*)24);

    api.gl.glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_sprites);
    api.gl.glBufferData(GL_ARRAY_BUFFER, sizeof(renderer->sprites),
        renderer->sprites, GL_DYNAMIC_DRAW);
    api.gl.glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct sprite_data), (void*)(sizeof(struct v4) * 0));
    api.gl.glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct sprite_data), (void*)(sizeof(struct v4) * 1));
    api.gl.glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct sprite_data), (void*)(sizeof(struct v4) * 2));
    api.gl.glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct sprite_data), (void*)(sizeof(struct v4) * 3));
    api.gl.glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct sprite_data), (void*)(sizeof(struct v4) * 4));
    api.gl.glVertexAttribIPointer(8, 1, GL_UNSIGNED_BYTE,
        sizeof(struct sprite_data), (void*)(sizeof(struct v4) * 5));

    api.gl.glVertexAttribDivisor(3, 1);
    api.gl.glVertexAttribDivisor(4, 1);
    api.gl.glVertexAttribDivisor(5, 1);
    api.gl.glVertexAttribDivisor(6, 1);
    api.gl.glVertexAttribDivisor(7, 1);
    api.gl.glVertexAttribDivisor(8, 1);

    api.gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
    api.gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        renderer->num_indices * sizeof(u32), indices, GL_STATIC_DRAW);

    renderer->initialized = !gl_check_error("sprite_renderer_init");
}

void sprite_renderer_add(struct sprite_renderer* renderer,
    struct sprite_data* data)
{
    if (renderer->initialized)
    {
        if (renderer->num_sprites < MAX_SPRITES)
        {
            renderer->sprites[renderer->num_sprites++] = *data;
        }
    }
}

void sprite_renderer_flush(struct sprite_renderer* renderer, struct m4* view,
    struct m4* projection)
{
    if (renderer->initialized)
    {
        api.gl.glBindVertexArray(renderer->vao);
        api.gl.glUseProgram(renderer->shader);

        u32 uniform_texture = api.gl.glGetUniformLocation(renderer->shader,
            "uniform_texture");
        u32 uniform_vp = api.gl.glGetUniformLocation(renderer->shader,
            "uniform_vp");

        struct m4 vp = m4_mul_m4(*view, *projection);

        api.gl.glUniform1i(uniform_texture, 0);
        api.gl.glUniformMatrix4fv(uniform_vp, 1, GL_FALSE, (GLfloat*)&vp);

        api.gl.glActiveTexture(GL_TEXTURE0);
        api.gl.glBindTexture(GL_TEXTURE_2D_ARRAY, renderer->texture);

        api.gl.glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_sprites);
        api.gl.glBufferSubData(GL_ARRAY_BUFFER, 0,
            renderer->num_sprites * sizeof(struct sprite_data),
            renderer->sprites);

        api.gl.glDrawElementsInstanced(GL_TRIANGLES, renderer->num_indices,
            GL_UNSIGNED_INT, NULL, renderer->num_sprites);

        api.gl.glUseProgram(0);
        api.gl.glBindVertexArray(0);

        renderer->num_sprites = 0;
    }
}

void cube_renderer_init(struct cube_renderer* renderer, u32 shader, u32 texture)
{
    renderer->shader = shader;
    renderer->texture = texture;

    struct cube_vertex_data vertices[] =
    {
        // Top right
        {
            { 1.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f },
            { 0.0f, 1.0f }
        },
        // Top left
        {
            { -1.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f },
            { 0.0f, 0.0f }
        },
        // Bottom left
        {
            { -1.0f, -1.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f },
            { 1.0f, 0.0f }
        },
        // Bottom right
        {
            { 1.0f, -1.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f },
            { 1.0f, 1.0f }
        },
        // Top right
        {
            { -1.0f, 1.0f, -1.0f },
            { 0.0f, 0.0f, -1.0f },
            { 0.0f, 1.0f }
        },
        // Top left
        {
            { 1.0f, 1.0f, -1.0f },
            { 0.0f, 0.0f, -1.0f },
            { 0.0f, 0.0f }
        },
        // Bottom left
        {
            { 1.0f, -1.0f, -1.0f },
            { 0.0f, 0.0f, -1.0f },
            { 1.0f, 0.0f }
        },
        // Bottom right
        {
            { -1.0f, -1.0f, -1.0f },
            { 0.0f, 0.0f, -1.0f },
            { 1.0f, 1.0f }
        },
        // Top right
        {
            { -1.0f, -1.0f, 1.0f },
            { -1.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f }
        },
        // Top left
        {
            { -1.0f, 1.0f, 1.0f },
            { -1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f }
        },
        // Bottom left
        {
            { -1.0f, 1.0f, -1.0f },
            { -1.0f, 0.0f, 0.0f },
            { 1.0f, 0.0f }
        },
        // Bottom right
        {
            { -1.0f, -1.0f, -1.0f },
            { -1.0f, 0.0f, 0.0f },
            { 1.0f, 1.0f }
        },
        // Top right
        {
            { 1.0f, 1.0f, 1.0f },
            { 1.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f }
        },
        // Top left
        {
            { 1.0f, -1.0f, 1.0f },
            { 1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f }
        },
        // Bottom left
        {
            { 1.0f, -1.0f, -1.0f },
            { 1.0f, 0.0f, 0.0f },
            { 1.0f, 0.0f }
        },
        // Bottom right
        {
            { 1.0f, 1.0f, -1.0f },
            { 1.0f, 0.0f, 0.0f },
            { 1.0f, 1.0f }
        },
        // Top right
        {
            { -1.0f, 1.0f, 1.0f },
            { 0.0f, 1.0f, 0.0f },
            { 0.0f, 1.0f }
        },
        // Top left
        {
            { 1.0f, 1.0f, 1.0f },
            { 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f }
        },
        // Bottom left
        {
            { 1.0f, 1.0f, -1.0f },
            { 0.0f, 1.0f, 0.0f },
            { 1.0f, 0.0f }
        },
        // Bottom right
        {
            { -1.0f, 1.0f, -1.0f },
            { 0.0f, 1.0f, 0.0f },
            { 1.0f, 1.0f }
        },
        // Top right
        {
            { 1.0f, -1.0f, 1.0f },
            { 0.0f, -1.0f, 0.0f },
            { 0.0f, 1.0f }
        },
        // Top left
        {
            { -1.0f, -1.0f, 1.0f },
            { 0.0f, -1.0f, 0.0f },
            { 0.0f, 0.0f }
        },
        // Bottom left
        {
            { -1.0f, -1.0f, -1.0f },
            { 0.0f, -1.0f, 0.0f },
            { 1.0f, 0.0f }
        },
        // Bottom right
        {
            { 1.0f, -1.0f, -1.0f },
            { 0.0f, -1.0f, 0.0f },
            { 1.0f, 1.0f }
        }
    };

    u32 indices[] =
    {
         0,  1,  2,  0,  2,  3, // Top
         4,  5,  6,  4,  6,  7, // Bottom
         8,  9, 10,  8, 10, 11, // Left
        12, 13, 14, 12, 14, 15, // Right
        16, 17, 18, 16, 18, 19, // Up
        20, 21, 22, 20, 22, 23, // Down
    };

    renderer->num_indices = 36;

    api.gl.glGenVertexArrays(1, &renderer->vao);
    api.gl.glBindVertexArray(renderer->vao);

    api.gl.glGenBuffers(1, &renderer->vbo_vertices);
    api.gl.glGenBuffers(1, &renderer->vbo_cubes);
    api.gl.glGenBuffers(1, &renderer->ibo);
    api.gl.glGenBuffers(1, &renderer->ubo);

    api.gl.glEnableVertexAttribArray(0);
    api.gl.glEnableVertexAttribArray(1);
    api.gl.glEnableVertexAttribArray(2);
    api.gl.glEnableVertexAttribArray(3);
    api.gl.glEnableVertexAttribArray(4);
    api.gl.glEnableVertexAttribArray(5);
    api.gl.glEnableVertexAttribArray(6);
    api.gl.glEnableVertexAttribArray(7);
    api.gl.glEnableVertexAttribArray(8);
    api.gl.glEnableVertexAttribArray(9);
    api.gl.glEnableVertexAttribArray(10);
    api.gl.glEnableVertexAttribArray(11);
    api.gl.glEnableVertexAttribArray(12);

    api.gl.glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_vertices);
    api.gl.glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
        GL_STATIC_DRAW);
    api.gl.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct cube_vertex_data), (void*)0);
    api.gl.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct cube_vertex_data), (void*)12);
    api.gl.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
        sizeof(struct cube_vertex_data), (void*)24);

    api.gl.glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_cubes);
    api.gl.glBufferData(GL_ARRAY_BUFFER, sizeof(renderer->cubes),
        renderer->cubes, GL_DYNAMIC_DRAW);
    api.gl.glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct cube_data), (void*)0);
    api.gl.glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct cube_data), (void*)sizeof(struct v4));
    api.gl.glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct cube_data), (void*)(sizeof(struct v4) * 2));
    api.gl.glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct cube_data), (void*)(sizeof(struct v4) * 3));
    api.gl.glVertexAttribIPointer(7, 3, GL_UNSIGNED_BYTE,
        sizeof(struct cube_data), (void*)(sizeof(struct v4) * 4));
    api.gl.glVertexAttribIPointer(8, 3, GL_UNSIGNED_BYTE,
        sizeof(struct cube_data), (void*)(sizeof(struct v4) * 4 + 3));
    api.gl.glVertexAttribIPointer(9, 3, GL_UNSIGNED_BYTE,
        sizeof(struct cube_data), (void*)(sizeof(struct v4) * 4 + 6));
    api.gl.glVertexAttribIPointer(10, 3, GL_UNSIGNED_BYTE,
        sizeof(struct cube_data), (void*)(sizeof(struct v4) * 4 + 9));
    api.gl.glVertexAttribIPointer(11, 3, GL_UNSIGNED_BYTE,
        sizeof(struct cube_data), (void*)(sizeof(struct v4) * 4 + 12));
    api.gl.glVertexAttribIPointer(12, 3, GL_UNSIGNED_BYTE,
        sizeof(struct cube_data), (void*)(sizeof(struct v4) * 4 + 15));

    api.gl.glVertexAttribDivisor(3, 1);
    api.gl.glVertexAttribDivisor(4, 1);
    api.gl.glVertexAttribDivisor(5, 1);
    api.gl.glVertexAttribDivisor(6, 1);
    api.gl.glVertexAttribDivisor(7, 1);
    api.gl.glVertexAttribDivisor(8, 1);
    api.gl.glVertexAttribDivisor(9, 1);
    api.gl.glVertexAttribDivisor(10, 1);
    api.gl.glVertexAttribDivisor(11, 1);
    api.gl.glVertexAttribDivisor(12, 1);

    api.gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
    api.gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        renderer->num_indices * sizeof(u32), indices, GL_STATIC_DRAW);

    u32 uniform_block_index = api.gl.glGetUniformBlockIndex(renderer->shader,
        "uniform_colors");
    api.gl.glUniformBlockBinding(renderer->shader, uniform_block_index, 0);

    api.gl.glBindBuffer(GL_UNIFORM_BUFFER, renderer->ubo);
    api.gl.glBufferData(GL_UNIFORM_BUFFER, sizeof(renderer->colors), NULL,
        GL_STATIC_DRAW);

    api.gl.glBindBufferRange(GL_UNIFORM_BUFFER, 0, renderer->ubo, 0,
        sizeof(renderer->colors));

    renderer->initialized = !gl_check_error("cube_renderer_init");
}

void cube_renderer_add(struct cube_renderer* renderer, struct cube_data* data)
{
    if (renderer->initialized)
    {
        if (renderer->num_cubes < MAX_CUBES)
        {
            renderer->cubes[renderer->num_cubes++] = *data;
        }
    }
}

void cube_renderer_flush(struct cube_renderer* renderer, struct m4* view,
    struct m4* projection)
{
    if (renderer->initialized)
    {
        api.gl.glBindVertexArray(renderer->vao);
        api.gl.glUseProgram(renderer->shader);

        u32 uniform_texture = api.gl.glGetUniformLocation(renderer->shader,
            "uniform_texture");
        u32 uniform_vp = api.gl.glGetUniformLocation(renderer->shader,
            "uniform_vp");

        if (renderer->update_color_data)
        {
            api.gl.glBufferData(GL_UNIFORM_BUFFER, sizeof(renderer->colors),
                &renderer->colors, GL_STATIC_DRAW);

            renderer->update_color_data = false;
        }

        struct m4 vp = m4_mul_m4(*view, *projection);

        api.gl.glUniform1i(uniform_texture, 0);
        api.gl.glUniformMatrix4fv(uniform_vp, 1, GL_FALSE, (GLfloat*)&vp);

        api.gl.glActiveTexture(GL_TEXTURE0);
        api.gl.glBindTexture(GL_TEXTURE_2D_ARRAY, renderer->texture);

        api.gl.glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_cubes);
        api.gl.glBufferSubData(GL_ARRAY_BUFFER, 0,
            renderer->num_cubes * sizeof(struct cube_data), renderer->cubes);

        api.gl.glDrawElementsInstanced(GL_TRIANGLES, renderer->num_indices,
            GL_UNSIGNED_INT, NULL, renderer->num_cubes);

        api.gl.glUseProgram(0);
        api.gl.glBindVertexArray(0);

        renderer->num_cubes = 0;
    }
}

s32 cube_renderer_color_add(struct cube_renderer* renderer, struct v4 color)
{
    s32 result = -1;

    if (renderer->initialized)
    {
        // Todo: we should figure out what to do when the color buffer is full
        for (u32 i = 0; i < renderer->num_colors; i++)
        {
            if (v4_equals(renderer->colors[i], color))
            {
                result = i;
                renderer->update_color_data = true;
                break;
            }
        }

        if (result == -1 && renderer->num_colors < MAX_CUBE_COLORS)
        {
            result = renderer->num_colors++;
            renderer->colors[result] = color;
            renderer->update_color_data = true;
        }
    }

    return result;
}

void cube_data_color_update(struct cube_data* cube, u32 color)
{
    for (u32 i = 0; i < 6; i++)
    {
        cube->faces[i].color = color;
    }
}

void triangle_reorder_vertices_ccw(struct v2 a, struct v2* b, struct v2* c)
{
    struct v2 temp_b = { b->x - a.x, b->y - a.y };
    struct v2 temp_c = { c->x - a.x, c->y - a.y };
    struct v2 forward =
    {
        (temp_b.x + temp_c.x) * 0.5f,
        (temp_b.y + temp_c.y) * 0.5f
    };

    // Todo: use cross product
    f32 angle = F64_PI*1.5f;
    f32 tsin = f32_sin(angle);
    f32 tcos = f32_cos(angle);

    struct v2 right =
    {
        forward.x * tcos - forward.y * tsin,
        forward.x * tsin + forward.y * tcos
    };

    f32 diff_b = v2_dot(right, temp_b);
    f32 diff_c = v2_dot(right, temp_c);

    if (diff_c > diff_b)
    {
        v2_swap(b, c);
    }
}

void mesh_render(struct mesh* mesh, struct m4* mvp, u32 texture, u32 shader,
    struct v4 color)
{
    api.gl.glBindVertexArray(mesh->vao);

    api.gl.glUseProgram(shader);

    u32 uniform_mvp = api.gl.glGetUniformLocation(shader, "MVP");
    u32 uniform_texture = api.gl.glGetUniformLocation(shader, "texture");
    u32 uniform_color = api.gl.glGetUniformLocation(shader, "uniform_color");

    api.gl.glUniform1i(uniform_texture, 0);
    api.gl.glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, (GLfloat*)mvp);
    api.gl.glUniform4fv(uniform_color, 1, (GLfloat*)&color);

    api.gl.glActiveTexture(GL_TEXTURE0);
    api.gl.glBindTexture(GL_TEXTURE_2D, texture);

    api.gl.glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT,
        NULL);

    api.gl.glUseProgram(0);
    api.gl.glBindVertexArray(0);
}

void line_render(struct game_state* state, struct v2 start, struct v2 end,
    struct v4 color, f32 depth, f32 thickness)
{
    struct v2 direction = v2_direction(start, end);

    f32 length = v2_length(direction) * 0.5f;
    f32 angle = f32_atan(direction.x, direction.y);

    struct m4 transform = m4_translate(
        start.x + direction.x * 0.5f,
        start.y + direction.y * 0.5f,
        depth);

    struct m4 rotation = m4_rotate_z(-angle);
    struct m4 scale = m4_scale_xyz(thickness, length, 0.01f);

    struct m4 model = m4_mul_m4(scale, rotation);
    model = m4_mul_m4(model, transform);

    struct m4 mvp = m4_mul_m4(model, state->camera.view);
    mvp = m4_mul_m4(mvp, state->camera.projection);

    mesh_render(&state->floor, &mvp, state->texture_tileset,
        state->shader_simple, color);
}

void gui_rect_render(struct game_state* state, f32 x, f32 y, f32 width,
    f32 height, f32 angle, struct v4 color)
{
    f32 half_width = width * 0.5f;
    f32 half_height = height * 0.5f;
    struct m4 transform = m4_translate(x + half_width, y + half_height, 0.0f);
    struct m4 rotation = m4_rotate_z(angle);
    struct m4 scale = m4_scale_xyz(half_width, half_height, 1.0f);

    struct m4 model = m4_mul_m4(scale, rotation);
    model = m4_mul_m4(model, transform);

    struct m4 projection = m4_orthographic(0.0f, state->camera.screen_width,
        0.0f, state->camera.screen_height, 0.0f, 1.0f);

    struct m4 mp = m4_mul_m4(model, projection);

    mesh_render(&state->floor, &mp, state->texture_tileset,
        state->shader_simple, color);
}

void health_bar_render(struct game_state* state, struct v2 position,
    f32 health, f32 health_max)
{
    f32 bar_length_max = 70.0f;
    f32 bar_length = health / health_max * bar_length_max;

    struct v2 screen_pos = calculate_screen_pos(position.x, position.y + 0.5f,
        0.5f, &state->camera);

    f32 x = screen_pos.x - bar_length_max * 0.5f;
    f32 y = screen_pos.y + 12.0f;
    f32 width = bar_length;
    f32 height = 10.0f;
    f32 angle = 0.0f;

    gui_rect_render(state, x, y, width, height, angle, colors[RED]);
    // gui_rect_render(state, x, y, width + 5.0f, height + 5.0f, angle,
    //     colors[BLACK]);
}

void ammo_bar_render(struct game_state* state, struct v2 position,
    f32 ammo, f32 ammo_max)
{
    f32 bar_max_size = 20;

    if (ammo_max > bar_max_size)
    {
        ammo *= bar_max_size / ammo_max;
        ammo_max = bar_max_size;
    }

    f32 bar_length_max = 70.0f;
    f32 bar_length = bar_length_max / ammo_max;

    struct v2 screen_pos = calculate_screen_pos(position.x, position.y + 0.5f,
        0.5f, &state->camera);

    f32 x = screen_pos.x - bar_length_max * 0.5f;
    f32 y = screen_pos.y;
    f32 width = bar_length - 1.0f;
    f32 height = 10.0f;
    f32 angle = 0.0f;

    for (u32 i = 0; i < ammo; i++, x += bar_length)
    {
        gui_rect_render(state, x, y, width, height, angle, colors[YELLOW]);
    }
}

void weapon_level_bar_render(struct game_state* state, struct v2 position,
    u32 level, u32 level_max)
{
    f32 bar_length_max = 70.0f;
    f32 bar_length = bar_length_max / level_max;

    struct v2 screen_pos = calculate_screen_pos(position.x, position.y + 0.5f,
        0.5f, &state->camera);

    f32 x = screen_pos.x - bar_length_max * 0.5f;
    f32 y = screen_pos.y - 12.0f;
    f32 width = bar_length - 1.0f;
    f32 height = 10.0f;
    f32 angle = 0.0f;

    for (u32 i = 0; i < level; i++, x += bar_length)
    {
        gui_rect_render(state, x, y, width, height, angle, colors[LIME]);
    }
}

void sphere_render(struct game_state* state, struct v2 position, f32 radius,
    struct v4 color, f32 depth)
{
    struct m4 transform = m4_translate(position.x, position.y, depth);
    struct m4 rotation = m4_rotate_z(0);
    struct m4 scale = m4_scale_xyz(radius, radius, radius);

    struct m4 model = m4_mul_m4(scale, rotation);
    model = m4_mul_m4(model, transform);

    struct m4 mvp = m4_mul_m4(model, state->camera.view);
    mvp = m4_mul_m4(mvp, state->camera.projection);

    mesh_render(&state->sphere, &mvp, state->texture_sphere,
        state->shader_simple, color);
}

void triangle_render(struct game_state* state, struct v2 a, struct v2 b,
    struct v2 c, struct v4 color, f32 depth)
{
    // Todo: clean
    u32 vao;
    u32 vbo;
    u32 ibo;
    u32 num_vertices = 3;
    u32 num_indices = 3;
    u32 indices[] = { 0, 1, 2 };
    u32 texture = 0;
    struct vertex vertices[] =
    {
        {{ a.x, a.y, depth }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, color },
        {{ b.x, b.y, depth }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, color },
        {{ c.x, c.y, depth }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, color }
    };

    api.gl.glGenVertexArrays(1, &vao);
    api.gl.glBindVertexArray(vao);

    api.gl.glGenBuffers(1, &vbo);
    api.gl.glBindBuffer(GL_ARRAY_BUFFER, vbo);
    api.gl.glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(struct vertex),
        vertices, GL_DYNAMIC_DRAW);

    api.gl.glGenBuffers(1, &ibo);
    api.gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    api.gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(u32),
        indices, GL_DYNAMIC_DRAW);

    api.gl.glEnableVertexAttribArray(0);
    api.gl.glEnableVertexAttribArray(1);
    api.gl.glEnableVertexAttribArray(2);
    api.gl.glEnableVertexAttribArray(3);

    // Todo: implement offsetof
    api.gl.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)0);
    api.gl.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)12);
    api.gl.glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)20);
    api.gl.glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)32);

    api.gl.glUseProgram(state->shader_simple);

    u32 uniform_mvp = api.gl.glGetUniformLocation(state->shader_simple, "MVP");
    u32 uniform_texture = api.gl.glGetUniformLocation(state->shader_simple,
        "texture");
    u32 uniform_color = api.gl.glGetUniformLocation(state->shader_simple,
        "uniform_color");

    struct m4 mvp = state->camera.view;
    mvp = m4_mul_m4(mvp, state->camera.projection);

    api.gl.glUniform1i(uniform_texture, 0);
    api.gl.glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, (GLfloat*)&mvp);
    api.gl.glUniform4fv(uniform_color, 1, (GLfloat*)&color);

    api.gl.glActiveTexture(GL_TEXTURE0);
    api.gl.glBindTexture(GL_TEXTURE_2D, texture);

    api.gl.glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, NULL);

    api.gl.glUseProgram(0);
    api.gl.glBindVertexArray(0);
}

void cursor_render(struct game_state* state)
{
    struct m4 transform = m4_translate(
        state->mouse.screen.x,
        state->camera.screen_height - state->mouse.screen.y,
        0.0f);
    struct m4 rotation = m4_identity();
    struct m4 scale = m4_scale_xyz(2.0f, 2.0f, 1.0f);

    struct m4 model = m4_mul_m4(scale, rotation);
    model = m4_mul_m4(model, transform);

    struct m4 projection = m4_orthographic(0.0f, state->camera.screen_width,
        0.0f, state->camera.screen_height, 0.0f, 1.0f);

    struct m4 mp = m4_mul_m4(model, projection);

    mesh_render(&state->floor, &mp, state->texture_tileset,
        state->shader_simple, colors[WHITE]);
}

b32 tile_ray_cast_to_direction(struct game_state* state, struct v2 start,
    struct v2 direction, f32 length_max, struct ray_cast_collision* collision,
    b32 render)
{
    // Returns true if the ray collides before length_max
    b32 result = false;
    f32 tile_size = WALL_SIZE;
    u32 iteration = 0;

    struct v2 current = start;
    struct v2 step =
    {
        (direction.x > 0 ? tile_size : -tile_size),
        (direction.y > 0 ? tile_size : -tile_size)
    };
    struct v2 wall =
    {
        direction.x ? (f32_round(current.x) + step.x * 0.5f) : current.x,
        direction.y ? (f32_round(current.y) + step.y * 0.5f) : current.y
    };

    // LOG("Ray cast from %.2fx%.2f to %.2fx%.2f ", start.x, start.y, end.x,
    //     end.y);

    while (true)
    {
        struct v2 distance_to_wall = { wall.x - current.x, wall.y - current.y };
        struct v2 previous = current;
        struct v2 cut_x = wall;
        struct v2 cut_y = wall;

        if (direction.x)
        {
            cut_x.y = current.y + direction.y * (distance_to_wall.x /
                direction.x);
        }

        if (direction.y)
        {
            cut_y.x = current.x + direction.x * (distance_to_wall.y /
                direction.y);
        }

        b32 check_x = false;
        b32 check_y = false;

        if (direction.y && direction.x)
        {
            if (v2_distance(current, cut_x) < v2_distance(current, cut_y))
            {
                current.x = wall.x;
                current.y = cut_x.y;

                wall.x += step.x;

                check_x = true;
            }
            else if (v2_distance(current, cut_x) > v2_distance(current, cut_y))
            {
                current.x = cut_y.x;
                current.y = wall.y;

                wall.y += step.y;

                check_y = true;
            }
            else
            {
                current = wall;

                wall.x += step.x;
                wall.y += step.y;
            }

        }
        else if (direction.x)
        {
            current.x = wall.x;
            current.y = cut_x.y;

            wall.x += step.x;

            check_x = true;
        }
        else
        {
            current.x = cut_y.x;
            current.y = wall.y;

            wall.y += step.y;

            check_y = true;
        }

        f32 ray_length = v2_distance(current, start);

        if (ray_length > length_max)
        {
            current = v2_mul_f32(direction, length_max);
            current.x += start.x;
            current.y += start.y;

            if (render)
            {
                line_render(state, previous, current,
                    colors[4 + iteration++ % 2], 0.025f, 0.0125f);
            }

            if (collision)
            {
                collision->ray_length = length_max;
            }

            break;
        }

        if (render)
        {
            line_render(state, previous, current, colors[4 + iteration++ % 2],
                0.025f, 0.0125f);
        }

        f32 epsilon = 0.001f;

        struct level* level = &state->level;

        // Todo: these both cases can possibly be merged
        if (check_x)
        {
            struct v2 tile_left = { current.x - epsilon, current.y };
            struct v2 tile_right = { current.x + epsilon, current.y };

            if (!tile_is_free(level, tile_left) ||
                !tile_is_free(level, tile_right))
            {
                if (collision)
                {
                    collision->ray_length = ray_length;
                    collision->wall_start.x = current.x;
                    collision->wall_end.x = current.x;
                    collision->wall_start.y = wall.y - step.y;
                    collision->wall_end.y = wall.y;
                }

                result = true;
                break;
            }
        }

        if (check_y)
        {
            struct v2 tile_top = { current.x, current.y + epsilon };
            struct v2 tile_bottom = { current.x, current.y - epsilon };

            if (!tile_is_free(level, tile_top) ||
                !tile_is_free(level, tile_bottom))
            {
                if (collision)
                {
                    collision->ray_length = ray_length;
                    collision->wall_start.y = current.y;
                    collision->wall_end.y = current.y;
                    collision->wall_start.x = wall.x - step.x;
                    collision->wall_end.x = wall.x;
                }

                result = true;
                break;
            }
        }
    }

    if (collision)
    {
        collision->position = current;
    }

    // LOG("length: %.2f\n", result);

    return result;
}

b32 tile_ray_cast_to_angle(struct game_state* state, struct v2 start,
     f32 angle, f32 length_max, struct ray_cast_collision* collision,
     b32 render)
{
    // Returns true if the ray collides before length_max
    b32 result = false;

    struct v2 direction = v2_direction_from_angle(angle);

    result = tile_ray_cast_to_direction(state, start, direction, length_max,
        collision, render);

    return result;
}

b32 tile_ray_cast_to_position(struct game_state* state, struct v2 start,
    struct v2 end, struct ray_cast_collision* collision, b32 render)
{
    // Returns true if the ray is longer than the distance between start and end
    b32 result = false;

    struct v2 direction = v2_normalize(v2_direction(start, end));
    struct ray_cast_collision temp = { 0 };
    f32 length = v2_distance(end, start);

    tile_ray_cast_to_direction(state, start, direction, length,
        &temp, render);

    result = f32_abs(temp.ray_length - length) < 0.01f;

    if (collision)
    {
        *collision = temp;
    }

    return result;
}

b32 collision_point_to_aabb(f32 x, f32 y, f32 min_x, f32 max_x, f32 min_y,
    f32 max_y)
{
    b32 result = false;

    result = x >= min_x && x <= max_x && y >= min_y && y <= max_y;

    return result;
}

b32 collision_point_to_obb(struct v2 pos, struct v2 corners[4])
{
    b32 result = false;

    b32 result_top = v2_cross(v2_direction(corners[0], corners[1]),
        v2_direction(corners[0], pos)) < 0.0f;
    b32 result_right = v2_cross(v2_direction(corners[1], corners[2]),
        v2_direction(corners[1], pos)) < 0.0f;
    b32 result_bottom = v2_cross(v2_direction(corners[2], corners[3]),
        v2_direction(corners[2], pos)) < 0.0f;
    b32 result_left = v2_cross(v2_direction(corners[3], corners[0]),
        v2_direction(corners[3], pos)) < 0.0f;

    result = result_top && result_right && result_bottom && result_left;

    return result;
}

void level_generate(struct game_state* state, struct level* level,
    struct level* layout_mask)
{
    u32 width = layout_mask->width;
    u32 height = layout_mask->height;
    u32 start_x = layout_mask->start_pos.x;
    u32 start_y = layout_mask->start_pos.y;

    if (!width || !height)
    {
        LOG("Error: no level width or height set: %u x %u\n", width, height);
        return;
    }

    u64 size = width * height;
    u8* data = 0;

    data = stack_alloc(&state->stack, size);
    memory_copy(layout_mask->tile_types, data, size);

    memory_set(level, sizeof(struct level), 0);

    // Generate randomized level
    // Each room will be numbered, starting from three
    u32 room_index = 3;

    for (u32 y = 0; y < height; y++)
    {
        for (u32 x = 0; x < width; x++)
        {
            u32 index = y * width + x;

            // Don't create room on an empty space (zeroed)
            if (!data[index])
            {
                continue;
            }

            while (true)
            {
                u32 action = u32_random_number_get(state, 0, 2);

                // Continue previous room
                if (action == 0)
                {
                    u32 prev = index - 1;

                    // Skip check if previous room is an empty space
                    if (x == 0 || !data[prev])
                    {
                        continue;
                    }

                    data[index] = data[prev];
                }
                // Continue upper room
                else if (action == 1)
                {
                    u32 prev = index - width;

                    // Skip check if upper room is an empty space
                    if (y == 0 || !data[prev])
                    {
                        continue;
                    }

                    data[index] = data[prev];
                }
                else
                {
                    data[index] = room_index++;
                }

                break;
            }
        }
    }

    // Start room
    data[start_y * width + start_x] = 2;

    for (u32 y = 0; y < height; y++)
    {
        for (u32 x = 0; x < width; x++)
        {
            LOG("%d, ", data[y * width + x]);
        }

        LOG("\n");
    }

    // Room dimensions, always multiple of two!
    u32 room_width = 4;
    u32 room_height = 4;

    // Calculate level dimensions; +1 for outer wall
    level->width = height * room_height + 1;
    level->height = width * room_width + 1;

    // Create walls around rooms
    for (u32 y = 0; y < height; y++)
    {
        for (u32 x = 0; x < width; x++)
        {
            u32 room_index = y * width + x;
            u32 room_type = data[y * width + x];

            for (u32 j = 0; j < room_height; j++)
            {
                for (u32 k = 0; k < room_width; k++)
                {
                    u32 tile_x = room_width * x + k;
                    u32 tile_y = room_height * y + j;
                    u32 tile_type = room_type;
                    u32 tile_index = tile_y * level->width + tile_x;
                    s32 tile_left = tile_index - 1;
                    s32 tile_up = tile_index - level->width;
                    s32 room_left = room_index - 1;
                    s32 room_up = room_index - width;

                    if (j == 0 && k == 0 && tile_left >= 0 && tile_up >= 0 &&
                        level->tile_types[tile_left] == TILE_WALL &&
                        level->tile_types[tile_up] == TILE_WALL)
                    {
                        tile_type = TILE_WALL;
                    }
                    else if (room_type == TILE_NOTHING)
                    {
                        tile_type = TILE_NOTHING;

                        if (k == 0 && room_left >= 0 && x != 0 &&
                            data[room_left] != TILE_NOTHING)
                        {
                            tile_type = TILE_WALL;
                        }

                        if (j == 0 && room_up >= 0 && y != 0 &&
                            data[room_up] != TILE_NOTHING)
                        {
                            tile_type = TILE_WALL;
                        }
                    }
                    else if (tile_x == 0 || tile_y == 0 ||
                        (k == 0 && data[room_left] != room_type) ||
                        (j == 0 && data[room_up] != room_type))
                    {
                        tile_type = TILE_WALL;
                    }

                    level->tile_types[tile_index] = tile_type;
                }
            }
        }
    }

    // Set outer wall
    for (u32 x = 0, y = level->height - 1; x < level->width; x++)
    {
        u32 tile_index = y * level->width + x;
        u32 tile_prev = tile_index - level->width;
        u32 tile_type = level->tile_types[tile_prev] == TILE_NOTHING ?
            TILE_NOTHING : TILE_WALL;

        level->tile_types[tile_index] = tile_type;
    }

    for (u32 y = 0, x = level->width - 1; y < level->height; y++)
    {
        u32 tile_index = y * level->width + x;
        u32 tile_prev = tile_index - 1;
        u32 tile_type = level->tile_types[tile_prev] == TILE_NOTHING ?
            TILE_NOTHING : TILE_WALL;

        level->tile_types[tile_index] = tile_type;
    }

    // Find doors
    for (u32 i = 3; i < room_index - 1; i++)
    {
        for (u32 j = i + 1; j < room_index; j++)
        {
            struct v2 tile_i = tile_random_get(state, level, i);
            struct v2 tile_j = tile_random_get(state, level, j);

            u32 walls[256] = { 0 };
            u32 wall_count = 0;

            // Find each wall block between rooms i and j
            for (u32 y = 1; y < level->height - 1; y++)
            {
                for (u32 x = 1; x < level->width - 1; x++)
                {
                    u32 tile_index = y * level->width + x;

                    if (level->tile_types[tile_index] == TILE_WALL)
                    {
                        u32 tile_prev = level->tile_types[tile_index - 1];
                        u32 tile_next = level->tile_types[tile_index + 1];
                        u32 tile_up   = level->tile_types[tile_index -
                            level->width];
                        u32 tile_down = level->tile_types[tile_index +
                            level->width];

                        if ((i == tile_prev && j == tile_next) ||
                            (i == tile_next && j == tile_prev) ||
                            (i == tile_up && j == tile_down) ||
                            (i == tile_down && j == tile_up))
                        {
                            walls[wall_count++] = tile_index;
                        }
                    }
                }
            }

            // Pick a random block and mark it as door
            if (wall_count)
            {
                u32 path_max_length = 20;
                u32 path_length = path_find(level, tile_i, tile_j, NULL, 0);

                if (!path_length || path_length > path_max_length)
                {
                    u32 num_doors_to_open = 1;

                    if (wall_count > 8)
                    {
                        num_doors_to_open = 3;
                    }
                    else if (wall_count > 4)
                    {
                        num_doors_to_open = 2;
                    }

                    u32 random_doors[3];

                    for (u32 k = 0; k < num_doors_to_open; k++)
                    {
                        u32 random_door = u32_random_number_get(state, 0,
                            wall_count - 1);
                        b32 door_picked_already = false;

                        for (u32 l = 0; l < k; l++)
                        {
                            s32 difference = random_door - random_doors[l];

                            if (difference <= 1 && difference >= -1)
                            {
                                door_picked_already = true;
                                break;
                            }
                        }

                        if (!door_picked_already)
                        {
                            random_doors[k] = random_door;

                            level->tile_types[walls[random_door]] = TILE_FLOOR;
                            level->tile_sprites[walls[random_door]] = 16;
                        }
                    }
                }
            }
        }
    }

    // Open start room door
    u32 room_center_x = (u32)(room_width * 0.5f);
    u32 room_center_y = (u32)(room_height * 0.5f);

    level->start_pos.x = start_x * room_width + room_center_x;
    level->start_pos.y = start_y * room_height + room_center_y;

    // Always pointing south
    u32 start_door_index = (start_y * room_height + room_center_y +
        -room_center_y) * level->width + start_x * room_width + room_center_x;

    level->tile_types[start_door_index] = TILE_FLOOR;
    level->tile_sprites[start_door_index] = 16;

    // Create floors
    for (u32 i = 0; i < level->width * level->height; i++)
    {
        u8 type = level->tile_types[i];

        if (!level->tile_sprites[i])
        {
            if (type == TILE_WALL)
            {
                level->tile_sprites[i] = 8;
            }
            else if (type == 2)
            {
                // Use specific tile for start room
                level->tile_types[i] = TILE_START;
                level->tile_sprites[i] = 17;
            }
            else if (type > 2)
            {
                level->tile_types[i] = TILE_FLOOR;
                level->tile_sprites[i] = 8 + type % 8;
            }
        }
        else if (level->tile_sprites[i] == 16)
        {
            level->tile_types[i] = TILE_DOOR;
        }
    }

    for (u32 y = 0; y < level->height; y++)
    {
        for (u32 x = 0; x < level->width; x++)
        {
            LOG("%d, ", level->tile_sprites[y * level->width + x]);
        }

        LOG("\n");
    }

    stack_free(&state->stack);
}

void level_render(struct game_state* state, struct level* level)
{
    // Todo: fix level rendering glitch (a wall block randomly drawn in a
    //       wrong place)

    struct sprite_data data;
    data.color = colors[WHITE];

    for (u32 y = 0; y < level->height; y++)
    {
        for (u32 x = 0; x < level->width; x++)
        {
            struct v4 color = colors[WHITE];

            f32 top = y + TILE_WALL * 0.5f;
            f32 bottom = y - TILE_WALL * 0.5f;
            f32 left = x - TILE_WALL * 0.5f;
            f32 right = x + TILE_WALL * 0.5f;

            struct v2 corners[] =
            {
                { left, top },
                { right, top },
                { right, bottom },
                { left, bottom }
            };

            b32 collision_obb = collision_point_to_obb(state->mouse.world,
                corners);

            if (collision_obb)
            {
                color = colors[RED];
            }

            u64 tile_index = y * level->width + x;
            u8 tile_type = level->tile_types[tile_index];

            if (tile_type == TILE_WALL)
            {
                struct m4 transform = m4_translate(x, y, 0.5f);
                struct m4 rotation = m4_rotate_z(0.0f);
                struct m4 scale = m4_scale_all(WALL_SIZE * 0.5f);

                struct m4 model = m4_mul_m4(scale, rotation);
                model = m4_mul_m4(model, transform);

                struct m4 mvp = m4_mul_m4(model, state->camera.view);
                mvp = m4_mul_m4(mvp, state->camera.projection);

                mesh_render(&state->wall, &mvp, state->texture_tileset,
                    state->shader, color);
            }
            else if (tile_type != TILE_NOTHING)
            {
                struct m4 transform = m4_translate(x, y, 0.0f);
                struct m4 rotation = m4_rotate_z(0.0f);
                struct m4 scale = m4_scale_all(WALL_SIZE * 0.5f);

                struct m4 model = m4_mul_m4(scale, rotation);
                model = m4_mul_m4(model, transform);

                data.model = model;
                data.texture = level->tile_sprites[tile_index];
                data.color = colors[WHITE];

                sprite_renderer_add(&state->sprite_renderer, &data);
            }
        }
    }

    if (state->level_cleared)
    {
        cube_data_color_update(&level->elevator_light, LIME);
    }

    cube_renderer_add(&state->cube_renderer, &level->elevator_light);
}

void collision_map_render(struct game_state* state)
{
    for (u32 i = 0; i < state->num_cols_static; i++)
    {
        line_render(state, state->cols_static[i].start,
            state->cols_static[i].end, colors[RED], WALL_SIZE + 0.01f,
            0.025f);
    }
}

b32 collision_circle_to_circle(struct v2 position_a, f32 radius_a,
    struct v2 position_b, f32 radius_b)
{
    f32 result = v2_distance(position_a, position_b) < (radius_a + radius_b);

    return result;
}

b32 collision_circle_to_rect(struct v2 circle, f32 circle_radius, f32 rect_top,
    f32 rect_bottom, f32 rect_left, f32 rect_right, struct v2* col)
{
    f32 result;

    struct v2 collision_point = circle;

    if (circle.x < rect_left)
    {
        collision_point.x = rect_left;
    }
    else if (circle.x > rect_right)
    {
        collision_point.x = rect_right;
    }

    if (circle.y < rect_bottom)
    {
        collision_point.y = rect_bottom;
    }
    else if (circle.y > rect_top)
    {
        collision_point.y = rect_top;
    }

    result = collision_circle_to_circle(circle, circle_radius, collision_point,
         0.0f);

    if (result && col)
    {
        col->x = circle.x > 0 ? rect_right : rect_left;
        col->y = circle.y > 0 ? rect_top : rect_bottom;
    }

    return result;
}

b32 collision_wall_resolve(f32 wall_x, f32 pos_x, f32 pos_y, f32 move_delta_x,
    f32 move_delta_y, f32 wall_size, f32* move_time)
{
    b32 result = false;

    if (move_delta_x != 0.0f)
    {
        f32 diff = wall_x - pos_x;
        f32 t = diff / move_delta_x;
        f32 y = pos_y + move_delta_y * t;

        if (y >= -wall_size && y <= wall_size)
        {
            if (t < *move_time && t >= 0.0f)
            {
                // Todo: what is the best amount for the epsilon?
                f32 epsilon = 0.01f;
                *move_time = MAX(0.0f, t - epsilon);

                result = true;
            }
        }
    }

    return result;
}

b32 collision_corner_resolve(struct v2 rel, struct v2 move_delta, f32 radius,
    f32* move_time, struct v2* normal)
{
    b32 result = false;

    struct v2 rel_new =
    {
        rel.x + move_delta.x,
        rel.y + move_delta.y
    };

    struct v2 col = { 0.0f };

    f32 wall_half = WALL_SIZE * 0.5f;

    if (collision_circle_to_rect(rel_new, radius, wall_half, -wall_half,
        -wall_half, wall_half, &col))
    {
        // 1. find the closest point on the line from
        //    relative position - relative_position_new
        struct v2 plr_to_col =
        {
            col.x - rel.x,
            col.y - rel.y
        };

        struct v2 plr_to_new =
        {
            rel_new.x - rel.x,
            rel_new.y - rel.y
        };

        f32 dot = v2_dot(plr_to_col, plr_to_new);
        f32 length_squared = v2_length_squared(plr_to_new);
        f32 temp = dot / length_squared;

        struct v2 closest =
        {
            rel.x + plr_to_new.x * temp,
            rel.y + plr_to_new.y * temp
        };

        // 2. calculate distance from closest point to the perfect point
        f32 distance_closest_to_collision = v2_distance(closest, col);

        f32 distance_closest_to_perfect = f32_sqrt(f32_square(radius) -
            f32_square(distance_closest_to_collision));

        // 3. calculate distance from relative point to perfect point
        f32 distance_closest_to_relative = v2_distance(closest, rel);

        f32 distance_relative_to_perfect = distance_closest_to_relative -
            distance_closest_to_perfect;

        struct v2 velocity_direction = v2_normalize(move_delta);

        struct v2 perfect =
        {
            rel.x + velocity_direction.x * distance_relative_to_perfect,
            rel.y + velocity_direction.y * distance_relative_to_perfect
        };

        // 4. calculate t and normal
        f32 distance_relative_to_new = v2_length(plr_to_new);

        f32 t = distance_relative_to_perfect / distance_relative_to_new;

        if (t < *move_time)
        {
            // Todo: what is the best amount for the epsilon?
            f32 epsilon = 0.01f;
            *move_time = MAX(0.0f, t - epsilon);

            normal->x = perfect.x - col.x;
            normal->y = perfect.y - col.y;

            *normal = v2_normalize(*normal);

            result = true;
        }
    }

    return result;
}

b32 check_tile_collisions(struct level* level, struct v2* pos, struct v2* vel,
    struct v2 move_delta, f32 radius, f32 bounce_factor)
{
    b32 result = false;

    f32 wall_low = WALL_SIZE * 0.5f;
    f32 wall_high = wall_low + radius;

    f32 margin_x = move_delta.x + radius;
    f32 margin_y = move_delta.y + radius;

    f32 min_x = pos->x + wall_low - margin_x - WALL_SIZE;
    f32 min_y = pos->y + wall_low - margin_y - WALL_SIZE;

    u32 start_x = min_x < 0.0f ? 0 : (u32)min_x / WALL_SIZE;
    u32 start_y = min_y < 0.0f ? 0 : (u32)min_y / WALL_SIZE;
    u32 end_x = (u32)((pos->x + wall_low + margin_x) / WALL_SIZE) + 1;
    u32 end_y = (u32)((pos->y + wall_low + margin_y) / WALL_SIZE) + 1;

    f32 time_remaining = 1.0f;

    for (u32 i = 0; i < 4 && time_remaining > 0.0f; i++)
    {
        f32 time = 1.0f;
        struct v2 normal = { 0.0f };

        for (u32 y = start_y; y <= end_y; y++)
        {
            for (u32 x = start_x; x <= end_x; x++)
            {
                if (level->tile_types[y * level->width + x] != 1)
                {
                    continue;
                }

                struct v2 rel =
                {
                    pos->x - x,
                    pos->y - y
                };

                if (collision_wall_resolve(-wall_high, rel.y, rel.x,
                    move_delta.y, move_delta.x, wall_low, &time))
                {
                    normal.x = 0.0f;
                    normal.y = -1.0f;
                    result = true;
                }
                if (collision_wall_resolve(-wall_high, rel.x, rel.y,
                    move_delta.x, move_delta.y, wall_low, &time))
                {
                    normal.x = -1.0f;
                    normal.y = 0.0f;
                    result = true;
                }
                if (collision_wall_resolve(wall_high, rel.y, rel.x,
                    move_delta.y, move_delta.x, wall_low, &time))
                {
                    normal.x = 0.0f;
                    normal.y = 1.0f;
                    result = true;
                }
                if (collision_wall_resolve(wall_high, rel.x, rel.y,
                    move_delta.x, move_delta.y, wall_low, &time))
                {
                    normal.x = 1.0f;
                    normal.y = 0.0f;
                    result = true;
                }

                if (v2_length(normal) == 0.0f)
                {
                    if (collision_corner_resolve(rel, move_delta, radius, &time,
                        &normal))
                    {
                        result = true;
                    }
                }
            }
        }

        pos->x += move_delta.x * time;
        pos->y += move_delta.y * time;

        f32 vel_dot = bounce_factor * v2_dot(*vel, normal);

        vel->x -= vel_dot * normal.x;
        vel->y -= vel_dot * normal.y;

        f32 move_delta_dot = bounce_factor * v2_dot(move_delta, normal);

        move_delta.x -= move_delta_dot * normal.x;
        move_delta.y -= move_delta_dot * normal.y;

        time_remaining -= time * time_remaining;
    }

    return result;
}

void particle_sphere_create(struct game_state* state, struct v2 position,
    struct v2 velocity, struct v4 color, f32 radius_start, f32 radius_end,
    f32 time)
{
    if (++state->free_particle_sphere == MAX_PARTICLES)
    {
        state->free_particle_sphere = 0;
    }

    struct particle_sphere* particle = &state->particle_spheres[
        state->free_particle_sphere];

    *particle = (struct particle_sphere){ 0 };
    particle->position = position;
    particle->velocity = velocity;
    particle->color = color;
    particle->alive = true;
    particle->radius_start = radius_start;
    particle->radius_end = radius_end;
    particle->time_start = time;
    particle->time_current = particle->time_start;
    particle->radius_current = particle->radius_start;
}

void particle_line_create(struct game_state* state, struct v2 start,
    struct v2 end, struct v4 color_start, struct v4 color_end, f32 time)
{
    if (++state->free_particle_line == MAX_PARTICLES)
    {
        state->free_particle_line = 0;
    }

    struct particle_line* particle =
        &state->particle_lines[state->free_particle_line];

    *particle = (struct particle_line){ 0 };
    particle->start = start;
    particle->end = end;
    particle->color_start = color_start;
    particle->color_end = color_end;
    particle->alive = true;
    particle->time_start = time;
    particle->time_current = particle->time_start;
}

void bullet_create(struct game_state* state, struct v2 position,
    struct v2 start_velocity, struct v2 direction, f32 speed, f32 damage,
    b32 player_owned, f32 size)
{
    if (++state->free_bullet == MAX_BULLETS)
    {
        state->free_bullet = 0;
    }

    struct bullet* bullet = &state->bullets[state->free_bullet];
    bullet->body.position = position;
    bullet->body.velocity = start_velocity;
    bullet->body.velocity.x += direction.x * speed;
    bullet->body.velocity.y += direction.y * speed;
    bullet->alive = true;
    bullet->damage = damage;
    bullet->player_owned = player_owned;
    bullet->start = bullet->body.position;

    particle_sphere_create(state, position, (struct v2){ 0.0f, 0.0f },
        colors[YELLOW], size, size * 5.0f, 0.10f);
}

struct item* item_create(struct game_state* state, struct v2 position, u32 type)
{
    struct item* result = NULL;

    if (type > ITEM_NONE && type < ITEM_COUNT)
    {
        if (++state->free_item == MAX_ITEMS)
        {
            state->free_item = 0;
        }

        result = &state->items[state->free_item];
        result->body.position = position;
        result->alive = ITEM_ALIVE_TIME;
        result->type = type;

        result->cube.faces[0].texture = 4 + type - 1;

        u32 color = OLIVE;

        switch (type)
        {
            case ITEM_HEALTH:
            {
                color = WHITE;
            } break;
            case ITEM_WEAPON_LEVEL_UP:
            {
                color = LIME;
            } break;
        }

        for (u32 i = 0; i < 6; i++)
        {
            result->cube.faces[i].color = color;
        }
    }

    return result;
}

void gun_shot_register(struct game_state* state, struct v2 position, f32 volume)
{
    if (state->num_gun_shots < MAX_GUN_SHOTS)
    {
        struct gun_shot* shot = &state->gun_shots[state->num_gun_shots++];

        shot->position = position;
        shot->volume = volume;
    }
}

struct weapon weapon_create(u32 type)
{
    struct weapon weapon = { 0 };

    switch (type)
    {
        case WEAPON_PISTOL:
        {
            weapon.type = WEAPON_PISTOL;
            weapon.level = 1;
            weapon.last_shot = 0.0f;
            weapon.fired = false;
            weapon.ammo_max = weapon.ammo = 12;
            weapon.fire_rate = 0.0f;
            weapon.reload_time = 0.8f / weapon.ammo;
            weapon.reloading = false;
            weapon.spread = 0.0125f;
            weapon.projectile_size = PROJECTILE_RADIUS;
            weapon.projectile_speed = PROJECTILE_SPEED;
            weapon.projectile_damage = 20.0f;
        } break;
        case WEAPON_MACHINEGUN:
        {
            weapon.type = WEAPON_MACHINEGUN;
            weapon.level = 1;
            weapon.last_shot = 0.0f;
            weapon.fired = false;
            weapon.ammo_max = weapon.ammo = 40;
            weapon.fire_rate = 0.075f;
            weapon.reload_time = 1.1f / weapon.ammo;
            weapon.reloading = false;
            weapon.spread = 0.035f;
            weapon.projectile_size = PROJECTILE_RADIUS * 0.75f;
            weapon.projectile_speed = PROJECTILE_SPEED;
            weapon.projectile_damage = 10.0f;
        } break;
        case WEAPON_SHOTGUN:
        {
            weapon.type = WEAPON_SHOTGUN;
            weapon.level = 1;
            weapon.last_shot = 0.0f;
            weapon.fired = false;
            weapon.ammo_max = weapon.ammo = 8;
            weapon.fire_rate = 0.5f;
            weapon.reload_time = 3.0f / weapon.ammo;
            weapon.reloading = false;
            weapon.spread = 0.125f;
            weapon.projectile_size = PROJECTILE_RADIUS * 0.25f;
            weapon.projectile_speed = PROJECTILE_SPEED;
            weapon.projectile_damage = 7.5f;
        } break;
    }

    return weapon;
}

void weapon_shoot(struct game_state* state, struct weapon* weapon, bool player)
{
    if (weapon->type == WEAPON_PISTOL)
    {
        if (weapon->last_shot <= 0.0f && weapon->ammo && !weapon->fired &&
            !weapon->reloading)
        {
            struct v2 randomized = v2_rotate(weapon->direction,
                f32_random_number_get(state, -weapon->spread, weapon->spread));

            bullet_create(state, weapon->position, weapon->velocity, randomized,
                weapon->projectile_speed, weapon->projectile_damage, player,
                weapon->projectile_size);

            weapon->fired = true;

            if (--weapon->ammo == 0)
            {
                weapon->reloading = true;
                weapon->last_shot = weapon->reload_time;
            }
            else
            {
                weapon->last_shot = weapon->fire_rate;
            }

            gun_shot_register(state, weapon->position, 10.0f);
        }
    }
    else if (weapon->type == WEAPON_MACHINEGUN)
    {
        if (weapon->last_shot <= 0.0f && weapon->ammo && !weapon->fired &&
            !weapon->reloading)
        {
            struct v2 randomized = v2_rotate(weapon->direction,
                f32_random_number_get(state, -weapon->spread, weapon->spread));

            bullet_create(state, weapon->position, weapon->velocity, randomized,
                weapon->projectile_speed, weapon->projectile_damage, player,
                weapon->projectile_size);

            if (--weapon->ammo == 0)
            {
                weapon->reloading = true;
                weapon->last_shot = weapon->reload_time;
            }
            else
            {
                weapon->last_shot = weapon->fire_rate;
            }

            gun_shot_register(state, weapon->position, 10.0f);
        }
    }
    else if (weapon->type == WEAPON_SHOTGUN)
    {
        // Todo: shotgun should be able to interrupt reloading and shoot
        // automatically after the next shell is loaded
        if (weapon->ammo && !weapon->fired && weapon->last_shot <= 0.0f)
        {
            weapon->reloading = false;

            for (u32 i = 0; i < 10; i++)
            {
                struct v2 randomized = v2_rotate(weapon->direction,
                    f32_random_number_get(state, -weapon->spread,
                        weapon->spread));

                f32 speed = f32_random_number_get(state,
                    0.75f * weapon->projectile_speed, weapon->projectile_speed);

                bullet_create(state, weapon->position, weapon->velocity,
                    randomized, speed, weapon->projectile_damage, player,
                    weapon->projectile_size);
            }

            weapon->fired = true;

            if (--weapon->ammo == 0)
            {
                weapon->reloading = true;
                weapon->last_shot = weapon->reload_time;
            }
            else
            {
                weapon->last_shot = weapon->fire_rate;
            }

            gun_shot_register(state, weapon->position, 10.0f);
        }
    }
}

void weapon_reload(struct weapon* weapon, b32 full_reload)
{
    if (weapon->type == WEAPON_PISTOL)
    {
        weapon->ammo = 0;
        weapon->reloading = true;
        weapon->last_shot = weapon->reload_time;
    }
    else if (weapon->type == WEAPON_MACHINEGUN)
    {
        weapon->ammo = 0;
        weapon->reloading = true;
        weapon->last_shot = weapon->reload_time;
    }
    else if (weapon->type == WEAPON_SHOTGUN)
    {
        weapon->ammo = full_reload ? 0 : weapon->ammo;

        if (weapon->ammo < weapon->ammo_max)
        {
            weapon->reloading = true;
            weapon->last_shot = weapon->reload_time;
        }
    }
}

b32 weapon_level_up(struct weapon* weapon)
{
    b32 result = false;

    if (weapon->level < WEAPON_LEVEL_MAX)
    {
        weapon->level++;

        weapon->ammo_max *= 1.1f;
        weapon->projectile_damage *= 1.1f;
        weapon->reload_time *= 0.9f;
        weapon->fire_rate *= 0.9f;
        result = true;
    }

    return result;
}

b32 enemy_sees_player(struct game_state* state, struct enemy* enemy,
    struct player* player)
{
    b32 result = false;

    // Todo: maybe use direction_in_range() here instead
    struct v2 direction_player = v2_direction(enemy->body.position,
        player->body.position);
    struct v2 direction_current = v2_direction_from_angle(
        enemy->body.angle);

    f32 angle_player = v2_angle(direction_player, direction_current);

    // Todo: enemy AI goes nuts if two enemies are on top of each other and all
    // collisions are checked. Check collision against static (walls) and player
    // for now
    f32 player_ray_cast = ray_cast_body(state, enemy->eye_position,
        player->body, NULL, COLLISION_STATIC | COLLISION_PLAYER);

    result = enemy->state != ENEMY_STATE_SLEEP && player->alive &&
        angle_player < ENEMY_LINE_OF_SIGHT_HALF && player_ray_cast > 0.0f &&
        player_ray_cast < ENEMY_LINE_OF_SIGHT_DISTANCE;

    return result;
}

b32 enemy_hears_gun_shot(struct game_state* state, struct enemy* enemy)
{
    b32 result = false;
    f32 closest = ENEMY_GUN_FIRE_HEAR_DISTANCE;

    for (u32 i = 0; i < state->num_gun_shots; i++)
    {
        struct gun_shot* shot = &state->gun_shots[i];

        f32 distance = v2_distance(enemy->body.position, shot->position);

        if (distance < closest && distance < shot->volume)
        {
            enemy->gun_shot_position = shot->position;
            closest = distance;
            result = true;
        }
    }

    return result;
}

f32 enemy_reaction_time_get(struct game_state* state, u32 state_enemy)
{
    f32 result = 0.0f;
    f32 reaction_multiplier = 1.0f;

    switch (state_enemy)
    {
        case ENEMY_STATE_SHOOT:
        case ENEMY_STATE_RUSH_TO_TARGET:
        case ENEMY_STATE_REACT_TO_PLAYER_SEEN:
        case ENEMY_STATE_REACT_TO_BEING_SHOT_AT:
        case ENEMY_STATE_REACT_TO_GUN_SHOT:
        {
            // Lower reaction time
            reaction_multiplier = 0.5f;
        } break;
        case ENEMY_STATE_SLEEP:
        {
            // Higher reaction time
            reaction_multiplier = 2.0f;
        } break;
        default:
        {
            // Normal reaction time
            reaction_multiplier = 1.0f;
        } break;
    }

    result = f32_random_number_get(state, ENEMY_REACTION_TIME_MIN,
        ENEMY_REACTION_TIME_MAX) * reaction_multiplier;

    return result;
}

f32 turn_amount_calculate(f32 angle_from, f32 angle_to)
{
    f32 circle = F64_PI * 2.0f;
    f32 result = 0.0f;
    f32 original_from = angle_from;
    f32 original_to = angle_to;

    while (angle_to < 0.0f)
    {
        angle_to += circle;
    }

    while (angle_from < 0.0f)
    {
        angle_from += circle;
    }

    f32 diff_clockwise = 0.0f;
    f32 diff_counter_clockwise = 0.0f;

    if (angle_to < angle_from)
    {
        diff_clockwise = angle_to + circle - angle_from;
        diff_counter_clockwise = angle_from - angle_to;
    }
    else
    {
        diff_clockwise = angle_to - angle_from;
        diff_counter_clockwise = angle_from + circle -
            angle_to;
    }

    result = MIN(diff_clockwise, diff_counter_clockwise);
    result *= diff_clockwise > diff_counter_clockwise ? 1.0f : -1.0f;

    return result;
}

void enemy_look_towards_angle(struct enemy* enemy, f32 angle)
{
    enemy->turn_amount = turn_amount_calculate(enemy->body.angle, angle);
}

void enemy_look_towards_direction(struct enemy* enemy, struct v2 direction)
{
    enemy_look_towards_angle(enemy, f32_atan(direction.y, direction.x));
}

void enemy_look_towards_position(struct enemy* enemy, struct v2 position)
{
    struct v2 direction = v2_direction(enemy->body.position, position);

    enemy_look_towards_angle(enemy, f32_atan(direction.y, direction.x));
}

void enemy_calculate_path_to_target(struct game_state* state,
    struct level* level, struct enemy* enemy)
{
    enemy->path_length = path_find(level, enemy->body.position,
        enemy->target, enemy->path, MAX_PATH);
    path_trim(state, enemy->body.position, enemy->path,
        &enemy->path_length);
    enemy_look_towards_position(enemy, enemy->path[0]);
    enemy->path_index = 0;
}

f32 enemy_turn_speed_get(struct enemy* enemy)
{
    f32 result = 0.0f;
    f32 speed_multiplier = 1.0f;

    switch (enemy->state)
    {
        case ENEMY_STATE_SHOOT:
        case ENEMY_STATE_RUSH_TO_TARGET:
        case ENEMY_STATE_REACT_TO_PLAYER_SEEN:
        case ENEMY_STATE_REACT_TO_BEING_SHOT_AT:
        case ENEMY_STATE_REACT_TO_GUN_SHOT:
        case ENEMY_STATE_LOOK_FOR_PLAYER:
        {
            // Fast turning
            speed_multiplier = 1.5f;
        } break;
        case ENEMY_STATE_SLEEP:
        {
            // Don't turn when sleeping
            speed_multiplier = 0.0f;
        } break;
        case ENEMY_STATE_LOOK_AROUND:
        case ENEMY_STATE_WANDER_AROUND:
        {
            // Chilling speed
            speed_multiplier = 0.5f;
        } break;
        default:
        {
            // Normal turn speed
            speed_multiplier = 1.0f;
        } break;
    };

    // One full turn per second * multiplier
    result = F64_PI * 2.0f * speed_multiplier;

    return result;
}

f32 enemy_look_around_delay_get(struct game_state* state, struct enemy* enemy)
{
    f32 result = f32_random_number_get(state, ENEMY_LOOK_AROUND_DELAY_MIN,
        ENEMY_LOOK_AROUND_DELAY_MAX);

    if (enemy->state == ENEMY_STATE_LOOK_FOR_PLAYER)
    {
        result *= 0.5f;
    }

    return result;
}

void enemy_state_transition(struct game_state* state, struct enemy* enemy,
    u32 state_new)
{
    if (state_new == enemy->state)
    {
        return;
    }

    u32 state_old = enemy->state;

    enemy->state_timer = 0.0f;
    enemy->path_length = 0;
    enemy->state = state_new;

    struct level* level = &state->level;

    // LOG("Enemy transition from %s to %s\n", enemy_state_str[state_old],
    //     enemy_state_str[state_new]);

    switch (state_new)
    {
        case ENEMY_STATE_REACT_TO_PLAYER_SEEN:
        {
            enemy->acceleration = 0.0f;
            enemy->state_timer = enemy_reaction_time_get(state, state_old);
        } break;
        case ENEMY_STATE_REACT_TO_GUN_SHOT:
        {
            enemy->acceleration = 0.0f;
            enemy->state_timer = enemy_reaction_time_get(state, state_old);
            enemy_look_towards_position(enemy, enemy->gun_shot_position);
        } break;
        case ENEMY_STATE_RUSH_TO_TARGET:
        {
            enemy->acceleration = ENEMY_ACCELERATION;
            enemy_calculate_path_to_target(state, level, enemy);
        } break;
        case ENEMY_STATE_WANDER_AROUND:
        {
            enemy->acceleration = ENEMY_ACCELERATION * 0.5f;
            enemy->target = tile_random_get(state, level, TILE_FLOOR);
            enemy_calculate_path_to_target(state, level, enemy);
        } break;
        case ENEMY_STATE_LOOK_AROUND:
        {
            enemy->acceleration = 0.0f;
            enemy->turns_left = 3;
        } break;
        case ENEMY_STATE_REACT_TO_BEING_SHOT_AT:
        {
            enemy_look_towards_direction(enemy, enemy->hit_direction);
            enemy->state_timer = enemy_reaction_time_get(state, state_old);
        } break;
        case ENEMY_STATE_LOOK_FOR_PLAYER:
        {
            enemy->acceleration = 0.0f;
            enemy->turns_left = 5;
        } break;
    }
}

void enemies_update(struct game_state* state, struct game_input* input, f32 dt)
{
    // Todo: clean this function...
    for (u32 i = 0; i < MAX_ENEMIES; i++)
    {
        struct enemy* enemy = &state->enemies[i];

        if (enemy->alive)
        {
            if (enemy->health < 0.0f)
            {
                enemy->alive = false;

                {
                    u32 count = 0;

                    for (u32 i = 0; i < state->num_enemies; i++)
                    {
                        count += state->enemies[i].alive;
                    }

                    if (count)
                    {
                        LOG("Enemies left %u of %u\n", count,
                            state->num_enemies);
                    }
                    else
                    {
                        state->level_cleared = true;
                        state->level_clear_notify = 2.5f;
                    }
                }

                item_create(state, enemy->body.position,
                    ITEM_HEALTH + enemy->weapon.type);

                u32 random_item_count = u32_random_number_get(state, 0, 3);

                for (u32 j = 0; j < random_item_count; j++)
                {
                    struct v2 position = enemy->body.position;

                    f32 offset_max = 0.25f;
                    f32 offset_x = f32_random_number_get(state, -offset_max,
                        offset_max);
                    f32 offset_y = f32_random_number_get(state, -offset_max,
                        offset_max);

                    position.x += offset_x;
                    position.y += offset_y;

                    u32 random_item_type = u32_random_number_get(state, 0, 2);
                    random_item_type =
                        random_item_type ? ITEM_HEALTH : ITEM_WEAPON_LEVEL_UP;

                    item_create(state, position, random_item_type);
                }

                item_create(state, enemy->body.position,
                    ITEM_HEALTH + enemy->weapon.type);

                continue;
            }

            enemy->player_in_view = enemy_sees_player(state, enemy,
                &state->player);

            enemy->gun_shot_heard = enemy_hears_gun_shot(state, enemy);

            if (enemy->player_in_view)
            {
                enemy->player_last_seen_position = state->player.body.position;
                enemy->player_last_seen_direction = v2_normalize(
                    state->player.body.velocity);

                if (enemy->state != ENEMY_STATE_SHOOT &&
                    enemy->state != ENEMY_STATE_SLEEP)
                {
                    enemy_state_transition(state, enemy,
                        ENEMY_STATE_REACT_TO_PLAYER_SEEN);
                }
            }
            else if (enemy->gun_shot_heard)
            {
                // Todo: enemy should react only to the closest source of noise
                if (enemy->state == ENEMY_STATE_SLEEP)
                {
                    enemy_state_transition(state, enemy,
                        ENEMY_STATE_LOOK_FOR_PLAYER);
                }
                else if (enemy->state == ENEMY_STATE_RUSH_TO_TARGET)
                {
                    enemy->target = enemy->gun_shot_position;
                    enemy_calculate_path_to_target(state, &state->level, enemy);
                }
                else if (enemy->state != ENEMY_STATE_SHOOT)
                {
                    enemy_state_transition(state, enemy,
                        ENEMY_STATE_REACT_TO_GUN_SHOT);
                }
            }
            else if (enemy->got_hit)
            {
                // Todo: react only if the source of bullet is not visible?
                if (enemy->state == ENEMY_STATE_SLEEP)
                {
                    enemy_state_transition(state, enemy,
                        ENEMY_STATE_LOOK_FOR_PLAYER);
                }
                else if (enemy->state != ENEMY_STATE_SHOOT)
                {
                    enemy_state_transition(state, enemy,
                        ENEMY_STATE_REACT_TO_BEING_SHOT_AT);
                }

                enemy->got_hit = false;
            }

            switch (enemy->state)
            {
                case ENEMY_STATE_SLEEP:
                {
                } break;
                case ENEMY_STATE_REACT_TO_PLAYER_SEEN:
                {
                    if (enemy->state_timer_finished)
                    {
                        if (enemy->player_in_view)
                        {
                            enemy_state_transition(state, enemy,
                                ENEMY_STATE_SHOOT);
                        }
                        else
                        {
                            enemy->target = enemy->player_last_seen_position;
                            enemy_state_transition(state, enemy,
                                ENEMY_STATE_RUSH_TO_TARGET);
                        }
                    }
                    else if (enemy->player_in_view)
                    {
                        enemy_look_towards_position(enemy,
                            state->player.body.position);
                    }
                } break;
                case ENEMY_STATE_REACT_TO_GUN_SHOT:
                {
                    if (enemy->state_timer_finished)
                    {
                        if (ray_cast_position(state, enemy->eye_position,
                            enemy->gun_shot_position, NULL,
                            COLLISION_STATIC | COLLISION_PLAYER))
                        {
                            enemy_state_transition(state, enemy,
                                ENEMY_STATE_LOOK_FOR_PLAYER);
                        }
                        else
                        {
                            enemy->target = enemy->gun_shot_position;
                            enemy_state_transition(state, enemy,
                                ENEMY_STATE_RUSH_TO_TARGET);

                            // Here we can skip the last node since we only need
                            // to see the location where gun shot occured
                            if (enemy->path_length > 0)
                            {
                                enemy->path_length--;
                            }
                        }
                    }
                } break;
                case ENEMY_STATE_SHOOT:
                {
                    enemy->acceleration = 0.0f;

                    if (enemy->player_in_view)
                    {
                        enemy->player_last_seen_position =
                            state->player.body.position;
                        enemy->player_last_seen_direction = v2_normalize(
                            state->player.body.velocity);

                        struct v2 target_forward = v2_direction(
                            enemy->body.position,
                            enemy->player_last_seen_position);

                        if (v2_length(enemy->body.velocity))
                        {
                            // Todo: calculate right with cross product
                            f32 angle = F64_PI*1.5f;
                            f32 tsin = f32_sin(angle);
                            f32 tcos = f32_cos(angle);

                            struct v2 target_right =
                            {
                                target_forward.x * tcos - target_forward.y *
                                    tsin,
                                target_forward.x * tsin + target_forward.y *
                                    tcos
                            };

                            f32 initial_velocity = v2_dot(enemy->body.velocity,
                                target_right);

                            struct v2 desired_velocity = { 0.0f };

                            if (PROJECTILE_SPEED > initial_velocity)
                            {
                                desired_velocity.x = -initial_velocity;
                            }
                            else
                            {
                                desired_velocity.x = PROJECTILE_SPEED;
                            }

                            if (desired_velocity.x)
                            {
                                desired_velocity.y = f32_sqrt(
                                    (PROJECTILE_SPEED * PROJECTILE_SPEED) -
                                    (desired_velocity.x * desired_velocity.x));
                            }
                            else
                            {
                                desired_velocity.y = -PROJECTILE_SPEED;
                            }

                            struct v2 target_right_rotate_back =
                            {
                                target_right.x, -target_right.y
                            };

                            struct v2 target_forward_rotate_back =
                            {
                                -target_forward.x, target_forward.y
                            };

                            struct v2 final_direction =
                            {
                                v2_dot(desired_velocity,
                                    target_right_rotate_back),
                                v2_dot(desired_velocity,
                                    target_forward_rotate_back)
                            };

                            enemy->direction_aim =
                                v2_normalize(final_direction);
                        }
                        else
                        {
                            enemy->direction_aim = target_forward;
                        }

                        enemy_look_towards_direction(enemy, target_forward);

                        struct weapon* weapon = &enemy->weapon;

                        weapon->direction = enemy->direction_aim;
                        weapon->position = enemy->eye_position;
                        weapon->velocity = enemy->body.velocity;

                        weapon_shoot(state, weapon, false);

                        if (weapon->fired && enemy->trigger_release <= 0.0f)
                        {
                            enemy->trigger_release = 0.5f;
                        }
                    }
                    else if (!state->player.alive)
                    {
                        enemy_state_transition(state, enemy,
                            ENEMY_STATE_LOOK_FOR_PLAYER);
                    }
                    else
                    {
                        enemy_state_transition(state, enemy,
                            ENEMY_STATE_REACT_TO_PLAYER_SEEN);
                    }
                } break;
                case ENEMY_STATE_RUSH_TO_TARGET:
                {
                    if (!enemy->path_length)
                    {
                        enemy_look_towards_position(enemy, enemy->target);
                        enemy_state_transition(state, enemy,
                            ENEMY_STATE_LOOK_FOR_PLAYER);
                    }
                } break;
                case ENEMY_STATE_LOOK_AROUND:
                {
                    if (enemy->state_timer_finished)
                    {
                        if (enemy->turns_left)
                        {
                            f32 diff = f32_random_number_get(state, -F64_PI,
                                F64_PI);
                            f32 angle_new = enemy->body.angle + diff;

                            enemy_look_towards_angle(enemy, angle_new);

                            enemy->turns_left--;
                        }
                        else
                        {
                            enemy_state_transition(state, enemy,
                                ENEMY_STATE_WANDER_AROUND);
                        }
                    }
                    else if (enemy->state_timer <= 0.0f && !enemy->turn_amount)
                    {
                        enemy->state_timer =
                            enemy_look_around_delay_get(state, enemy);
                    }
                } break;
                case ENEMY_STATE_WANDER_AROUND:
                {
                    if (!enemy->path_length)
                    {
                        enemy_state_transition(state, enemy,
                            ENEMY_STATE_LOOK_AROUND);
                    }
                } break;
                case ENEMY_STATE_REACT_TO_BEING_SHOT_AT:
                {
                    if (enemy->state_timer_finished)
                    {
                        f32 length = ray_cast_direction(state,
                            enemy->eye_position, enemy->hit_direction, NULL,
                            COLLISION_STATIC);

                        // Skip one tile so we don't find a path into a wall
                        length = MAX(0, length - 1.0f);

                        enemy->target = v2_add(enemy->eye_position,
                            v2_mul_f32(enemy->hit_direction, length));

                        enemy_state_transition(state, enemy,
                            ENEMY_STATE_RUSH_TO_TARGET);

                        enemy->hit_direction = v2_zero;
                    }
                } break;
                case ENEMY_STATE_LOOK_FOR_PLAYER:
                {
                    if (!v2_is_zero(enemy->player_last_seen_direction))
                    {
                        enemy_look_towards_direction(enemy,
                                enemy->player_last_seen_direction);
                        enemy->player_last_seen_direction = v2_zero;
                        enemy->state_timer =
                            enemy_look_around_delay_get(state, enemy);
                    }
                    else
                    {
                        if (enemy->state_timer_finished)
                        {
                            if (enemy->turns_left)
                            {
                                f32 diff = f32_random_number_get(state, -F64_PI,
                                    F64_PI);
                                f32 angle_new = enemy->body.angle + diff;

                                enemy_look_towards_angle(enemy, angle_new);

                                enemy->turns_left--;
                            }
                            else
                            {
                                enemy_state_transition(state, enemy,
                                    ENEMY_STATE_WANDER_AROUND);
                            }
                        }
                        else if (enemy->state_timer <= 0.0f &&
                            !enemy->turn_amount)
                        {
                            enemy->state_timer =
                                enemy_look_around_delay_get(state, enemy);
                        }
                    }
                } break;
            }

            enemy->state_timer_finished = false;

            if (enemy->state_timer > 0.0f)
            {
                enemy->state_timer -= dt;

                if (enemy->state_timer < 0.0f)
                {
                    enemy->state_timer_finished = true;
                }
            }

            if (enemy->path_length)
            {
                struct v2 current = enemy->path[enemy->path_index];

                f32 distance_to_current = v2_distance(enemy->body.position,
                    current);
                f32 epsilon = 0.25f;

                if (distance_to_current < epsilon)
                {
                    if (++enemy->path_index >= enemy->path_length)
                    {
                        enemy->path_length = 0;
                        enemy->acceleration = 0.0f;
                    }
                    else
                    {
                        enemy_look_towards_position(enemy,
                            enemy->path[enemy->path_index]);
                    }
                }
                else
                {
                    struct v2 direction = v2_normalize(v2_direction(
                        enemy->body.position, current));

                    f32 length = v2_length(direction);

                    if (length > 1.0f)
                    {
                        direction.x /= length;
                        direction.y /= length;
                    }

                    enemy->direction_move = direction;
                    enemy->direction_aim = direction;
                }
            }

            struct v2 acceleration = { 0.0f };
            struct v2 move_delta = { 0.0f };

            acceleration.x = enemy->direction_move.x * enemy->acceleration;
            acceleration.y = enemy->direction_move.y * enemy->acceleration;

            acceleration.x += -enemy->body.velocity.x * FRICTION;
            acceleration.y += -enemy->body.velocity.y * FRICTION;

            move_delta.x = 0.5f * acceleration.x * f32_square(dt) +
                enemy->body.velocity.x * dt;
            move_delta.y = 0.5f * acceleration.y * f32_square(dt) +
                enemy->body.velocity.y * dt;

            enemy->body.velocity.x = enemy->body.velocity.x + acceleration.x
                * dt;
            enemy->body.velocity.y = enemy->body.velocity.y + acceleration.y
                * dt;

            check_tile_collisions(&state->level, &enemy->body.position,
                &enemy->body.velocity,
                move_delta, PLAYER_RADIUS, 1);

            if (enemy->turn_amount)
            {
                f32 speed = enemy_turn_speed_get(enemy) * dt;
                f32 remaining = MIN(f32_abs(enemy->turn_amount), speed);

                if (enemy->turn_amount > 0)
                {
                    enemy->body.angle -= remaining;
                    enemy->turn_amount = MAX(enemy->turn_amount - remaining, 0);
                }
                else
                {
                    enemy->body.angle += remaining;
                    enemy->turn_amount = MIN(enemy->turn_amount + remaining, 0);
                }
            }

            struct v2 eye = { PLAYER_RADIUS + 0.0001f, 0.0f };

            enemy->eye_position = v2_rotate(eye, enemy->body.angle);
            enemy->eye_position.x += enemy->body.position.x;
            enemy->eye_position.y += enemy->body.position.y;

            {
                f32 vision_cone_update_speed = 3.0f;
                f32 vision_cone_size_min = 0.25f;
                f32 vision_cone_size_max = 1.0f;

                if (enemy->player_in_view)
                {
                    enemy->vision_cone_size -= dt * vision_cone_update_speed;
                    enemy->vision_cone_size = MAX(enemy->vision_cone_size,
                        vision_cone_size_min);
                }
                else
                {
                    enemy->vision_cone_size += dt * vision_cone_update_speed;
                    enemy->vision_cone_size = MIN(enemy->vision_cone_size,
                        vision_cone_size_max);
                }
            }

            // Todo: clean this, maybe move somewhere else
            {
                struct weapon* weapon = &enemy->weapon;

                if (enemy->trigger_release > 0.0f)
                {
                    enemy->trigger_release -= dt;

                    if (enemy->trigger_release <= 0.0f)
                    {
                        weapon->fired = false;
                    }
                }

                if (weapon->last_shot > 0.0f)
                {
                    weapon->last_shot -= dt;
                }

                if (weapon->reloading)
                {
                    if (weapon->last_shot <= 0.0f)
                    {
                        weapon->ammo++;

                        if (weapon->ammo == weapon->ammo_max)
                        {
                            weapon->reloading = false;
                        }
                        else
                        {
                            weapon->last_shot = weapon->reload_time;
                        }
                    }
                }
            }
        }
    }
}

b32 insert_corner(struct v2 corner, struct v2 corners[], u32 max, u32* count)
{
    b32 result = false;

    if (*count < max)
    {
        b32 found = false;

        for (u32 i = 0; i < *count; i++)
        {
            if (v2_equals(corner, corners[i]))
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            corners[(*count)++] = corner;
        }

        result = true;
    }

    return result;
}

void get_wall_corners(struct level* level, struct v2 corners[], u32 max,
    u32* count)
{
    for (u32 y = 0; y < level->height; y++)
    {
        for (u32 x = 0; x < level->width; x++)
        {
            struct v2 tile = { x, y };

            if (tile_is_of_type(level, tile, TILE_WALL))
            {
                f32 t = WALL_SIZE * 0.5f;

                if (!insert_corner((struct v2){ tile.x + t, tile.y + t},
                    corners, max, count))
                {
                    return;
                }

                if (!insert_corner((struct v2){ tile.x - t, tile.y - t},
                    corners, max, count))
                {
                    return;
                }

                if (!insert_corner((struct v2){ tile.x + t, tile.y - t},
                    corners, max, count))
                {
                    return;
                }

                if (!insert_corner((struct v2){ tile.x - t, tile.y + t},
                    corners, max, count))
                {
                    return;
                }
            }
        }
    }
}

b32 line_segment_equals(struct line_segment a, struct line_segment b)
{
    b32 result = v2_equals(a.start, b.start) && v2_equals(a.end, b.end);

    return result;
}

b32 line_segment_empty(struct line_segment a)
{
    b32 result = a.start.x == 0.0f && a.start.y == 0.0f && a.end.x == 0.0f &&
        a.end.y == 0.0f;

    return result;
}

b32 insert_face(struct line_segment* face, struct line_segment faces[], u32 max,
    u32* count)
{
    b32 result = false;

    if (line_segment_empty(*face))
    {
        result = true;
    }
    else if (*count < max)
    {
        b32 found = false;

        for (u32 i = 0; i < *count; i++)
        {
            if (line_segment_equals(*face, faces[i]))
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            faces[(*count)++] = *face;
            *face = (struct line_segment) { 0.0f };
        }

        result = true;
    }

    return result;
}

void collision_map_static_calculate(struct level* level,
    struct line_segment faces[], u32 max, u32* count)
{
    for (u32 y = 0; y < level->height; y++)
    {
        struct line_segment face_top    = { 0.0f };
        struct line_segment face_bottom = { 0.0f };

        for (u32 x = 0; x <= level->width; x++)
        {
            struct v2 tile = { x, y };
            face_top.type = COLLISION_STATIC;
            face_bottom.type = COLLISION_STATIC;

            if (tile_is_of_type(level, tile, TILE_WALL))
            {
                f32 t = WALL_SIZE * 0.5f;

                struct v2 tile_top    = { tile.x, tile.y + WALL_SIZE };
                struct v2 tile_bottom = { tile.x, tile.y - WALL_SIZE };

                if (tile_inside_level_bounds(level, tile_bottom) &&
                    tile_is_free(level, tile_bottom))
                {
                    if (line_segment_empty(face_bottom))
                    {
                        face_bottom.start =
                            (struct v2){ tile.x - t, tile.y - t };
                    }

                    face_bottom.end = (struct v2){ tile.x + t, tile.y - t };
                }
                else if (!insert_face(&face_bottom, faces, max, count))
                {
                    return;
                }

                if (tile_inside_level_bounds(level, tile_top) &&
                    tile_is_free(level, tile_top))
                {
                    if (line_segment_empty(face_top))
                    {
                        face_top.start = (struct v2){ tile.x - t, tile.y + t };
                    }

                    face_top.end = (struct v2){ tile.x + t, tile.y + t };
                }
                else if (!insert_face(&face_top, faces, max, count))
                {
                    return;
                }
            }
            else
            {
                if (!insert_face(&face_top, faces, max, count))
                {
                    return;
                }

                if (!insert_face(&face_bottom, faces, max, count))
                {
                    return;
                }
            }
        }
    }

    for (u32 x = 0; x < level->width; x++)
    {
        struct line_segment face_left  = { 0.0f };
        struct line_segment face_right = { 0.0f };

        for (u32 y = 0; y < level->height; y++)
        {
            struct v2 tile = { x, y };
            face_left.type = COLLISION_STATIC;
            face_right.type = COLLISION_STATIC;

            if (tile_is_of_type(level, tile, TILE_WALL))
            {
                f32 t = WALL_SIZE * 0.5f;

                struct v2 tile_left  = { tile.x - WALL_SIZE, tile.y };
                struct v2 tile_right = { tile.x + WALL_SIZE, tile.y };

                if (tile_inside_level_bounds(level, tile_left) &&
                    tile_is_free(level, tile_left))
                {
                    if (line_segment_empty(face_left))
                    {
                        face_left.start =
                            (struct v2){ tile.x - t, tile.y - t };
                    }

                    face_left.end = (struct v2){ tile.x - t, tile.y + t };
                }
                else if (!insert_face(&face_left, faces, max, count))
                {
                    return;
                }

                if (tile_inside_level_bounds(level, tile_right) &&
                    tile_is_free(level, tile_right))
                {
                    if (line_segment_empty(face_right))
                    {
                        face_right.start =
                            (struct v2){ tile.x + t, tile.y - t };
                    }

                    face_right.end = (struct v2){ tile.x + t, tile.y + t };
                }
                else if (!insert_face(&face_right, faces, max, count))
                {
                    return;
                }
            }
            else
            {
                if (!insert_face(&face_right, faces, max, count))
                {
                    return;
                }

                if (!insert_face(&face_left, faces, max, count))
                {
                    return;
                }
            }
        }
    }
}

void collision_map_dynamic_calculate(struct game_state* state)
{
    state->num_cols_dynamic = 0;
    struct line_segment segments[4] = { 0 };

    if (state->player.alive)
    {
        get_body_rectangle(state->player.body, PLAYER_RADIUS, PLAYER_RADIUS,
            segments);

        for (u32 i = 0;
            i < 4 && state->num_cols_static < MAX_COLLISION_SEGMENTS; i++)
        {
            segments[i].type = COLLISION_PLAYER;
            state->cols_dynamic[state->num_cols_dynamic++] = segments[i];
        }
    }

    for (u32 i = 0; i < state->num_enemies; i++)
    {
        struct enemy* enemy = &state->enemies[i];

        if (enemy->alive)
        {
            get_body_rectangle(enemy->body, PLAYER_RADIUS, PLAYER_RADIUS,
                segments);

            for (u32 i = 0;
                i < 4 && state->num_cols_dynamic < MAX_COLLISION_SEGMENTS; i++)
            {
                segments[i].type = COLLISION_ENEMY;
                state->cols_dynamic[state->num_cols_dynamic++] = segments[i];
            }
        }
    }
}

void get_wall_corners_from_faces(struct v2 corners[], u32 max, u32* count,
    struct line_segment faces[], u32 num_faces)
{
    for (u32 i = 0; i < num_faces; i++)
    {
        if (!insert_corner(faces[i].start, corners, max, count))
        {
            return;
        }

        if (!insert_corner(faces[i].end, corners, max, count))
        {
            return;
        }
    }
}

b32 direction_in_range(struct v2 dir, struct v2 left, struct v2 right)
{
    b32 result = v2_cross(left, dir) < 0.0f && v2_cross(right, dir) > 0.0f;

    return result;
}

void reorder_corners_ccw(struct v2* corners, u32 count, struct v2 position)
{
    for (u32 i = 0; i < count - 1; i++)
    {
        for (u32 j = i + 1; j < count; j++)
        {
            if (f32_triangle_area_signed(position, corners[i], corners[j]) < 0)
            {
                v2_swap(&corners[i], &corners[j]);
            }
        }
    }
}

void line_of_sight_render(struct game_state* state, struct v2 position,
    f32 angle_start, f32 angle_max, struct v4 color, b32 render_lines)
{
    struct v2 corners[MAX_WALL_CORNERS] = { 0 };
    u32 num_corners = 0;

    struct v2 collision = { 0 };
    struct v2 dir_left = v2_direction_from_angle(angle_start + angle_max);
    struct v2 dir_right = v2_direction_from_angle(angle_start - angle_max);

    ray_cast_direction(state, position, dir_right, &collision, COLLISION_ALL);
    corners[num_corners++] = collision;

    ray_cast_direction(state, position, dir_left, &collision, COLLISION_ALL);
    corners[num_corners++] = collision;

    for (u32 i = 0; i < state->num_cols_dynamic &&
        num_corners < MAX_WALL_CORNERS; i++)
    {
        struct v2 temp = state->cols_dynamic[i].start;

        if (direction_in_range(v2_direction(position, temp), dir_left,
            dir_right))
        {
            corners[num_corners++] = temp;
        }
    }

    for (u32 i = 0; i < state->num_wall_corners &&
        num_corners < MAX_WALL_CORNERS; i++)
    {
        struct v2 temp = state->wall_corners[i];

        if (direction_in_range(v2_direction(position, temp), dir_left,
            dir_right))
        {
            corners[num_corners++] = temp;
        }
    }

    struct v2 finals[MAX_WALL_CORNERS] = { 0 };
    u32 num_finals = 0;

    for (u32 i = 0; i < num_corners; i++)
    {
        struct v2 direction = v2_direction(position, corners[i]);
        f32 angle = f32_atan(direction.y, direction.x);
        f32 t = 0.00001f;

        ray_cast_direction(state, position, direction, &collision,
            COLLISION_ALL);

        struct v2 collision_temp = { 0 };
        finals[num_finals++] = collision;

        if (render_lines)
        {
            line_render(state, position, collision, colors[RED],
                0.005f, 0.005f);
        }

        ray_cast_direction(state, position, v2_direction_from_angle(angle + t),
            &collision_temp, COLLISION_ALL);

        if (v2_distance(collision_temp, collision) > 0.001f)
        {
            if (render_lines)
            {
                line_render(state, position, collision_temp,
                    colors[RED], 0.005f, 0.005f);
            }

            finals[num_finals++] = collision_temp;
        }

        ray_cast_direction(state, position, v2_direction_from_angle(angle - t),
            &collision_temp, COLLISION_ALL);

        if (v2_distance(collision_temp, collision) > 0.001f)
        {
            if (render_lines)
            {
                line_render(state, position, collision_temp,
                    colors[RED], 0.005f, 0.005f);
            }

            finals[num_finals++] = collision_temp;
        }
    }

    reorder_corners_ccw(finals, num_finals, position);

    for (u32 i = 0; i < num_finals-1; i++)
    {
        triangle_render(state, position, finals[i], finals[i+1], color,
            0.0025f);
    }
}

void enemies_render(struct game_state* state)
{
    for (u32 i = 0; i < MAX_ENEMIES; i++)
    {
        struct enemy* enemy = &state->enemies[i];

        if (enemy->alive)
        {
            switch (enemy->state)
            {
                case ENEMY_STATE_SHOOT:
                {
                    enemy->cube.faces[0].texture = 15;
                } break;
                case ENEMY_STATE_WANDER_AROUND:
                case ENEMY_STATE_LOOK_AROUND:
                {
                    enemy->cube.faces[0].texture = 13;
                } break;
                case ENEMY_STATE_SLEEP:
                {
                    enemy->cube.faces[0].texture = 12;
                } break;
                case ENEMY_STATE_RUSH_TO_TARGET:
                case ENEMY_STATE_REACT_TO_PLAYER_SEEN:
                case ENEMY_STATE_REACT_TO_GUN_SHOT:
                case ENEMY_STATE_REACT_TO_BEING_SHOT_AT:
                case ENEMY_STATE_LOOK_FOR_PLAYER:
                default:
                {
                    enemy->cube.faces[0].texture = 14;
                } break;
            }

            struct m4 transform = m4_translate(enemy->body.position.x,
                enemy->body.position.y, PLAYER_RADIUS);
            struct m4 rotation = m4_rotate_z(enemy->body.angle);
            struct m4 scale = m4_scale_xyz(PLAYER_RADIUS, PLAYER_RADIUS, 0.25f);
            struct m4 model = m4_mul_m4(scale, rotation);
            model = m4_mul_m4(model, transform);

            enemy->cube.model = model;

            cube_renderer_add(&state->cube_renderer, &enemy->cube);

            if (state->render_debug)
            {
                line_of_sight_render(state, enemy->eye_position,
                    enemy->body.angle, enemy->vision_cone_size *
                    ENEMY_LINE_OF_SIGHT_HALF,
                    enemy->player_in_view ? colors[PURPLE] : colors[TEAL],
                    true);
            }

            health_bar_render(state, enemy->body.position, enemy->health,
                ENEMY_HEALTH_MAX);
            ammo_bar_render(state, enemy->body.position, enemy->weapon.ammo,
                enemy->weapon.ammo_max);

            // Render velocity vector
            if (state->render_debug)
            {
                f32 max_speed = 7.0f;
                f32 length = v2_length(enemy->body.velocity) / max_speed;
                f32 angle = f32_atan(enemy->body.velocity.x,
                    enemy->body.velocity.y);

                transform = m4_translate(
                    enemy->body.position.x + enemy->body.velocity.x / max_speed,
                    enemy->body.position.y + enemy->body.velocity.y / max_speed,
                    0.012f);

                rotation = m4_rotate_z(-angle);
                scale = m4_scale_xyz(0.05f, length, 0.01f);

                model = m4_mul_m4(scale, rotation);
                model = m4_mul_m4(model, transform);

                struct m4 mvp = m4_mul_m4(model, state->camera.view);
                mvp = m4_mul_m4(mvp, state->camera.projection);

                mesh_render(&state->floor, &mvp, state->texture_tileset,
                    state->shader_simple, colors[GREY]);
            }

            // Render aim vector
            if (state->render_debug)
            {
                f32 length = 1.0f;
                f32 angle = f32_atan(enemy->direction_aim.x,
                    enemy->direction_aim.y);

                transform = m4_translate(
                    enemy->body.position.x + enemy->direction_aim.x / length,
                    enemy->body.position.y + enemy->direction_aim.y / length,
                    0.011f);

                rotation = m4_rotate_z(-angle);
                scale = m4_scale_xyz(0.025f, length, 0.01f);

                model = m4_mul_m4(scale, rotation);
                model = m4_mul_m4(model, transform);

                struct m4 mvp = m4_mul_m4(model, state->camera.view);
                mvp = m4_mul_m4(mvp, state->camera.projection);

                mesh_render(&state->floor, &mvp, state->texture_tileset,
                    state->shader_simple, colors[YELLOW]);
            }

            // Render look vector
            if (state->render_debug)
            {
                f32 length = 1.0f;
                f32 angle = f32_atan(enemy->direction_look.x,
                    enemy->direction_look.y);

                transform = m4_translate(
                    enemy->body.position.x + enemy->direction_look.x / length,
                    enemy->body.position.y + enemy->direction_look.y / length,
                    0.010f);

                rotation = m4_rotate_z(-angle);
                scale = m4_scale_xyz(0.025f, length, 0.01f);

                model = m4_mul_m4(scale, rotation);
                model = m4_mul_m4(model, transform);

                struct m4 mvp = m4_mul_m4(model, state->camera.view);
                mvp = m4_mul_m4(mvp, state->camera.projection);

                mesh_render(&state->floor, &mvp, state->texture_tileset,
                    state->shader_simple, colors[BLUE]);
            }

            // Render path
            if (state->render_debug)
            {
                if (enemy->path_length)
                {
                    struct v2 current = enemy->body.position;

                    for (u32 j = enemy->path_index; j < enemy->path_length; j++)
                    {
                        struct v2 next = enemy->path[j];

                        line_render(state, current, next, colors[i],
                            0.005f, 0.02f);

                        current = next;
                    }
                }
            }
        }
    }
}

void bullets_update(struct game_state* state, struct game_input* input, f32 dt)
{
    for (u32 i = 0; i < MAX_BULLETS; i++)
    {
        struct bullet* bullet = &state->bullets[i];

        if (bullet->alive)
        {
            struct v2 move_delta =
            {
                bullet->body.velocity.x * dt,
                bullet->body.velocity.y * dt
            };

            f32 distance_target = v2_length(move_delta);
            u32 flags = COLLISION_STATIC;

            if (bullet->player_owned)
            {
                flags |= COLLISION_ENEMY;
            }
            else
            {
                flags |= COLLISION_PLAYER;
            }

            f32 distance = ray_cast_direction(state, bullet->body.position,
                v2_normalize(move_delta), NULL, flags);

            f32 fraction = 1.0f;

            if (distance < distance_target)
            {
                fraction = distance / distance_target;
            }

            bullet->body.position.x += move_delta.x * fraction;
            bullet->body.position.y += move_delta.y * fraction;

            if (distance < distance_target)
            {
                // particle_sphere_create(state, bullet->body.position,
                //     (struct v2){ 0.0f, 0.0f }, colors[GREY], PROJECTILE_RADIUS,
                //     PROJECTILE_RADIUS * 5.0f, 0.15f);
                bullet->alive = false;


                particle_emitter_circle(&state->particles, 360,
                    bullet->body.position, 1.0f, 0.05f, colors[RED], 1.0f);
            }

            particle_line_create(state, bullet->start, bullet->body.position,
                colors[GREY], (struct v4){ colors[GREY].r, colors[GREY].g,
                    colors[GREY].b, 0.0f }, 0.30f);

            f32 target_size = PLAYER_RADIUS + PROJECTILE_RADIUS;

            if (bullet->player_owned)
            {
                for (u32 j = 0; j < state->num_enemies; j++)
                {
                    struct enemy* enemy = &state->enemies[j];
                    struct line_segment segments[4] = { 0 };

                    get_body_rectangle(enemy->body, target_size, target_size,
                        segments);

                    struct v2 corners[] =
                    {
                        segments[0].start,
                        segments[1].start,
                        segments[2].start,
                        segments[3].start
                    };

                    if (enemy->alive && collision_point_to_obb(
                        bullet->body.position, corners))
                    {
                        bullet->alive = false;
                        enemy->health -= bullet->damage;
                        enemy->got_hit = true;
                        enemy->hit_direction = v2_flip(v2_normalize(
                            bullet->body.velocity));

                        particle_sphere_create(state, bullet->body.position,
                            (struct v2){ bullet->body.velocity.x * 0.5,
                                bullet->body.velocity.y * 0.5 }, colors[RED],
                            PROJECTILE_RADIUS, PROJECTILE_RADIUS * 2.5f, 0.15f);

                        break;
                    }
                }
            }
            else
            {
                struct player* player = &state->player;
                struct line_segment segments[4] = { 0 };

                get_body_rectangle(player->body, target_size, target_size,
                    segments);

                struct v2 corners[] =
                {
                    segments[0].start,
                    segments[1].start,
                    segments[2].start,
                    segments[3].start
                };

                if (player->alive && collision_point_to_obb(
                    bullet->body.position, corners))
                {
                    bullet->alive = false;
                    player->health -= bullet->damage;

                    particle_sphere_create(state, bullet->body.position,
                        (struct v2){ bullet->body.velocity.x * 0.5,
                            bullet->body.velocity.y * 0.5 }, colors[RED],
                        PROJECTILE_RADIUS, PROJECTILE_RADIUS * 2.5f, 0.15f);
                }
            }

            bullet->start = bullet->body.position;
        }
    }
}

void bullets_render(struct game_state* state)
{
    for (u32 i = 0; i < MAX_BULLETS; i++)
    {
        struct bullet* bullet = &state->bullets[i];

        if (bullet->alive)
        {
            struct m4 transform = m4_translate(bullet->body.position.x,
                bullet->body.position.y, PLAYER_RADIUS);
            struct m4 rotation = m4_rotate_z(bullet->body.angle);
            struct m4 scale = m4_scale_all(PROJECTILE_RADIUS);

            struct m4 model = m4_mul_m4(scale, rotation);
            model = m4_mul_m4(model, transform);

            struct m4 mvp = m4_mul_m4(model, state->camera.view);
            mvp = m4_mul_m4(mvp, state->camera.projection);

            mesh_render(&state->sphere, &mvp, state->texture_sphere,
                state->shader, colors[WHITE]);
        }
    }
}

void player_update(struct game_state* state, struct game_input* input, f32 dt)
{
    struct player* player = &state->player;

    player->alive = player->health > 0.0f;

    if (player->alive)
    {
        struct v2 direction = { 0.0f };
        struct v2 acceleration = { 0.0f };
        struct v2 move_delta = { 0.0f };

        struct v2 dir = { state->mouse.world.x - player->body.position.x,
            (state->mouse.world.y - PLAYER_RADIUS) - player->body.position.y };
        dir = v2_normalize(dir);

        player->body.angle = f32_atan(dir.y, dir.x);
        // player->body.angle = f32_radians(-90);

        if (input->move_left.key_down)
        {
            direction.x -= 1.0f;
        }

        if (input->move_right.key_down)
        {
            direction.x += 1.0f;
        }

        if (input->move_down.key_down)
        {
            direction.y -= 1.0f;
        }

        if (input->move_up.key_down)
        {
            direction.y += 1.0f;
        }

        f32 length = v2_length(direction);

        if (length > 1.0f)
        {
            direction.x /= length;
            direction.y /= length;
        }

        acceleration.x = direction.x * PLAYER_ACCELERATION;
        acceleration.y = direction.y * PLAYER_ACCELERATION;

        acceleration.x += -player->body.velocity.x * FRICTION;
        acceleration.y += -player->body.velocity.y * FRICTION;

        move_delta.x = 0.5f * acceleration.x * f32_square(dt) +
            player->body.velocity.x * dt;
        move_delta.y = 0.5f * acceleration.y * f32_square(dt) +
            player->body.velocity.y * dt;

        player->body.velocity.x = player->body.velocity.x + acceleration.x * dt;
        player->body.velocity.y = player->body.velocity.y + acceleration.y * dt;

        check_tile_collisions(&state->level, &player->body.position,
            &player->body.velocity, move_delta, PLAYER_RADIUS, 1);

        struct v2 eye = { PLAYER_RADIUS + 0.0001f, 0.0f };

        player->eye_position = v2_rotate(eye, player->body.angle);
        player->eye_position.x += player->body.position.x;
        player->eye_position.y += player->body.position.y;

        struct weapon* weapon = &player->weapon;

        if (weapon->last_shot > 0.0f)
        {
            weapon->last_shot -= dt;
        }

        if (player->item_picked)
        {
            switch (player->item_picked)
            {
                case ITEM_HEALTH:
                {
                    player->health += ITEAM_HEALTH_AMOUNT;
                    player->health = MIN(player->health, PLAYER_HEALTH_MAX);
                } break;
                case ITEM_PISTOL:
                case ITEM_MACHINEGUN:
                case ITEM_SHOTGUN:
                {
                    u32 weapon_type = player->item_picked - 1;

                    if (weapon->type != weapon_type)
                    {
                        *weapon = weapon_create(player->item_picked - 1);
                        weapon->ammo = 0;
                        weapon_reload(weapon, true);
                    }

                } break;
                case ITEM_WEAPON_LEVEL_UP:
                {
                    if (weapon_level_up(weapon))
                    {
                        weapon_reload(weapon, true);
                    }
                }
            }

            player->item_picked = ITEM_NONE;
        }

        weapon->direction = dir;
        weapon->position = player->eye_position;
        weapon->velocity = player->body.velocity;

        if (key_times_pressed(&input->reload))
        {
            weapon_reload(weapon, false);
        }

        if (state->level_cleared)
        {
            if (key_times_pressed(&input->weapon_pick) &&
                tile_type_get(&state->level, player->body.position) ==
                TILE_START)
            {
                LOG("Changing level...\n");
                state->level_change = true;
            }
        }

        if (input->shoot.key_down && !state->player_in_start_room)
        {
            weapon_shoot(state, weapon, true);
        }
        else
        {
            weapon->fired = false;
        }

        if (weapon->reloading)
        {
            if (weapon->last_shot <= 0.0f)
            {
                weapon->ammo++;

                if (weapon->ammo == weapon->ammo_max)
                {
                    weapon->reloading = false;
                }
                else
                {
                    weapon->last_shot = weapon->reload_time;
                }
            }
        }
    }
}

void player_render(struct game_state* state)
{
    struct player* player = &state->player;

    if (player->alive)
    {
        struct m4 transform = m4_translate(player->body.position.x,
            player->body.position.y, PLAYER_RADIUS);
        struct m4 rotation = m4_rotate_z(player->body.angle);
        struct m4 scale = m4_scale_xyz(PLAYER_RADIUS, PLAYER_RADIUS, 0.25f);
        struct m4 model = m4_mul_m4(scale, rotation);
        model = m4_mul_m4(model, transform);

        player->cube.model = model;

        cube_renderer_add(&state->cube_renderer, &player->cube);

        if (state->render_debug)
        {
            struct v4 color = colors[LIME];
            color.a = 0.5f;
            line_of_sight_render(state, player->eye_position,
                player->body.angle, ENEMY_LINE_OF_SIGHT_HALF, color, true);
        }

        // Render velocity vector
        if (state->render_debug)
        {
            f32 max_speed = 7.0f;
            f32 length = v2_length(player->body.velocity) / max_speed;
            f32 angle = f32_atan(player->body.velocity.x,
                player->body.velocity.y);

            transform = m4_translate(
                player->body.position.x + player->body.velocity.x / max_speed,
                player->body.position.y + player->body.velocity.y / max_speed,
                0.01f);

            rotation = m4_rotate_z(-angle);
            scale = m4_scale_xyz(0.05f, length, 0.01f);

            model = m4_mul_m4(scale, rotation);
            model = m4_mul_m4(model, transform);

            struct m4 mvp = m4_mul_m4(model, state->camera.view);
            mvp = m4_mul_m4(mvp, state->camera.projection);

            mesh_render(&state->floor, &mvp, state->texture_tileset,
                state->shader_simple, colors[GREY]);
        }

        // Render aim vector
        if (state->render_debug)
        {
            struct v2 vec =
            {
                state->mouse.world.x - player->body.position.x,
                (state->mouse.world.y - PLAYER_RADIUS) - player->body.position.y
            };

            f32 length = v2_length(vec) * 0.5f;
            f32 angle = f32_atan(vec.x, vec.y);

            struct v2 direction = v2_normalize(vec);

            transform = m4_translate(
                player->body.position.x + direction.x * length,
                player->body.position.y + direction.y * length, PLAYER_RADIUS);
            rotation = m4_rotate_z(-angle);
            scale = m4_scale_xyz(0.01f, length, 0.01f);

            model = m4_mul_m4(scale, rotation);
            model = m4_mul_m4(model, transform);

            struct m4 mvp = m4_mul_m4(model, state->camera.view);
            mvp = m4_mul_m4(mvp, state->camera.projection);

            mesh_render(&state->floor, &mvp, state->texture_tileset,
                state->shader_simple, colors[RED]);
        }

        struct weapon* weapon = &player->weapon;
        health_bar_render(state, player->body.position, player->health,
            PLAYER_HEALTH_MAX);
        ammo_bar_render(state, player->body.position, weapon->ammo,
            weapon->ammo_max);
        weapon_level_bar_render(state, player->body.position, weapon->level,
            WEAPON_LEVEL_MAX);
    }
}

void items_update(struct game_state* state, struct game_input* input, f32 dt)
{
    for (u32 i = 0; i < MAX_ITEMS; i++)
    {
        struct item* item = &state->items[i];

        if (item->alive)
        {
            item->body.angle -= F64_PI * dt;
            item->alive -= dt;

            if (item->alive < 0.0f)
            {
                item->alive = 0.0f;
            }
            else if (item->alive < ITEM_FLASH_TIME)
            {
                if (item->flash_timer <= 0.0f)
                {
                    item->flash_timer = ITEM_FLASH_SPEED;
                    item->flash_hide = !item->flash_hide;
                }
                else
                {
                    item->flash_timer -= dt;
                }
            }

            struct player* player = &state->player;

            if (player->alive && collision_circle_to_circle(item->body.position,
                ITEM_RADIUS, player->body.position, PLAYER_RADIUS))
            {
                if (item->type < ITEM_SHOTGUN || item->type > ITEM_PISTOL ||
                    key_times_pressed(&input->weapon_pick))
                {
                    player->item_picked = item->type;
                    item->alive = false;
                }
            }
        }
    }
}

void items_render(struct game_state* state)
{
    for (u32 i = 0; i < MAX_ITEMS; i++)
    {
        struct item* item = &state->items[i];

        if (item->alive && !item->flash_hide)
        {
            f32 t = f32_sin(item->body.angle) * 0.25f;

            struct m4 transform = m4_translate(item->body.position.x,
                item->body.position.y, ITEM_RADIUS);
            struct m4 rotation = m4_rotate_z(item->body.angle);
            struct m4 scale = m4_scale_all(ITEM_RADIUS + t * ITEM_RADIUS);
            struct m4 model = m4_mul_m4(scale, rotation);

            model = m4_mul_m4(model, transform);

            item->cube.model = model;

            cube_renderer_add(&state->cube_renderer, &item->cube);
        }
    }
}

void particle_lines_update(struct game_state* state, struct game_input* input,
    f32 dt)
{
    for (u32 i = 0; i < MAX_PARTICLES; i++)
    {
        struct particle_line* particle = &state->particle_lines[i];

        if (particle->alive)
        {
            if (particle->time_current <= 0.0f)
            {
                particle->alive = false;
            }
            else
            {
                particle->time_current -= dt;
                f32 t = 1.0f - particle->time_current / particle->time_start;

                particle->color_current = particle->color_start;

                particle->color_current.r = particle->color_start.r +
                    (particle->color_end.r - particle->color_start.r) * t;
                particle->color_current.g = particle->color_start.g +
                    (particle->color_end.g - particle->color_start.g) * t;
                particle->color_current.b = particle->color_start.b +
                    (particle->color_end.b - particle->color_start.b) * t;
                particle->color_current.a = particle->color_start.a +
                    (particle->color_end.a - particle->color_start.a) * t;
            }
        }
    }
}

void particle_lines_render(struct game_state* state)
{
    for (u32 i = 0; i < MAX_PARTICLES; i++)
    {
        struct particle_line* particle = &state->particle_lines[i];

        if (particle->alive)
        {
            line_render(state, particle->start, particle->end,
                particle->color_current, PLAYER_RADIUS, PROJECTILE_RADIUS);
        }
    }
}

void particle_spheres_update(struct game_state* state, struct game_input* input,
    f32 dt)
{
    for (u32 i = 0; i < MAX_PARTICLES; i++)
    {
        struct particle_sphere* particle = &state->particle_spheres[i];

        if (particle->alive)
        {
            if (particle->time_current <= 0.0f)
            {
                particle->alive = false;
            }
            else
            {
                particle->position.x += particle->velocity.x * dt;
                particle->position.y += particle->velocity.y * dt;
                particle->time_current -= dt;
                f32 t = 1.0f - particle->time_current / particle->time_start;

                particle->radius_current = particle->radius_start +
                    (particle->radius_end - particle->radius_start) * t;
            }
        }
    }
}

void particle_spheres_render(struct game_state* state)
{
    for (u32 i = 0; i < MAX_PARTICLES; i++)
    {
        struct particle_sphere* particle = &state->particle_spheres[i];

        if (particle->alive)
        {
            sphere_render(state, particle->position, particle->radius_current,
                particle->color, PLAYER_RADIUS);
        }
    }
}

void particles_render(struct game_state* state,
    struct particle_storage* storage)
{
    for (u32 i = 0; i < storage->num_particles; i++)
    {
        struct particle_data* particle = &storage->data[i];

        if (particle->time > 0)
        {
            triangle_render(state, particle->tl, particle->bl, particle->tr,
                particle->color, 1.0f);
            triangle_render(state, particle->bl, particle->br, particle->tr,
                particle->color, 1.0f);
        }
    }
}

void particles_update(struct particle_storage* storage, f32 dt)
{
    for (u32 i = 0; i < storage->num_particles; i++)
    {
        struct particle_data* particle = &storage->data[i];

        if (particle->time > 0)
        {
            f32 step = MIN(particle->time, dt);

            particle->time -= dt;
            particle->tl.x += particle->velocity.x * step;
            particle->tl.y += particle->velocity.y * step;
            particle->tr.x += particle->velocity.x * step;
            particle->tr.y += particle->velocity.y * step;
            particle->br.x += particle->velocity.x * step;
            particle->br.y += particle->velocity.y * step;
            particle->bl.x += particle->velocity.x * step;
            particle->bl.y += particle->velocity.y * step;
        }
    }
}

// Todo: create single struct for header (requires packing)
struct color_map_spec
{
    s16 index;
    s16 length;
    s8 size;
};

struct image_spec
{
    s16 x;
    s16 y;
    s16 width;
    s16 height;
    s8 depth;
    s8 desc;
};

void tga_decode(s8* input, u64 out_size, s8* output, u32* width, u32* height)
{
    input += 8;
    struct image_spec* i_spec = (struct image_spec*)input;
    input += 10;

    // Todo: read (or skip) image id and color map stuff

    u64 bytes_per_color = i_spec->depth / 8;
    u64 byte_count = i_spec->height * i_spec->width;

    for (u64 i = 0; i < byte_count && i < out_size; i++)
    {
        output[2] = input[0];
        output[1] = input[1];
        output[0] = input[2];
        output[3] = input[3];

        output += bytes_per_color;
        input += bytes_per_color;
    }

    *width = i_spec->width;
    *height = i_spec->height;
}

u32 texture_create(struct memory_block* block, char* path)
{
    u64 read_bytes = 0;
    u32 target = GL_TEXTURE_2D;
    u32 id = 0;
    u32 width = 0;
    u32 height = 0;

    file_handle file;
    u64 file_size = 0;
    s8* file_data = 0;
    s8* pixel_data = 0;

    api.file.file_open(&file, path, true);
    api.file.file_size_get(&file, &file_size);

    file_data = stack_alloc(block, file_size);
    pixel_data = stack_alloc(block, file_size);

    api.file.file_read(&file, file_data, file_size, &read_bytes);
    api.file.file_close(&file);

    tga_decode(file_data, read_bytes, pixel_data, &width, &height);

    api.gl.glGenTextures(1, &id);
    api.gl.glBindTexture(target, id);

    api.gl.glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    api.gl.glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    api.gl.glTexImage2D(target, 0, GL_RGBA, width, height, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, pixel_data);

    stack_free(block);
    stack_free(block);

    gl_check_error("texture_create");

    return id;
}

u32 texture_array_create(struct memory_block* block, char* path, u32 rows,
    u32 cols)
{
    file_handle file;
    u64 read_bytes = 0;
    u64 file_size = 0;
    s8* file_data = 0;
    s8* pixel_data = 0;
    s8* tile_data = 0;

    api.file.file_open(&file, path, true);
    api.file.file_size_get(&file, &file_size);

    file_data = stack_alloc(block, file_size);
    pixel_data = stack_alloc(block, file_size);

    api.file.file_read(&file, file_data, file_size, &read_bytes);
    api.file.file_close(&file);

    u32 image_width = 0;
    u32 image_height = 0;

    tga_decode(file_data, read_bytes, pixel_data, &image_width, &image_height);

    u32 tile_width = image_width / cols;
    u32 tile_height = image_height / rows;
    s32 depth = rows * cols;

    u32 id = 0;
    api.gl.glGenTextures(1, &id);
    api.gl.glBindTexture(GL_TEXTURE_2D_ARRAY, id);

    api.gl.glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,
        GL_LINEAR);
    api.gl.glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,
        GL_LINEAR);

    api.gl.glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, tile_width,
        tile_height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    u32 channels = 4;
    u32 tile_size_bytes = tile_width * tile_height * channels;

    tile_data = stack_alloc(block, tile_size_bytes);

    for (u32 y = 0; y < rows; y++)
    {
        for (u32 x = 0; x < cols; x++)
        {
            s8* ptr_dest = tile_data;
            memory_set(ptr_dest, tile_size_bytes, 0);

            for (u32 i = 0; i < tile_height; i++)
            {
                s8* ptr_src = pixel_data + ((y * tile_height + i) *
                    image_width + x * tile_width) * channels;

                for (u32 j = 0; j < tile_width; j++)
                {
                    for (u32 k = 0; k < channels; k++)
                    {
                        *ptr_dest++ = *ptr_src++;
                    }
                }
            }

            u32 i = y * cols + x;
            api.gl.glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, tile_width,
                tile_height, 1, GL_RGBA, GL_UNSIGNED_BYTE, tile_data);
        }
    }

    stack_free(block);
    stack_free(block);
    stack_free(block);

    gl_check_error("texture_array_create");

    return id;
}

b32 str_compare(char* str1, char* str2)
{
    while (*str1++ == *str2++)
    {
        if (*str1 == '\0')
        {
            return true;
        }
    }

    return false;
}

b32 is_digit(char c)
{
    return c >= '0' && c <= '9';
}

b32 is_space(char c)
{
    return (c >= 9 && c <= 13) || c == 32;
}

s32 s32_parse(char* str, u64* size)
{
    // Todo: ignore leading whitespaces
    s32 value = 0;
    u64 bytes = 0;
    b32 negative = false;

    if (*str == '-')
    {
        negative = true;
        str++;
        bytes++;
    }

    while (is_digit(*str))
    {
        u8 val = *str++ - '0';

        value *= 10.0;
        value += val;

        bytes++;
    }

    if (negative)
    {
        value *= -1;
    }

    if (size)
    {
        *size = bytes;
    }

    return value;
}

f32 f32_parse(char* str, u64* size)
{
    // Todo: ignore leading whitespaces
    // Todo: int parser has duplicate code
    f32 value = 0;
    b32 negative = false;
    u64 bytes = 0;

    if (*str == '-')
    {
        negative = true;
        str++;
        bytes++;
    }

    while (is_digit(*str))
    {
        u8 val = *str++ - '0';

        value *= 10.0;
        value += val;
        bytes++;
    }

    if (*str++ == '.')
    {
        s32 num_decimals = 0;
        bytes++;

        while (is_digit(*str))
        {
            u8 val = *str++ - '0';

            value *= 10.0;
            value += val;

            num_decimals--;
            bytes++;
        }

        if (*str++ == 'e')
        {
            b32 negative_exponent = false;

            bytes++;

            if (*str == '-')
            {
                negative_exponent = true;
                str++;
                bytes++;
            }
            else if (*str == '+')
            {
                negative_exponent = false;
                str++;
                bytes++;
            }

            u32 exponent = 0;

            while (is_digit(*str))
            {
                u8 val = *str++ - '0';

                exponent *= 10.0;
                exponent += val;

                bytes++;
            }

            num_decimals += negative_exponent ? -exponent : exponent;
        }

        while (num_decimals < 0)
        {
            value *= 0.1;

            num_decimals++;
        }

        while (num_decimals > 0)
        {
            value *= 10;

            num_decimals--;
        }
    }

    if (negative)
    {
        value *= -1;
    }

    if (size)
    {
        *size = bytes;
    }

    return value;
}

u64 string_read(char* data, char* str, u64 max_size)
{
    u64 bytes_read = 0;

    while (is_space(*data) && *data != '\0')
    {
        data++;
        bytes_read++;
    }

    u64 str_size = 0;

    while (!is_space(*data) && *data != '\0' && str_size < max_size - 1)
    {
        *str++ = *data++;

        str_size++;
        bytes_read++;
    }

    *str = '\0';

    return bytes_read;
}

void mesh_create(struct memory_block* block, char* path, struct mesh* mesh)
{
    // Todo: remove statics
    static struct v3 in_vertices[4096];
    static struct v3 in_normals[4096];
    static struct v2 in_uvs[4096];
    static struct vertex vertices[4096];
    static u32 in_faces[4096*3];
    static u32 indices[4096];

    u32 num_in_vertices = 0;
    u32 num_in_normals = 0;
    u32 num_in_uvs = 0;
    u32 num_in_faces = 0;
    u32 num_vertices = 0;

    // Todo:
    // X read file size
    // X reserve space for file data
    // - reserve space for vertices, faces, texture coords and normals
    //   - can it be calculated beforehand?
    //   - read each line and count each face, vertex normal etc and then
    //     reserve memory accordingly?
    // - reserve space for vertices and indices
    // - count indices

    u64 read_bytes = 0;
    u64 file_size = 0;
    s8* file_data = 0;

    file_handle file;
    api.file.file_open(&file, path, true);
    api.file.file_size_get(&file, &file_size);

    // Todo: haxy way, but +1 for the ending \0, otherwise following loops may
    // fail if there's already something in the memory. Fix the loop to not go
    // beyond reserved memory!
    file_data = stack_alloc(block, file_size + 1);
    *(file_data + file_size) = '\0';

    api.file.file_read(&file, file_data, file_size, &read_bytes);
    api.file.file_close(&file);

    char* data = (char*)file_data;
    char str[255] = {0};

    for (u64 i = 0; i < read_bytes && *data != '\0'; i++)
    {
        u64 str_size = string_read(data, str, 255);

        if (str_compare(str, "v"))
        {
            struct v3* v = &in_vertices[num_in_vertices++];

            // LOG("v");

            data += str_size;
            str_size = string_read(data, str, 255);
            v->x = f32_parse(str, NULL);
            // LOG(" %f", v->x);

            data += str_size;
            str_size = string_read(data, str, 255);
            v->y = f32_parse(str, NULL);
            // LOG(" %f", v->y);

            data += str_size;
            str_size = string_read(data, str, 255);
            v->z = f32_parse(str, NULL);
            // LOG(" %f\n", v->z);
        }
        else if (str_compare(str, "vt"))
        {
            struct v2* uv = &in_uvs[num_in_uvs++];

            // LOG("vt");

            data += str_size;
            str_size = string_read(data, str, 255);
            uv->x = f32_parse(str, NULL);
            // LOG(" %f", uv->x);

            data += str_size;
            str_size = string_read(data, str, 255);
            uv->y = f32_parse(str, NULL);
            // LOG(" %f\n", uv->y);
        }
        else if (str_compare(str, "vn"))
        {
            struct v3* n = &in_normals[num_in_normals++];

            // LOG("vn");

            data += str_size;
            str_size = string_read(data, str, 255);
            n->x = f32_parse(str, NULL);
            // LOG(" %f", n->x);

            data += str_size;
            str_size = string_read(data, str, 255);
            n->y = f32_parse(str, NULL);
            // LOG(" %f", n->y);

            data += str_size;
            str_size = string_read(data, str, 255);
            n->z = f32_parse(str, NULL);
            // LOG(" %f\n", n->z);
        }
        else if (str_compare(str, "f"))
        {
            // LOG("f");

            for (u32 i = 0; i < 3; i++)
            {
                u32* face = &in_faces[num_in_faces];

                data += str_size;
                str_size = string_read(data, str, 255);

                char* s = str;

                u64 bytes_read = 0;
                face[0] = s32_parse(s, &bytes_read);
                // LOG(" %d", face[0]);
                s += bytes_read + 1;
                // LOG("%c", *s++);
                face[1] = s32_parse(s, &bytes_read);
                // LOG("%d", face[1]);
                s += bytes_read + 1;
                // LOG("%c", *s++);
                face[2] = s32_parse(s, &bytes_read);
                // LOG("%d", face[2]);

                num_in_faces += 3;
            }

            // LOG("\n");
        }
        else
        {
            data += str_size;
        }
    }

    for (u32 i = 0; i < num_in_faces; i += 3)
    {
        u32* face = &in_faces[i];

        struct vertex v;

        v.position = in_vertices[face[0] - 1];
        v.uv = in_uvs[face[1] - 1];
        v.normal = in_normals[face[2] - 1];
        v.color.r = 1.0f;
        v.color.g = 1.0f;
        v.color.b = 1.0f;
        v.color.a = 1.0f;

        b32 found = false;

        for (u32 j = 0; j < num_vertices; j++)
        {
            struct vertex other = vertices[j];

            if (v3_equals(v.position, other.position) &&
                v2_equals(v.uv, other.uv) &&
                v3_equals(v.normal, other.normal))
            {
                indices[mesh->num_indices++] = j;

                found = true;
                break;
            }
        }

        if (!found)
        {
            vertices[num_vertices++] = v;
            indices[mesh->num_indices++] = num_vertices - 1;
        }
    }

    stack_free(block);

    generate_vertex_array(mesh, vertices, num_vertices, indices);

    gl_check_error("mesh_create");
}

u32 program_create(struct memory_block* block, char* vertex_shader_path,
    char* fragment_shader_path)
{
    u64 read_bytes = 0;
    u64 file_size = 0;
    s8* file_data = 0;

    u32 result = 0;
    u32 program = api.gl.glCreateProgram();
    u32 vertex_shader = api.gl.glCreateShader(GL_VERTEX_SHADER);
    u32 fragment_shader = api.gl.glCreateShader(GL_FRAGMENT_SHADER);

    // Todo: implement assert
    // assert(program);
    // assert(vertex_shader);
    // assert(fragment_shader);

    // Note: glShaderSource requires for each string to be null terminated.
    // If read directly from a file, each line ends in CR LF (0d 0a).
    // These should be replaced with 0.
    file_handle file;

    api.file.file_open(&file, vertex_shader_path, true);
    api.file.file_size_get(&file, &file_size);

    // Reserve space for 0 at the end of shader data.
    file_data = stack_alloc(block, file_size + 1);
    *(file_data + file_size) = 0;

    api.file.file_read(&file, file_data, file_size, &read_bytes);
    api.file.file_close(&file);

    const GLchar* temp = (const GLchar*)file_data;

    api.gl.glShaderSource(vertex_shader, 1, &temp, 0);
    api.gl.glCompileShader(vertex_shader);
    api.gl.glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, (GLint*)&result);
    // assert(result);

    stack_free(block);

    api.file.file_open(&file, fragment_shader_path, true);
    api.file.file_size_get(&file, &file_size);

    // Reserve space for 0 at the end of shader data.
    file_data = stack_alloc(block, file_size + 1);
    *(file_data + file_size) = 0;

    api.file.file_read(&file, file_data, file_size, &read_bytes);
    api.file.file_close(&file);

    temp = (const GLchar*)file_data;

    api.gl.glShaderSource(fragment_shader, 1, &temp, 0);
    api.gl.glCompileShader(fragment_shader);
    api.gl.glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, (GLint*)&result);
    // assert(result);

    stack_free(block);

    api.gl.glAttachShader(program, vertex_shader);
    api.gl.glAttachShader(program, fragment_shader);

    api.gl.glLinkProgram(program);
    api.gl.glGetProgramiv(program, GL_LINK_STATUS, (GLint*)&result);
    // assert(result);

    api.gl.glDeleteShader(vertex_shader);
    api.gl.glDeleteShader(fragment_shader);

    gl_check_error("shader_create");

    return program;
}

void level_mask_init(struct game_state* state)
{
    // Inited once per game round e.g. until the player has died

    struct level* mask = &state->level_mask;

    // Todo: now hard coded, randomize in the future
    mask->width = 8;
    mask->height = 8;
    mask->start_pos = (struct v2) { 3.0f, 3.0f };

    // Level mask mask is used to make each level structurally compatible.
    // The outer walls and start room location will be the same for each
    // level but the inner walls may differ.
    {
        u32 width = mask->width;
        u32 height = mask->height;
        u32 start_x = mask->start_pos.x;
        u32 start_y = mask->start_pos.y;

        u64 size = width * height;
        u8* data = mask->tile_types;

        memory_set(data, size, 1);

        {
            data[5 * width + 3] = 0;
            data[5 * width + 4] = 0;
            data[6 * width + 3] = 0;
            data[6 * width + 4] = 0;
        }

        // Reserve space for the starting room
        data[start_y * width + start_x] = 0;
    }
}

void level_init(struct game_state* state)
{
    // Clear everything
    memory_set(state->bullets,
        sizeof(struct bullet) * MAX_BULLETS, 0);
    memory_set(state->enemies,
        sizeof(struct enemy) * MAX_ENEMIES, 0);
    memory_set(state->items,
        sizeof(struct item) * MAX_ITEMS, 0);
    memory_set(state->particle_lines,
        sizeof(struct particle_line) * MAX_PARTICLES, 0);
    memory_set(state->particle_spheres,
        sizeof(struct particle_sphere) * MAX_PARTICLES, 0);
    memory_set(state->wall_corners,
        sizeof(struct v2) * MAX_WALL_CORNERS, 0);
    memory_set(state->wall_faces,
        sizeof(struct line_segment) * MAX_WALL_FACES, 0);
    memory_set(state->cols_static,
        sizeof(struct line_segment) * MAX_COLLISION_SEGMENTS, 0);
    memory_set(state->cols_dynamic,
        sizeof(struct line_segment) * MAX_COLLISION_SEGMENTS, 0);
    memory_set(state->gun_shots,
        sizeof(struct gun_shot) * MAX_GUN_SHOTS, 0);

    state->num_enemies = 0;
    state->num_wall_corners = 0;
    state->num_wall_faces = 0;
    state->num_cols_static = 0;
    state->num_cols_dynamic = 0;
    state->num_gun_shots = 0;
    state->free_bullet = 0;
    state->free_item = 0;
    state->free_particle_line = 0;
    state->free_particle_sphere = 0;

    // Inited once per level
    u32 enemies_min = state->level_current;
    u32 enemies_max =  MIN(enemies_min * 4, MAX_ENEMIES);
    state->num_enemies = u32_random_number_get(state, enemies_min, enemies_max);
    // state->num_enemies = 1;

    LOG("%u enemies\n", state->num_enemies);

    level_generate(state, &state->level, &state->level_mask);

    u32 color_enemy = cube_renderer_color_add(&state->cube_renderer,
        (struct v4){ 0.7f, 0.90f, 0.1f, 1.0f });
    u32 color_player = cube_renderer_color_add(&state->cube_renderer,
        (struct v4){ 1.0f, 0.4f, 0.9f, 1.0f });

    for (u32 i = 0; i < state->num_enemies; i++)
    {
        struct enemy* enemy = &state->enemies[i];
        enemy->body.position = tile_random_get(state, &state->level,
            TILE_FLOOR);
        enemy->alive = true;
        enemy->health = ENEMY_HEALTH_MAX;
        enemy->vision_cone_size = 0.2f * i;
        enemy->shooting = false;
        enemy->cube.faces[0].texture = 13;
        enemy->state = u32_random_number_get(state, 0, 1) ? ENEMY_STATE_SLEEP :
            ENEMY_STATE_WANDER_AROUND;
        // enemy->state = ENEMY_STATE_WANDER_AROUND;

        LOG("Enemy %u is %s\n", i,
            enemy->state == ENEMY_STATE_SLEEP ? "sleeping" : "wandering");

        cube_data_color_update(&enemy->cube, color_enemy);

        enemy->weapon = weapon_create(u32_random_number_get(state, 1, 3));
        enemy->weapon.projectile_damage *= 0.2f;
    }

    if (state->level_current == 1)
    {
        state->player.weapon = weapon_create(WEAPON_PISTOL);
    }

    if (!state->player.alive)
    {
        state->player.body.position = state->level.start_pos;
        state->player.alive = true;
        state->player.health = PLAYER_HEALTH_MAX;
        state->player.cube.faces[0].texture = 11;
    }

    cube_data_color_update(&state->player.cube, color_player);

    state->mouse.world = state->player.body.position;

    state->camera.position.xy = state->level.start_pos;
    state->camera.target.xy = state->level.start_pos;
    state->camera.target.z = 3.75f;

    state->camera.view = m4_translate(-state->camera.position.x,
        -state->camera.position.y, -state->camera.position.z);

    state->camera.view_inverse = m4_inverse(state->camera.view);

    collision_map_static_calculate(&state->level, state->cols_static,
        MAX_COLLISION_SEGMENTS, &state->num_cols_static);

    LOG("Wall faces: %d/%d\n", state->num_cols_static,
        MAX_COLLISION_SEGMENTS);

    get_wall_corners_from_faces(state->wall_corners, MAX_WALL_CORNERS,
        &state->num_wall_corners, state->cols_static,
        state->num_cols_static);

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

void game_init(struct game_memory* memory, struct game_init* init)
{
    _log = *init->log;

    api = init->api;

    if (!memory->initialized)
    {
        struct game_state* state = (struct game_state*)memory->base;
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

        state->stack.base = (s8*)state + sizeof(struct game_state);
        state->stack.current = state->stack.base;
        state->stack.size = 100*1024*1024;
        state->random_seed = init->init_time;

        state->shader = program_create(&state->stack,
            "assets/shaders/vertex.glsl",
            "assets/shaders/fragment.glsl");

        state->shader_simple = program_create(&state->stack,
            "assets/shaders/vertex.glsl",
            "assets/shaders/fragment_simple.glsl");

        state->shader_cube = program_create(&state->stack,
            "assets/shaders/vertex_cube.glsl",
            "assets/shaders/fragment_cube.glsl");

        state->shader_sprite = program_create(&state->stack,
            "assets/shaders/vertex_sprite.glsl",
            "assets/shaders/fragment_sprite.glsl");

        state->texture_tileset = texture_create(&state->stack,
            "assets/textures/tileset.tga");
        state->texture_sphere = texture_create(&state->stack,
            "assets/textures/sphere.tga");
        state->texture_cube = texture_array_create(&state->stack,
            "assets/textures/cube.tga", 4, 4);
        state->texture_sprite = texture_array_create(&state->stack,
            "assets/textures/tileset.tga", 8, 8);

        cube_renderer_init(&state->cube_renderer, state->shader_cube,
            state->texture_cube);
        sprite_renderer_init(&state->sprite_renderer, state->shader_sprite,
            state->texture_sprite);

        mesh_create(&state->stack, "assets/meshes/sphere.mesh",
            &state->sphere);
        mesh_create(&state->stack, "assets/meshes/wall.mesh",
            &state->wall);
        mesh_create(&state->stack, "assets/meshes/floor.mesh",
            &state->floor);
        mesh_create(&state->stack, "assets/meshes/triangle.mesh",
            &state->triangle);

        state->camera.screen_width = init->screen_width;
        state->camera.screen_height = init->screen_height;
        state->camera.projection = m4_perspective(60.0f,
            (f32)state->camera.screen_width/(f32)state->camera.screen_height,
            0.1f, 15.0f);
        // state->camera.projection = m4_orthographic(-10.0f, 10.0f, -10.0f,
        //     10.0f, 0.1f, 100.0f);
        state->camera.projection_inverse = m4_inverse(state->camera.projection);
        state->render_debug = false;

        u32 num_colors = sizeof(colors) / sizeof(struct v4);

        for (u32 i = 0; i < num_colors; i++)
        {
            cube_renderer_color_add(&state->cube_renderer, colors[i]);
        }

        state->level_current = 1;

        level_mask_init(state);
        level_init(state);

        api.gl.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

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
        struct camera* camera = &state->camera;

        state->render_debug = input->enable_debug_rendering;

        api.gl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        f32 step = 1.0f / 120.0f;

        state->mouse.screen.x = input->mouse_x;
        state->mouse.screen.y = input->mouse_y;

        state->mouse.world = calculate_world_pos(input->mouse_x,
            input->mouse_y, camera);

        if (!input->pause)
        {
            state->accumulator += input->delta_time;

            while (!input->pause && state->accumulator >= step)
            {
                state->accumulator -= step;

                if (state->level_clear_notify <= 0.0f)
                {
                    player_update(state, input, step);
                    enemies_update(state, input, step);
                    bullets_update(state, input, step);
                    items_update(state, input, step);
                    particle_lines_update(state, input, step);
                    particle_spheres_update(state, input, step);
                    particles_update(&state->particles, step);
                }

                struct v2 start_min = v2_sub_f32(state->level.start_pos, 2.0f);
                struct v2 start_max = v2_add_f32(state->level.start_pos, 2.0f);
                struct v2 plr_pos = state->player.body.position;

                state->player_in_start_room = plr_pos.x > start_min.x &&
                    plr_pos.x < start_max.x && plr_pos.y > start_min.y &&
                    plr_pos.y < start_max.y;

                if (state->level_clear_notify > 0.0f)
                {
                    camera->target.x = state->level.start_pos.x;
                    camera->target.y = state->level.start_pos.y + 1.0f;
                    camera->target.z = 3.75f;

                    state->level_clear_notify -= step;
                }
                else if (state->player_in_start_room)
                {
                    camera->target.xy = state->level.start_pos;
                    camera->target.z = 3.75f;
                }
                else
                {
                    f32 distance_to_activate = 0.0f;

                    struct v2 direction_to_mouse = v2_normalize(v2_direction(
                        state->player.body.position, state->mouse.world));

                    struct v2 target_pos = v2_average(state->mouse.world,
                        state->player.body.position);

                    f32 distance_to_target = v2_distance(target_pos,
                        state->player.body.position);

                    distance_to_target -= distance_to_activate;

                    struct v2 target = state->player.body.position;

                    if (distance_to_target > 0)
                    {
                        target.x = state->player.body.position.x +
                            direction_to_mouse.x * distance_to_target;
                        target.y = state->player.body.position.y +
                            direction_to_mouse.y * distance_to_target;
                    }

                    camera->target.xy = target;
                    camera->target.z = 10.0f;
                }

                {
                    struct v3 dir = v3_direction(camera->position,
                        camera->target);

                    camera->position.x += dir.x * CAMERA_ACCELERATION * step;
                    camera->position.y += dir.y * CAMERA_ACCELERATION * step;
                    camera->position.z += dir.z * CAMERA_ACCELERATION * step;

                    struct v3 up = { 0.0f, 1.0f, 0.0f };

                    camera->view = m4_look_at(camera->position,
                        (struct v3) { camera->position.xy, 0.0f }, up);
                    camera->view_inverse = m4_inverse(camera->view);
                }

                state->mouse.world = calculate_world_pos(input->mouse_x,
                    input->mouse_y, camera);

                collision_map_dynamic_calculate(state);

                u32 num_keys = sizeof(input->keys)/sizeof(input->keys[0]);

                for (u32 i = 0; i < num_keys; i++)
                {
                    input->keys[i].transitions = 0;
                }

                state->num_gun_shots = 0;
            }
        }

        // u64 render_start = ticks_current_get();
        level_render(state, &state->level);
        player_render(state);
        enemies_render(state);
        bullets_render(state);
        items_render(state);

        cube_renderer_flush(&state->cube_renderer, &state->camera.view,
            &state->camera.projection);
        sprite_renderer_flush(&state->sprite_renderer, &state->camera.view,
            &state->camera.projection);

        particle_lines_render(state);
        particle_spheres_render(state);
        particles_render(state, &state->particles);

        // u64 render_end = ticks_current_get();

        // LOG("Render time: %f\n", time_elapsed_seconds(state, render_start,
        //     render_end));

        if (state->render_debug)
        {
            collision_map_render(state);
        }
        // cursor_render(state);

        if (state->level_change)
        {
            state->level_cleared = false;
            state->level_change = false;
            state->accumulator = 0;
            state->level_current++;
            level_init(state);
        }
    }
    else
    {
        LOG("game_update: memory not initialized!\n");
    }
}
