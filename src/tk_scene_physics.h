// Todo: these structs are temporarily named to avoid conflicts

enum
{
    TK_ENTITY_ALIVE = 1 << 0,
    TK_ENTITY_RENDERABLE = 1 << 1
};

enum
{
    TK_RENDER_TYPE_CIRCLE,
    TK_RENDER_TYPE_AABB
};

struct tk_entity
{
    u64 flags;

    f32 x;
    f32 y;

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

#define TK_MAX_ENTITIES 256

struct scene_physics
{
    struct renderer renderer;

    struct tk_entity entities[TK_MAX_ENTITIES];
    u32 num_entities;
};
