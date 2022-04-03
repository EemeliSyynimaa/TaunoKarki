#include "tk_platform.h"
#include "tk_math.h"
#include "tk_opengl.h"
#include "tk_file.h"
#include "tk_time.h"

#include <string.h>

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
    struct weapon weapons[3];
    f32 health;
    b32 alive;
    u32 weapon_current;
};

struct bullet
{
    struct rigid_body body;
    struct v2 start;
    f32 damage;
    b32 alive;
    b32 player_owned;
};

struct enemy
{
    struct rigid_body body;
    struct v2 path[256];
    struct v2 direction_aim;
    struct v2 direction_look;
    struct v2 eye_position;
    struct weapon weapon;
    u32 path_index;
    u32 path_length;
    f32 health;
    f32 trigger_release;
    f32 vision_cone_size;
    b32 player_in_view;
    b32 alive;
    b32 shooting;
};

struct camera
{
    struct m4 projection;
    struct m4 view;
    struct m4 projection_inverse;
    struct m4 view_inverse;
    struct v3 position;
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

#define MAX_CUBES 1024

struct cube_vertex_data
{
    struct v3 position;
    struct v3 normal;
    struct v2 uv;
};

struct cube_renderer
{
    struct m4 models[MAX_CUBES];
    struct v4 colors[MAX_CUBES];
    u32 textures[MAX_CUBES];
    u32 vao;
    u32 vbo_vertices;
    u32 vbo_colors;
    u32 vbo_textures;
    u32 vbo_models;
    u32 ibo;
    u32 num_indices;
    u32 num_cubes;
};

#define MAX_BULLETS 64
#define MAX_ENEMIES 5
#define MAX_WALL_CORNERS 512
#define MAX_WALL_FACES 512
#define MAX_COLLISION_SEGMENTS 1024
#define MAX_PARTICLES 1024

#define WEAPON_PISTOL     0
#define WEAPON_MACHINEGUN 1
#define WEAPON_SHOTGUN    2

struct memory_block
{
    s8* base;
    s8* current;
    s8* last;
    u64 size;
};

struct line_segment
{
    struct v2 start;
    struct v2 end;
    u32 type;
};

struct game_state
{
    struct player player;
    struct bullet bullets[MAX_BULLETS];
    struct enemy enemies[MAX_ENEMIES];
    struct particle_line particle_lines[MAX_PARTICLES];
    struct particle_sphere particle_spheres[MAX_PARTICLES];
    struct camera camera;
    struct mouse mouse;
    struct mesh sphere;
    struct mesh wall;
    struct mesh floor;
    struct mesh triangle;
    struct memory_block temporary;
    struct v2 wall_corners[MAX_WALL_CORNERS];
    struct line_segment wall_faces[MAX_WALL_FACES];
    struct line_segment cols_static[MAX_COLLISION_SEGMENTS];
    struct line_segment cols_dynamic[MAX_COLLISION_SEGMENTS];
    struct cube_renderer cube_renderer;
    b32 render_debug;
    f32 accumulator;
    u32 shader;
    u32 shader_simple;
    u32 shader_cube;
    u32 texture_tileset;
    u32 texture_sphere;
    u32 texture_cube;
    u32 free_bullet;
    u32 free_particle_line;
    u32 free_particle_sphere;
    u32 num_enemies;
    u32 num_wall_corners;
    u32 num_wall_faces;
    u32 num_cols_static;
    u32 num_cols_dynamic;
    u32 level;
    u32 random_seed;
    u32 ticks_per_second;
};

f32 time_elapsed_seconds(struct game_state* state, u32 ticks_start,
    u32 ticks_end)
{
    f32 result =  (f32)(ticks_end - ticks_start) / (f32)state->ticks_per_second;

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
    u32 result = random_number_generate(state) % max + min;

    return result;
}

f32 f32_random_number_get(struct game_state* state, f32 min, f32 max)
{
    f32 result = 0.0f;
    f32 rand = u32_random_number_get(state, 0, S32_MAX) / (f32)S32_MAX;
    result = min + rand * (max - min);

    return result;
}

#define MAP_WIDTH  20
#define MAP_HEIGHT 20

f32 PLAYER_ACCELERATION      = 40.0f;
f32 ENEMY_ACCELERATION       = 35.0f;
f32 ENEMY_LINE_OF_SIGHT_HALF = F64_PI * 0.125f; // 22.5 degrees
f32 FRICTION                 = 10.0f;
f32 PROJECTILE_RADIUS        = 0.035f;
f32 PROJECTILE_SPEED         = 100.0f;
f32 PLAYER_RADIUS            = 0.25f;
f32 WALL_SIZE                = 1.0f;

u32 TILE_NOTHING = 0;
u32 TILE_WALL    = 1;
u32 TILE_FLOOR   = 2;

u32 COLLISION_STATIC  = 1;
u32 COLLISION_PLAYER  = 2;
u32 COLLISION_ENEMY   = 4;
u32 COLLISION_BULLET  = 8;
u32 COLLISION_DYNAMIC = 14;
u32 COLLISION_ALL     = 255;

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

u8 map_data[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0,
    1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 0,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 1, 2, 1, 0,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 1, 0,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 1, 2, 1, 0,
    1, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2, 1, 0,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 1, 2, 1, 0,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 1, 0,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 1, 2, 1, 0,
    1, 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 0,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
    1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 1,
    1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 1,
    1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 1,
    1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

b32 tile_inside_map(struct v2 position)
{
    b32 result = false;

    s32 x = f32_round(position.x);
    s32 y = f32_round(position.y);

    result = x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT;

    return result;
}

b32 tile_is_of_type(struct v2 position, u32 type)
{
    b32 result = false;

    if (tile_inside_map(position))
    {
        s32 x = f32_round(position.x);
        s32 y = f32_round(position.y);

        result = map_data[y * MAP_WIDTH + x] == type;
    }

    return result;
}

b32 tile_is_free(struct v2 position)
{
    return tile_is_of_type(position, TILE_FLOOR);
}

struct v2 tile_random_get(struct game_state* state, u32 type)
{
    struct v2 result = { 0 };

    u32 max_iterations = 1000;

    while (max_iterations)
    {
        struct v2 position =
        {
            u32_random_number_get(state, 0, MAP_WIDTH),
            u32_random_number_get(state, 0, MAP_HEIGHT)
        };

        if (tile_is_of_type(position, type))
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

#define MAX_NODES 512

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

b32 neighbor_check(f32 x, f32 y, struct v2 neighbors[], u32* index)
{
    b32 result = false;

    struct v2 neighbor = { x, y };

    if (tile_is_free(neighbor))
    {
        neighbors[(*index)++] = neighbor;
        result = true;
    }

    return result;
}

u32 neighbors_get(struct node* node, struct v2 neighbors[])
{
    u32 result = 0;

    f32 x = node->position.x;
    f32 y = node->position.y;

    b32 left  = neighbor_check(x - 1.0f, y, neighbors, &result);
    b32 right = neighbor_check(x + 1.0f, y, neighbors, &result);
    b32 up    = neighbor_check(x, y + 1.0f, neighbors, &result);
    b32 down  = neighbor_check(x, y - 1.0f, neighbors, &result);

    if (left && up)
    {
        neighbor_check(x - 1.0f, y + 1.0f, neighbors, &result);
    }

    if (left && down)
    {
        neighbor_check(x - 1.0f, y - 1.0f, neighbors, &result);
    }

    if (right && up)
    {
        neighbor_check(x + 1.0f, y + 1.0f, neighbors, &result);
    }

    if (right && down)
    {
        neighbor_check(x + 1.0f, y - 1.0f, neighbors, &result);
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

    path[(*index)++] = node->position;
}

u32 path_find(struct v2 start, struct v2 goal, struct v2 path[], u32 path_size)
{
    u32 result = 0;

    if (!tile_is_free(start) || !tile_is_free(goal))
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

        u32 num_neighbors = neighbors_get(current, neighbors);

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
    }

    return result;
}

void* memory_get(struct memory_block* block, u64 size)
{
    if (block->current + size > block->base + block->size)
    {
        LOG("Not enough memory\n");

        return 0;
    }

    // Todo: check alignment
    block->last = block->current;
    block->current += size;

    return (void*)block->last;
}

void memory_free(struct memory_block* block)
{
    block->current = block->last;
}

void memory_set(void* data, u64 size, u8 value)
{
    for (u32 i = 0; i < size; i++)
    {
        *((u8*)data + i) = value;
    }
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
    struct v2 ndc = { clip.x / clip.z, clip.y / clip.z };

    result.x = (ndc.x + 1.0f) * camera->screen_width * 0.5f;
    result.y = (ndc.y + 1.0f) * camera->screen_height * 0.5f;

    return result;
}

void log_gl_error(char* t)
{
    GLenum error = glGetError();

    switch (error)
    {
        case GL_NO_ERROR:
            // LOG("glGetError(): NO ERROR (%s)\n", t);
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
}

void generate_vertex_array(struct mesh* mesh, struct vertex* vertices,
    u32 num_vertices, u32* indices)
{
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    glGenBuffers(1, &mesh->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(struct vertex),
        vertices, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &mesh->ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->num_indices * sizeof(u32),
        indices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    // Todo: implement offsetof
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex),
        (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex),
        (void*)12);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex),
        (void*)20);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(struct vertex),
        (void*)32);
}

void cube_renderer_init(struct cube_renderer* renderer)
{
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
            { 0.0f, 0.0f }
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
            { 0.0f, 0.0f }
        },
        // Bottom right
        {
            { -1.0f, -1.0f, -1.0f },
            { 0.0f, 0.0f, -1.0f },
            { 0.0f, 0.0f }
        },
        // Top right
        {
            { -1.0f, -1.0f, 1.0f },
            { -1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f }
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
            { 0.0f, 0.0f }
        },
        // Bottom right
        {
            { -1.0f, -1.0f, -1.0f },
            { -1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f }
        },
        // Top right
        {
            { 1.0f, 1.0f, 1.0f },
            { 1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f }
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
            { 0.0f, 0.0f }
        },
        // Bottom right
        {
            { 1.0f, 1.0f, -1.0f },
            { 1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f }
        },
        // Top right
        {
            { -1.0f, 1.0f, 1.0f },
            { 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f }
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
            { 0.0f, 0.0f }
        },
        // Bottom right
        {
            { -1.0f, 1.0f, -1.0f },
            { 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f }
        },
        // Top right
        {
            { 1.0f, -1.0f, 1.0f },
            { 0.0f, -1.0f, 0.0f },
            { 0.0f, 0.0f }
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
            { 0.0f, 0.0f }
        },
        // Bottom right
        {
            { 1.0f, -1.0f, -1.0f },
            { 0.0f, -1.0f, 0.0f },
            { 0.0f, 0.0f }
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

    glGenVertexArrays(1, &renderer->vao);
    glBindVertexArray(renderer->vao);

    glGenBuffers(1, &renderer->vbo_vertices);
    glGenBuffers(1, &renderer->vbo_colors);
    glGenBuffers(1, &renderer->vbo_textures);
    glGenBuffers(1, &renderer->vbo_models);
    glGenBuffers(1, &renderer->ibo);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    glEnableVertexAttribArray(7);
    glEnableVertexAttribArray(8);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct cube_vertex_data), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct cube_vertex_data), (void*)12);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
        sizeof(struct cube_vertex_data), (void*)24);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_textures);
    glBufferData(GL_ARRAY_BUFFER, sizeof(renderer->textures),
        renderer->textures, GL_DYNAMIC_DRAW);
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_colors);
    glBufferData(GL_ARRAY_BUFFER, sizeof(renderer->colors), renderer->colors,
        GL_DYNAMIC_DRAW);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_models);
    glBufferData(GL_ARRAY_BUFFER, sizeof(renderer->models), renderer->models,
        GL_DYNAMIC_DRAW);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(struct m4),
        (void*)0);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(struct m4),
        (void*)sizeof(struct v4));
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(struct m4),
        (void*)(sizeof(struct v4) * 2));
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(struct m4),
        (void*)(sizeof(struct v4) * 3));

    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);
    glVertexAttribDivisor(8, 1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderer->num_indices * sizeof(u32),
        indices, GL_DYNAMIC_DRAW);
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

