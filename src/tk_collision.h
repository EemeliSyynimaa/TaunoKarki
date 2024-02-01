#define MAX_STATICS 1024

struct line_segment
{
    struct v2 start;
    struct v2 end;
    u32 type;
};

struct collision_map
{
    struct line_segment statics[MAX_STATICS];
    struct line_segment dynamics[MAX_STATICS];
    u32 num_statics;
    u32 num_dynamics;
};

u32 COLLISION_NONE          = 0;
u32 COLLISION_WALL          = 1;
u32 COLLISION_PLAYER        = 2;
u32 COLLISION_PLAYER_HITBOX = 4;
u32 COLLISION_ENEMY         = 8;
u32 COLLISION_ENEMY_HITBOX  = 16;
u32 COLLISION_BULLET_PLAYER = 32;
u32 COLLISION_BULLET_ENEMY  = 64;
u32 COLLISION_ITEM          = 128;
u32 COLLISION_DYNAMIC       = 254;
u32 COLLISION_ALL           = 255;
