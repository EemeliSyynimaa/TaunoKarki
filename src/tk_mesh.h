struct vertex
{
    struct v3 position;
    struct v2 uv;
    struct v3 normal;
    struct v4 color;
};

struct mesh
{
    u32 vao;
    u32 vbo;
    u32 ibo;
    u32 num_indices;
};

// Todo: this is not really a particle thing but a primitive
// Todo: create a primitive renderer
struct particle_line
{
    struct v2 start;
    struct v2 end;
    struct v4 color_start;
    struct v4 color_end;
    struct v4 color_current;
    f32 time_start;
    f32 time_current;
    b32 alive;
};

struct mesh_render_info
{
    struct v4 color;
    struct mesh* mesh;
    u32 texture;
    u32 shader;
};