void cube_renderer_add(struct cube_renderer* renderer, struct m4 model,
    struct v4 color, u32 texture)
{
    if (renderer->num_cubes < MAX_CUBES)
    {
        renderer->models[renderer->num_cubes] = model;
        renderer->colors[renderer->num_cubes] = color;
        renderer->textures[renderer->num_cubes] = texture;
        renderer->num_cubes++;
    }
}

void cube_renderer_flush(struct cube_renderer* renderer, u32 texture,
    u32 shader)
{
    glBindVertexArray(renderer->vao);

    glUseProgram(shader);

    u32 uniform_texture = glGetUniformLocation(shader, "uni_texture");

    glUniform1i(uniform_texture, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_models);
    glBufferSubData(GL_ARRAY_BUFFER, 0, renderer->num_cubes * sizeof(struct m4),
        renderer->models);

    // Todo: maybe passing colors and textures each frame is not the smartest
    // thing ever
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_colors);
    glBufferSubData(GL_ARRAY_BUFFER, 0, renderer->num_cubes * sizeof(struct v4),
        renderer->colors);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_textures);
    glBufferSubData(GL_ARRAY_BUFFER, 0, renderer->num_cubes * sizeof(u32),
        renderer->textures);

    glDrawElementsInstanced(GL_TRIANGLES, renderer->num_indices,
        GL_UNSIGNED_INT, NULL, renderer->num_cubes);

    glUseProgram(0);

    renderer->num_cubes = 0;
}

