// Todo: some structs etc. are temporarily prefixed with tk_ to avoid conflicts

enum
{
    TK_ENTITY_ALIVE             = 1 << 0,
    TK_ENTITY_RENDERABLE        = 1 << 1,
    TK_ENTITY_DYNAMIC           = 1 << 2,
    TK_ENTITY_PLAYER_CONTROLLED = 1 << 3,
};

enum
{
    TK_RENDER_TYPE_CIRCLE,
    TK_RENDER_TYPE_AABB
};

struct tk_contact
{
    struct v2 position;
    f32 time_delta;
    b32 active;
};

#define TK_ENTITY_NAME_MAX 256

struct tk_entity
{
    u64 flags;

    char name[TK_ENTITY_NAME_MAX];

    struct v2 position;

    // Physics
    struct v2 velocity;
    struct v2 move_delta;
    struct v2 force;

    // Todo: testing only
    b32 in_contact;

    f32 friction;
    f32 acceleration;
    f32 mass;

    // Render types
    u32 render_type;

    struct v4 color;

    // Circle
    f32 radius;
    u32 num_fans;

    // AABB
    f32 half_width;
    f32 half_height;
};

#define TK_MAX_ENTITIES 32

#define MAX_PHYSICS_FRAMES 3600

struct physics_frame
{
    struct tk_entity entities[TK_MAX_ENTITIES];
    u32 num_entities;
    u32 id;
};

struct scene_physics
{
    struct renderer renderer;

    // For debugging physics
    struct physics_frame frames[MAX_PHYSICS_FRAMES];
    b32 paused;
    u32 frame_current;
    u32 frame_max;
    u32 frame_min;
};
