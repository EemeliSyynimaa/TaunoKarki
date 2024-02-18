struct renderer
{
    u32 shader;
    struct m4 view_projection;
};

struct scene_physics
{
    struct renderer renderer;

    // Todo: create render buffers (vao, vbo, ibo) for primitives
    // (circle, aabb etc.)
};