void mesh_render(struct mesh* mesh, struct m4* mvp, u32 texture, u32 shader,
    struct v4 color)
{
    glBindVertexArray(mesh->vao);

    glUseProgram(shader);

    u32 uniform_mvp = glGetUniformLocation(shader, "MVP");
    u32 uniform_texture = glGetUniformLocation(shader, "texture");
    u32 uniform_color = glGetUniformLocation(shader, "uniform_color");

    glUniform1i(uniform_texture, 0);
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, (GLfloat*)mvp);
    glUniform4fv(uniform_color, 1, (GLfloat*)&color);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, NULL);

    glUseProgram(0);
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
    f32 y = screen_pos.y + 6.0f;
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
    f32 y = screen_pos.y - 6.0f;
    f32 width = bar_length - 1.0f;
    f32 height = 10.0f;
    f32 angle = 0.0f;

    for (u32 i = 0; i < ammo; i++, x += bar_length)
    {
        gui_rect_render(state, x, y, width, height, angle, colors[YELLOW]);
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

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(struct vertex),
        vertices, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(u32),
        indices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    // Todo: implement offsetof
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex),
        (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex),
        (void*)12);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex),
        (void*)20);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(struct vertex),
        (void*)32);

    glUseProgram(state->shader_simple);

    u32 uniform_mvp = glGetUniformLocation(state->shader_simple, "MVP");
    u32 uniform_texture = glGetUniformLocation(state->shader_simple, "texture");
    u32 uniform_color = glGetUniformLocation(state->shader_simple,
        "uniform_color");

    struct m4 mvp = state->camera.view;
    mvp = m4_mul_m4(mvp, state->camera.projection);

    glUniform1i(uniform_texture, 0);
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, (GLfloat*)&mvp);
    glUniform4fv(uniform_color, 1, (GLfloat*)&color);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, NULL);

    glUseProgram(0);
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

struct ray_cast_collision
{
    struct v2 position;
    struct v2 wall_start;
    struct v2 wall_end;
    // Todo: add wall normal? Could be nice
    f32 ray_length;
};

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

        // Todo: these both cases can possibly be merged
        if (check_x)
        {
            struct v2 tile_left = { current.x - epsilon, current.y };
            struct v2 tile_right = { current.x + epsilon, current.y };

            if (!tile_is_free(tile_left) || !tile_is_free(tile_right))
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

            if (!tile_is_free(tile_top) || !tile_is_free(tile_bottom))
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

void map_render(struct game_state* state)
{
    // Todo: fix map rendering glitch (a wall block randomly drawn in a
    //       wrong place)
    for (u32 y = 0; y < MAP_HEIGHT; y++)
    {
        for (u32 x = 0; x < MAP_WIDTH; x++)
        {
            u32 index = y * MAP_WIDTH + x;

            u8 tile = map_data[index];

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

            if (tile == TILE_WALL)
            {
                for (u32 i = 0; i < state->level; i++)
                {
                    struct m4 transform = m4_translate(x, y,
                        0.5f - state->level + i + 1);
                    struct m4 rotation = m4_rotate_z(0.0f);
                    struct m4 scale = m4_scale_all(WALL_SIZE * 0.5f);

                    struct m4 model = m4_mul_m4(scale, rotation);
                    model = m4_mul_m4(model, transform);

                    struct m4 mvp = m4_mul_m4(model, state->camera.view);
                    mvp = m4_mul_m4(mvp, state->camera.projection);

                    // cube_renderer_add(&state->cube_renderer, cube);
                    mesh_render(&state->wall, &mvp, state->texture_tileset,
                        state->shader, color);
                }
            }
            else if (tile == TILE_FLOOR)
            {
                struct m4 transform = m4_translate(x, y, 0.0f);
                struct m4 rotation = m4_rotate_z(0.0f);
                struct m4 scale = m4_scale_all(WALL_SIZE * 0.5f);

                struct m4 model = m4_mul_m4(scale, rotation);
                model = m4_mul_m4(model, transform);

                struct m4 mvp = m4_mul_m4(model, state->camera.view);
                mvp = m4_mul_m4(mvp, state->camera.projection);

                mesh_render(&state->floor, &mvp, state->texture_tileset,
                    state->shader, color);
            }
        }
    }
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

b32 check_tile_collisions(struct v2* pos, struct v2* vel, struct v2 move_delta,
    f32 radius, f32 bounce_factor)
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
                if (map_data[y * MAP_WIDTH + x] != 1)
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
        }
    }
}

