#define MAX_SPRITES 1024

struct sprite_vertex_data
{
    struct v2 position;
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
