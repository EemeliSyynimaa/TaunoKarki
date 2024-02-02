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

struct mesh_render_info
{
    struct v4 color;
    struct mesh* mesh;
    u32 texture;
    u32 shader;
};
