// Todo: player, bullet, item and enemy are entities that have a rigid body
// Todo: create proper entity system
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
    struct rigid_body* body;
    struct v2 eye_position;
    struct weapon weapon;
    struct cube_data cube;
    f32 health;
    b32 alive;
    u32 item_picked;
};

struct bullet
{
    struct rigid_body* body;
    struct v4 color;
    struct v2 start;
    f32 damage;
    b32 alive;
    b32 player_owned;
};

struct item
{
    struct rigid_body* body;
    struct cube_data cube;
    u32 type;
    f32 alive;
    f32 flash_timer;
    b32 flash_hide;
};

struct enemy
{
    struct rigid_body* body;
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
    b32 player_dead;
};
