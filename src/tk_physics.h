#define MAX_CIRCLES 64
#define MAX_CONTACTS 64
#define MAX_COLLIDERS 8
#define MAX_BODIES 512

struct collider
{
    u32 type;

    u32 tag;
    u32 collidesWith;

    struct rigid_body* body;

    union
    {
        // Circle collider
        struct
        {
            struct v2 position;
            f32 radius;
            f32 unused;
        } circle;

        // Line collider
        struct
        {
            struct v2 a;
            struct v2 b;
        } line;

        // Rect collider
        struct
        {
            struct v2 position;
            f32 half_width;
            f32 half_height;
        } rect;

        // No collider
        struct
        {
            u32 unused[4];
        } none;
    };
};

enum
{
    COLLIDER_NONE,
    COLLIDER_CIRCLE,
    COLLIDER_LINE,
    COLLIDER_RECT,
    COLLIDER_COUNT
};

enum
{
    RIGID_BODY_STATIC,
    RIGID_BODY_DYNAMIC
};

struct rigid_body
{
    struct contact* contact;
    struct collider colliders[MAX_COLLIDERS];
    struct v2 position;
    struct v2 velocity;
    struct v2 acceleration;
    struct v2 move_delta;
    struct entity* owner;
    u32 type;
    u32 num_colliders;
    f32 friction;
    f32 mass;
    f32 angle;
    b32 alive;
    b32 trigger;
};

struct contact
{
    struct collider* a;
    struct collider* b;
    struct v2 position;
    f32 t;

    b32 active;
};

struct physics_world
{
    struct rigid_body bodies[MAX_BODIES];
    struct contact contacts[MAX_CONTACTS];
    u32 num_contacts;
};
