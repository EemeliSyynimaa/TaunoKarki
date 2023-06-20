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
    ITEM_HEALTH,
    ITEM_SHOTGUN,
    ITEM_MACHINEGUN,
    ITEM_PISTOL,
    ITEM_WEAPON_LEVEL_UP,
    ITEM_COUNT
};

enum
{
    WEAPON_NONE = 0,
    WEAPON_SHOTGUN,
    WEAPON_MACHINEGUN,
    WEAPON_PISTOL,
    WEAPON_COUNT
};

u32 ITEAM_HEALTH_AMOUNT = 25;

f32 PROJECTILE_RADIUS = 0.035f;
f32 PROJECTILE_SPEED  = 50.0f;

f32 FRICTION  = 10.0f;

u32 WEAPON_LEVEL_MAX = 10;

#define MAX_BULLETS 64
#define MAX_ENEMIES 64
#define MAX_ITEMS 256
#define MAX_WALL_CORNERS 512
#define MAX_WALL_FACES 512
#define MAX_GUN_SHOTS 64
#define MAX_PATH 256
// Todo: this really depends on the level size
#define MAX_NODES 1024
