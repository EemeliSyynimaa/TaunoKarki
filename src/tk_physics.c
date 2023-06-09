struct line_segment
{
    struct v2 start;
    struct v2 end;
    u32 type;
};

#define MAX_CIRCLES 64
#define MAX_CONTACTS 64

struct contact
{
    struct rigid_body* a;
    struct rigid_body* b;
    struct line_segment* line;
    struct v2 position;
    f32 t;
};

struct collider_circle
{
    struct v2 position;
    f32 radius;
};

struct collider_line
{
    struct v2 a;
    struct v2 b;
};

struct circle
{
    struct contact* contact;
    struct v2 position;
    struct v2 velocity;
    struct v2 move_delta;
    struct v2 acceleration;
    struct v2 target;
    f32 radius;
    f32 mass;
    b32 dynamic;
};

struct rigid_body
{
    struct contact* contact;
    struct v2 position;
    struct v2 velocity;
    struct v2 acceleration;
    struct v2 move_delta;
    f32 friction;
    f32 radius;
    f32 mass;
    f32 angle;
    b32 alive;
    b32 bullet;
    b32 trigger;
};

#define MAX_STATICS 1024
#define MAX_BODIES 512

struct physics_world
{
    struct rigid_body bodies[MAX_BODIES];
    struct line_segment* walls;
    struct contact contacts[MAX_CONTACTS];
    u32 num_walls;
    u32 num_contacts;
};

struct rigid_body* rigid_body_get(struct physics_world* world)
{
    struct rigid_body* result = NULL;

    for (u32 i = 0; i < MAX_BODIES; i++)
    {
        if (!world->bodies[i].alive)
        {
            result = &world->bodies[i];
            *result = (struct rigid_body){ 0 };
            result->alive = true;
            result->mass = 1.0f;

            break;
        }
    }

    return result;
}

void rigid_body_free(struct rigid_body* body)
{
    body->alive = false;
}