void weapon_reload(struct weapon* weapon)
{
    if (weapon->type == WEAPON_PISTOL)
    {
        if (weapon->ammo < weapon->ammo_max && !weapon->reloading)
        {
            weapon->ammo = 0;
            weapon->reloading = true;
            weapon->last_shot = weapon->reload_time;
        }
    }
    else if (weapon->type == WEAPON_MACHINEGUN)
    {
        if (weapon->ammo < weapon->ammo_max && !weapon->reloading)
        {
            weapon->ammo = 0;
            weapon->reloading = true;
            weapon->last_shot = weapon->reload_time;
        }
    }
    else if (weapon->type == WEAPON_SHOTGUN)
    {
        if (weapon->ammo < weapon->ammo_max && !weapon->reloading)
        {
            weapon->reloading = true;
            weapon->last_shot = weapon->reload_time;
        }
    }
}

void enemies_update(struct game_state* state, struct game_input* input, f32 dt)
{
    // Todo: clean this function...
    for (u32 i = 0; i < MAX_ENEMIES; i++)
    {
        struct enemy* enemy = &state->enemies[i];

        enemy->alive = enemy->health > 0.0f;

        if (enemy->alive)
        {
            struct v2 acceleration = { 0.0f };
            struct v2 move_delta = { 0.0f };
            struct v2 direction_move = { 0.0f };

            if (enemy->path_index < enemy->path_length)
            {
                while (true)
                {
                    struct v2 current = enemy->path[enemy->path_index];

                    f32 distance_to_current = v2_distance(enemy->body.position,
                        current);
                    f32 epsilon = 0.25f;

                    if (distance_to_current < epsilon)
                    {
                        enemy->path_index += 1;

                        for (u32 j = enemy->path_index;
                            j < enemy->path_length;
                            j++)
                        {

                            // Todo: this doesn't take the enemy's size into
                            // account and it might stumble on the walls (but
                            // should not get stuck)
                            if (!ray_cast_position(state, enemy->eye_position,
                                enemy->path[j], NULL, COLLISION_STATIC))
                            {
                                break;
                            }

                            enemy->path_index = j;
                        }

                        if (enemy->path_index < enemy->path_length)
                        {
                            continue;
                        }
                    }
                    else
                    {
                        direction_move = v2_normalize(v2_direction(
                            enemy->body.position, current));

                        f32 length = v2_length(direction_move);

                        if (length > 1.0f)
                        {
                            direction_move.x /= length;
                            direction_move.y /= length;
                        }
                    }
                    break;
                }
            }
            else
            {
                struct v2 target = tile_random_get(state, TILE_FLOOR);

                enemy->path_length = path_find(enemy->body.position,
                    target, enemy->path, 256);
                enemy->path_index = 0;

                for (u32 j = 0; j < enemy->path_length; j++)
                {
                    if (!ray_cast_position(state, enemy->eye_position,
                        enemy->path[j], NULL, COLLISION_STATIC))
                    {
                        break;
                    }

                    enemy->path_index = j;
                }
            }

            acceleration.x = direction_move.x * ENEMY_ACCELERATION;
            acceleration.y = direction_move.y * ENEMY_ACCELERATION;

            acceleration.x += -enemy->body.velocity.x * FRICTION;
            acceleration.y += -enemy->body.velocity.y * FRICTION;

            move_delta.x = 0.5f * acceleration.x * f32_square(dt) +
                enemy->body.velocity.x * dt;
            move_delta.y = 0.5f * acceleration.y * f32_square(dt) +
                enemy->body.velocity.y * dt;

            enemy->body.velocity.x = enemy->body.velocity.x + acceleration.x *
                dt;
            enemy->body.velocity.y = enemy->body.velocity.y + acceleration.y *
                dt;

            check_tile_collisions(&enemy->body.position, &enemy->body.velocity,
                move_delta, PLAYER_RADIUS, 1);

            struct v2 direction_player = v2_normalize(v2_direction(
                enemy->body.position, state->player.body.position));
            struct v2 direction_current = v2_direction_from_angle(
                enemy->body.angle);

            f32 angle_player = v2_angle(direction_player, direction_current);

            f32 vision_cone_update_speed = 3.0f;
            f32 vision_cone_size_min = 0.25f;
            f32 vision_cone_size_max = 1.0f;

            // Todo: sometimes rotation goes crazy
            if (state->player.alive &&
                angle_player < ENEMY_LINE_OF_SIGHT_HALF &&
                ray_cast_body(state, enemy->eye_position, state->player.body,
                    NULL, COLLISION_ALL))
            {
                struct v2 target_forward = direction_player;

                if (v2_length(enemy->body.velocity))
                {
                    // Todo: calculate right with cross product
                    f32 angle = F64_PI*1.5f;
                    f32 tsin = f32_sin(angle);
                    f32 tcos = f32_cos(angle);

                    struct v2 target_right =
                    {
                        target_forward.x * tcos - target_forward.y * tsin,
                        target_forward.x * tsin + target_forward.y * tcos
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
                        v2_dot(desired_velocity, target_right_rotate_back),
                        v2_dot(desired_velocity, target_forward_rotate_back)
                    };

                    enemy->direction_aim = v2_normalize(final_direction);
                }

                enemy->vision_cone_size -= dt * vision_cone_update_speed;
                enemy->vision_cone_size = MAX(enemy->vision_cone_size,
                    vision_cone_size_min);
                enemy->direction_look = direction_player;
                enemy->player_in_view = true;

                // Todo: this is a bit hacky
                {
                    enemy->body.velocity.x = 0.0f;
                    enemy->body.velocity.y = 0.0f;
                }
            }
            else
            {
                enemy->vision_cone_size += dt * vision_cone_update_speed;
                enemy->vision_cone_size = MIN(enemy->vision_cone_size,
                    vision_cone_size_max);
                enemy->direction_look = direction_move;
                enemy->direction_aim = direction_move;
                enemy->player_in_view = false;
            }

            {
                f32 target_angle = f32_atan(enemy->direction_look.y,
                    enemy->direction_look.x);

                f32 circle = F64_PI * 2.0f;

                if (target_angle < 0.0f)
                {
                    target_angle += circle;
                }

                f32 diff_clockwise = 0.0f;
                f32 diff_counter_clockwise = 0.0f;

                if (target_angle < enemy->body.angle)
                {
                    diff_clockwise = target_angle + circle - enemy->body.angle;
                    diff_counter_clockwise = enemy->body.angle - target_angle;
                }
                else
                {
                    diff_clockwise = target_angle - enemy->body.angle;
                    diff_counter_clockwise = enemy->body.angle + circle -
                        target_angle;
                }

                f32 speed = F64_PI * 2.0f * dt;

                if (f32_abs(MIN(diff_clockwise, diff_counter_clockwise))
                    < (speed + 0.2f))
                {
                    enemy->body.angle = target_angle;
                }
                else if (diff_clockwise > diff_counter_clockwise)
                {
                    enemy->body.angle -= speed;
                }
                else
                {
                    enemy->body.angle += speed;
                }
            }

            struct v2 eye = { PLAYER_RADIUS + 0.0001f, 0.0f };

            enemy->eye_position = v2_rotate(eye, enemy->body.angle);
            enemy->eye_position.x += enemy->body.position.x;
            enemy->eye_position.y += enemy->body.position.y;

            enemy->shooting = enemy->player_in_view;

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

            if (enemy->shooting)
            {
                weapon->direction = enemy->direction_aim;
                weapon->position = enemy->eye_position;
                weapon->velocity = enemy->body.velocity;

                weapon_shoot(state, weapon, false);

                if (weapon->fired && enemy->trigger_release <= 0.0f)
                {

                    enemy->trigger_release = 0.5f;
                }
            }
            else
            {
                weapon_reload(weapon);
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

void get_wall_corners(struct v2 corners[], u32 max, u32* count)
{
    for (u32 y = 0; y < MAP_HEIGHT; y++)
    {
        for (u32 x = 0; x < MAP_WIDTH; x++)
        {
            struct v2 tile = { x, y };

            if (tile_is_of_type(tile, TILE_WALL))
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

void collision_map_static_calculate(struct line_segment faces[], u32 max,
    u32* count)
{
    for (u32 y = 0; y < MAP_HEIGHT; y++)
    {
        struct line_segment face_top    = { 0.0f };
        struct line_segment face_bottom = { 0.0f };

        for (u32 x = 0; x <= MAP_WIDTH; x++)
        {
            struct v2 tile = { x, y };
            face_top.type = COLLISION_STATIC;
            face_bottom.type = COLLISION_STATIC;

            if (tile_is_of_type(tile, TILE_WALL))
            {
                f32 t = WALL_SIZE * 0.5f;

                struct v2 tile_top    = { tile.x, tile.y + WALL_SIZE };
                struct v2 tile_bottom = { tile.x, tile.y - WALL_SIZE };

                if (tile_inside_map(tile_bottom) && tile_is_free(tile_bottom))
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

                if (tile_inside_map(tile_top) && tile_is_free(tile_top))
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

    for (u32 x = 0; x < MAP_WIDTH; x++)
    {
        struct line_segment face_left  = { 0.0f };
        struct line_segment face_right = { 0.0f };

        for (u32 y = 0; y < MAP_HEIGHT; y++)
        {
            struct v2 tile = { x, y };
            face_left.type = COLLISION_STATIC;
            face_right.type = COLLISION_STATIC;

            if (tile_is_of_type(tile, TILE_WALL))
            {
                f32 t = WALL_SIZE * 0.5f;

                struct v2 tile_left  = { tile.x - WALL_SIZE, tile.y };
                struct v2 tile_right = { tile.x + WALL_SIZE, tile.y };

                if (tile_inside_map(tile_left) && tile_is_free(tile_left))
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

                if (tile_inside_map(tile_right) && tile_is_free(tile_right))
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
            struct m4 transform = m4_translate(enemy->body.position.x,
                enemy->body.position.y, PLAYER_RADIUS);
            struct m4 rotation = m4_rotate_z(enemy->body.angle);
            struct m4 scale = m4_scale_xyz(PLAYER_RADIUS, PLAYER_RADIUS, 0.25f);

            struct m4 model = m4_mul_m4(scale, rotation);
            model = m4_mul_m4(model, transform);

            struct m4 mvp = m4_mul_m4(model, state->camera.view);
            mvp = m4_mul_m4(mvp, state->camera.projection);

            cube_renderer_add(&state->cube_renderer, mvp, colors[WHITE], 13);
            // cube_render(&state->cube, &mvp, state->texture_cube,
            //     state->shader_cube, colors[WHITE]);

            if (state->render_debug)
            {
                line_of_sight_render(state, enemy->eye_position,
                    enemy->body.angle, enemy->vision_cone_size *
                    ENEMY_LINE_OF_SIGHT_HALF,
                    enemy->player_in_view ? colors[PURPLE] : colors[TEAL],
                    true);
            }

            health_bar_render(state, enemy->body.position, enemy->health,
                100.0f);
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

                mvp = m4_mul_m4(model, state->camera.view);
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

                mvp = m4_mul_m4(model, state->camera.view);
                mvp = m4_mul_m4(mvp, state->camera.projection);

                mesh_render(&state->floor, &mvp, state->texture_tileset,
                    state->shader_simple, colors[YELLOW]);
            }

            // Render aim vector
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

                mvp = m4_mul_m4(model, state->camera.view);
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
                particle_sphere_create(state, bullet->body.position,
                    (struct v2){ 0.0f, 0.0f }, colors[GREY], PROJECTILE_RADIUS,
                    PROJECTILE_RADIUS * 5.0f, 0.15f);
                bullet->alive = false;
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

                        particle_sphere_create(state, bullet->body.position,
                            (struct v2){ bullet->body.velocity.x * 0.5,
                                bullet->body.velocity.y * 0.5 }, colors[RED],
                            PROJECTILE_RADIUS, PROJECTILE_RADIUS * 2.5f, 0.15f);
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

        check_tile_collisions(&player->body.position, &player->body.velocity,
            move_delta, PLAYER_RADIUS, 1);

        struct v2 eye = { PLAYER_RADIUS + 0.0001f, 0.0f };

        player->eye_position = v2_rotate(eye, player->body.angle);
        player->eye_position.x += player->body.position.x;
        player->eye_position.y += player->body.position.y;

        // Todo: maybe update the status of weapon that's changed
        u32 weapon_next = player->weapon_current;
        struct weapon* weapon = &player->weapons[player->weapon_current];

        if (input->weapon_slot_1.key_down)
        {
            LOG("Change weapon to PISTOL\n");
            weapon_next = 0;
        }
        else if (input->weapon_slot_2.key_down)
        {
            LOG("Change weapon to MACHINEGUN\n");
            weapon_next = 1;
        }
        else if (input->weapon_slot_3.key_down)
        {
            LOG("Change weapon to SHOTGUN\n");
            weapon_next = 2;
        }

        if (weapon_next != player->weapon_current)
        {
            weapon->fired = true;

            player->weapon_current = weapon_next;
            weapon = &player->weapons[player->weapon_current];
        }
        else if (weapon->last_shot > 0.0f)
        {
            weapon->last_shot -= dt;
        }

        weapon->direction = dir;
        weapon->position = player->eye_position;
        weapon->velocity = player->body.velocity;

        if (input->reload.key_down)
        {
            weapon_reload(weapon);
        }

        if (input->shoot.key_down)
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

        struct m4 mvp = m4_mul_m4(model, state->camera.view);
        mvp = m4_mul_m4(mvp, state->camera.projection);

        cube_renderer_add(&state->cube_renderer, mvp, colors[WHITE], 15);
        // cube_render(&state->cube, &mvp, state->texture_cube,
        //     state->shader_cube, colors[WHITE]);

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

            mvp = m4_mul_m4(model, state->camera.view);
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

            mvp = m4_mul_m4(model, state->camera.view);
            mvp = m4_mul_m4(mvp, state->camera.projection);

            mesh_render(&state->floor, &mvp, state->texture_tileset,
                state->shader_simple, colors[RED]);
        }

        struct weapon* weapon = &player->weapons[player->weapon_current];
        health_bar_render(state, player->body.position, player->health, 100.0f);
        ammo_bar_render(state, player->body.position, weapon->ammo,
            weapon->ammo_max);
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

    file_open(&file, path, true);
    file_size_get(&file, &file_size);

    file_data = memory_get(block, file_size);
    pixel_data = memory_get(block, file_size);

    file_read(&file, file_data, file_size, &read_bytes);
    file_close(&file);

    tga_decode(file_data, read_bytes, pixel_data, &width, &height);

    glGenTextures(1, &id);
    glBindTexture(target, id);

    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(target, 0, GL_RGBA, width, height, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, pixel_data);

    memory_free(block);
    memory_free(block);

    log_gl_error("texture_create");

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

    file_open(&file, path, true);
    file_size_get(&file, &file_size);

    file_data = memory_get(block, file_size);
    pixel_data = memory_get(block, file_size);

    file_read(&file, file_data, file_size, &read_bytes);
    file_close(&file);

    u32 image_width = 0;
    u32 image_height = 0;

    tga_decode(file_data, read_bytes, pixel_data, &image_width, &image_height);

    u32 tile_width = image_width / cols;
    u32 tile_height = image_height / rows;
    s32 depth = rows * cols;

    u32 id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, tile_width, tile_height,
        depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    u32 channels = 4;
    u32 tile_size_bytes = tile_width * tile_height * channels;

    tile_data = memory_get(block, tile_size_bytes);

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
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, tile_width,
                tile_height, 1, GL_RGBA, GL_UNSIGNED_BYTE, tile_data);
        }
    }

    memory_free(block);
    memory_free(block);
    memory_free(block);

    log_gl_error("texture_array_create");

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
    file_open(&file, path, true);
    file_size_get(&file, &file_size);

    file_data = memory_get(block, file_size);

    file_read(&file, file_data, file_size, &read_bytes);
    file_close(&file);

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

    memory_free(block);

    generate_vertex_array(mesh, vertices, num_vertices, indices);

    log_gl_error("mesh_create");
}

u32 program_create(struct memory_block* block, char* vertex_shader_path,
    char* fragment_shader_path)
{
    u64 read_bytes = 0;
    u64 file_size = 0;
    s8* file_data = 0;

    u32 result = 0;
    u32 program = glCreateProgram();
    u32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    u32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    // Todo: implement assert
    // assert(program);
    // assert(vertex_shader);
    // assert(fragment_shader);

    // Note: glShaderSource requires for each string to be null terminated.
    // If read directly from a file, each line ends in CR LF (0d 0a).
    // These should be replaced with 0.
    file_handle file;

    file_open(&file, vertex_shader_path, true);
    file_size_get(&file, &file_size);

    file_data = memory_get(block, file_size);

    file_read(&file, file_data, file_size, &read_bytes);
    file_close(&file);

    const GLchar* temp = (const GLchar*)file_data;

    glShaderSource(vertex_shader, 1, &temp, 0);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, (GLint*)&result);
    // assert(result);

    // Todo: remove memset
    memset((void*)file_data, 0, file_size);

    memory_free(block);

    file_open(&file, fragment_shader_path, true);
    file_size_get(&file, &file_size);

    file_data = memory_get(block, file_size);

    file_read(&file, file_data, file_size, &read_bytes);
    file_close(&file);

    temp = (const GLchar*)file_data;

    glShaderSource(fragment_shader, 1, &temp, 0);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, (GLint*)&result);
    // assert(result);

    // Todo: remove memset
    memset((void*)file_data, 0, file_size);

    memory_free(block);

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, (GLint*)&result);
    // assert(result);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    log_gl_error("shader_create");

    return program;
}

void game_init(struct game_memory* memory, struct game_init* init)
{
    struct game_state* state = (struct game_state*)memory->base;

    _log = *init->log;
    // Todo: should we check if copied functions are valid before use?
    opengl_functions_set(init->gl);
    file_functions_set(init->file);
    time_functions_set(init->time);

    s32 version_major = 0;
    s32 version_minor = 0;
    s32 uniform_blocks_max_vertex = 0;
    s32 uniform_blocks_max_geometry = 0;
    s32 uniform_blocks_max_fragment = 0;
    s32 uniform_blocks_max_combined = 0;
    s32 uniform_buffer_max_bindings = 0;
    s32 uniform_block_max_size = 0;

    glGetIntegerv(GL_MAJOR_VERSION, &version_major);
    glGetIntegerv(GL_MINOR_VERSION, &version_minor);
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &uniform_blocks_max_vertex);
    glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &uniform_blocks_max_geometry);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &uniform_blocks_max_fragment);
    glGetIntegerv(GL_MAX_COMBINED_UNIFORM_BLOCKS, &uniform_blocks_max_combined);
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &uniform_buffer_max_bindings);
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &uniform_block_max_size);

    LOG("OpenGL %i.%i\n", version_major, version_minor);
    LOG("Uniform blocks max vertex: %d\n", uniform_blocks_max_vertex);
    LOG("Uniform blocks max gemoetry: %d\n", uniform_blocks_max_geometry);
    LOG("Uniform blocks max fragment: %d\n", uniform_blocks_max_fragment);
    LOG("Uniform blocks max combined: %d\n", uniform_blocks_max_combined);
    LOG("Uniform buffer max bindings: %d\n", uniform_buffer_max_bindings);
    LOG("Uniform block max size: %d\n", uniform_block_max_size);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!memory->initialized)
    {
        state->ticks_per_second = ticks_frequency_get();
        state->temporary.base = (s8*)state + sizeof(struct game_state);
        state->temporary.last = state->temporary.base;
        state->temporary.current = state->temporary.base;
        state->temporary.size = 100*1024*1024;

        state->shader = program_create(&state->temporary,
            "assets/shaders/vertex.glsl",
            "assets/shaders/fragment.glsl");

        state->shader_simple = program_create(&state->temporary,
            "assets/shaders/vertex.glsl",
            "assets/shaders/fragment_simple.glsl");

        state->shader_cube = program_create(&state->temporary,
            "assets/shaders/vertex_cube.glsl",
            "assets/shaders/fragment_cube.glsl");

        state->texture_tileset = texture_create(&state->temporary,
            "assets/textures/tileset.tga");
        state->texture_sphere = texture_create(&state->temporary,
            "assets/textures/sphere.tga");
        state->texture_cube = texture_array_create(&state->temporary,
            "assets/textures/cube.tga", 4, 4);

        cube_renderer_init(&state->cube_renderer);

        mesh_create(&state->temporary, "assets/meshes/sphere.mesh",
            &state->sphere);
        mesh_create(&state->temporary, "assets/meshes/wall.mesh",
            &state->wall);
        mesh_create(&state->temporary, "assets/meshes/floor.mesh",
            &state->floor);
        mesh_create(&state->temporary, "assets/meshes/triangle.mesh",
            &state->triangle);

        memory->initialized = true;
    }

    state->camera.screen_width = init->screen_width;
    state->camera.screen_height = init->screen_height;
    state->camera.projection = m4_perspective(60.0f,
        (f32)state->camera.screen_width/(f32)state->camera.screen_height,
        0.1f, 100.0f);
    // state->camera.projection = m4_orthographic(-10.0f, 10.0f, -10.0f, 10.0f,
    //     0.1f, 100.0f);
    state->camera.projection_inverse = m4_inverse(state->camera.projection);

    state->random_seed = init->init_time;

    state->num_enemies = 5;

    for (u32 i = 0; i < state->num_enemies; i++)
    {
        struct enemy* enemy = &state->enemies[i];
        enemy->body.position = tile_random_get(state, TILE_FLOOR);
        enemy->alive = true;
        enemy->health = 100.0f;
        enemy->body.angle = f32_radians(270 - i * 15.0f);
        enemy->vision_cone_size = 0.2f * i;
        enemy->shooting = false;
    }

    state->level = 2;
    state->render_debug = false;

    state->player.body.position.x = 3.0f;
    state->player.body.position.y = 3.0f;
    state->player.alive = true;
    state->player.health = 100.0f;
    state->player.weapon_current = WEAPON_PISTOL;

    struct weapon* weapon = &state->player.weapons[0];
    weapon->type = WEAPON_PISTOL;
    weapon->last_shot = 0.0f;
    weapon->fired = true;
    weapon->ammo_max = weapon->ammo = 12;
    weapon->fire_rate = 0.0f;
    weapon->reload_time = 0.8f / weapon->ammo;
    weapon->reloading = false;
    weapon->spread = 0.0125f;
    weapon->projectile_size = PROJECTILE_RADIUS;
    weapon->projectile_speed = PROJECTILE_SPEED;
    weapon->projectile_damage = 20.0f;

    ++weapon;
    weapon->type = WEAPON_MACHINEGUN;
    weapon->last_shot = 0.0f;
    weapon->fired = true;
    weapon->ammo_max = weapon->ammo = 40;
    weapon->fire_rate = 0.075f;
    weapon->reload_time = 1.1f / weapon->ammo;
    weapon->reloading = false;
    weapon->spread = 0.035f;
    weapon->projectile_size = PROJECTILE_RADIUS * 0.75f;
    weapon->projectile_speed = PROJECTILE_SPEED;
    weapon->projectile_damage = 10.0f;

    ++weapon;
    weapon->type = WEAPON_SHOTGUN;
    weapon->last_shot = 0.0f;
    weapon->fired = true;
    weapon->ammo_max = weapon->ammo = 8;
    weapon->fire_rate = 0.5f;
    weapon->reload_time = 3.0f / weapon->ammo;
    weapon->reloading = false;
    weapon->spread = 0.125f;
    weapon->projectile_size = PROJECTILE_RADIUS * 0.25f;
    weapon->projectile_speed = PROJECTILE_SPEED;
    weapon->projectile_damage = 7.5f;

    for (u32 i = 0; i < state->num_enemies; i++)
    {
        struct enemy* enemy = &state->enemies[i];

        enemy->weapon = state->player.weapons[i % 3];
        enemy->weapon.projectile_damage *= 0.2f;
    }

    state->mouse.world = state->player.body.position;

    struct v2 temp =
    {
        state->mouse.world.x - state->player.body.position.x,
        state->mouse.world.y - state->player.body.position.y
    };

    state->camera.position.x = state->player.body.position.x + temp.x * 0.5f;
    state->camera.position.y = state->player.body.position.y + temp.y * 0.5f;
    state->camera.position.z = 10.0f;

    state->camera.view = m4_translate(-state->camera.position.x,
        -state->camera.position.y, -state->camera.position.z);

    state->camera.view_inverse = m4_inverse(state->camera.view);

    collision_map_static_calculate(state->cols_static, MAX_COLLISION_SEGMENTS,
        &state->num_cols_static);

    LOG("Wall faces: %d/%d\n", state->num_cols_static, MAX_COLLISION_SEGMENTS);

    get_wall_corners_from_faces(state->wall_corners, MAX_WALL_CORNERS,
        &state->num_wall_corners, state->cols_static, state->num_cols_static);

    LOG("Wall corners: %d/%d\n", state->num_wall_corners, MAX_WALL_CORNERS);

    glClearColor(0.2f, 0.65f, 0.4f, 0.0f);

    if (!memory->initialized)
    {
        LOG("game_init: end of init, memory not initalized!\n");
    }
}


