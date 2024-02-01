#define MAX_LEVEL_SIZE 256

f32 WALL_SIZE = 1.0f;

u32 TILE_NOTHING = 0;
u32 TILE_WALL    = 1;
u32 TILE_FLOOR   = 2;
u32 TILE_DOOR    = 3;
u32 TILE_START   = 4;

struct level
{
    u8 tile_sprites[MAX_LEVEL_SIZE*MAX_LEVEL_SIZE];
    u8 tile_types[MAX_LEVEL_SIZE*MAX_LEVEL_SIZE];
    u32 width;
    u32 height;
    struct v2 start_pos;
    struct cube_data elevator_light;
    struct mesh_render_info wall_info;
};
