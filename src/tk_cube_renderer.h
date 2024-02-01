#define MAX_CUBES 1024
#define MAX_CUBE_COLORS 2048

struct cube_vertex_data
{
    struct v3 position;
    struct v3 normal;
    struct v2 uv;
};

struct cube_face
{
    u8 texture;
    u8 rotation;
    u8 color;
};

struct cube_data
{
    struct m4 model;
    struct cube_face faces[6];
};

struct cube_renderer
{
    struct cube_data cubes[MAX_CUBES];
    struct v4 colors[MAX_CUBE_COLORS];
    u32 vao;
    u32 vbo_vertices;
    u32 vbo_cubes;
    u32 ibo;
    u32 ubo;
    u32 num_indices;
    u32 num_cubes;
    u32 num_colors;
    u32 shader;
    u32 texture;
    b32 update_color_data;
    b32 initialized;
};