void game_update(struct game_memory* memory, struct game_input* input)
{
    static f32 test_rotation = 0.0f;

    if (memory->initialized)
    {
        struct game_state* state = (struct game_state*)memory->base;
        struct camera* camera = &state->camera;

        state->render_debug = input->enable_debug_rendering;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        f32 step = 1.0f / 120.0f;
        state->accumulator += input->delta_time;

        state->mouse.screen.x = input->mouse_x;
        state->mouse.screen.y = input->mouse_y;

        state->mouse.world = calculate_world_pos(input->mouse_x, 
            input->mouse_y, camera);
            
        while (state->accumulator >= step)
        {
            state->accumulator -= step;

            player_update(state, input, step);
            enemies_update(state, input, step);
            bullets_update(state, input, step);
            particle_lines_update(state, input, step);
            particle_spheres_update(state, input, step);

            camera->position.x = state->player.body.position.x;
            camera->position.y = state->player.body.position.y - 5.0f;
            camera->position.z = 7.5f;

            struct v3 target = 
            {
                state->player.body.position.x,
                state->player.body.position.y,
                0.0f
            };

            struct v3 up = { 0.0f, 1.0f, 0.0f };

            camera->view = m4_look_at(camera->position, target, up);
            camera->view_inverse = m4_inverse(camera->view);

            state->mouse.world = calculate_world_pos(input->mouse_x, 
                input->mouse_y, camera);

            collision_map_dynamic_calculate(state);

            test_rotation += step;
        }

        u64 render_start = ticks_current_get();
        map_render(state);
        player_render(state);
        enemies_render(state);
        bullets_render(state);
        particle_lines_render(state);
        particle_spheres_render(state);
        cube_renderer_flush(&state->cube_renderer, state->texture_cube,
            state->shader_cube);
        u64 render_end = ticks_current_get();

        LOG("Render time: %f\n", time_elapsed_seconds(state, render_start,
            render_end));

        if (state->render_debug)
        {
            collision_map_render(state);
        }
        // cursor_render(state);
    }
    else
    {
        LOG("game_update: memory not initialized!\n");
    }
}
