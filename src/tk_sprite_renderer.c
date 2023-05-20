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

void sprite_renderer_init(struct sprite_renderer* renderer, u32 shader,
    u32 texture)
{
    renderer->shader = shader;
    renderer->texture = texture;

    struct sprite_vertex_data vertices[] =
    {
        // Top right
        {
            { 1.0f, 1.0f },
            { 0.0f, 1.0f }
        },
        // Top left
        {
            { -1.0f, 1.0f },
            { 0.0f, 0.0f }
        },
        // Bottom left
        {
            { -1.0f, -1.0f },
            { 1.0f, 0.0f }
        },
        // Bottom right
        {
            { 1.0f, -1.0f },
            { 1.0f, 1.0f }
        }
    };

    u32 indices[] =
    {
        0, 1, 2, 0, 2, 3
    };

    renderer->num_indices = 6;

    api.gl.glGenVertexArrays(1, &renderer->vao);
    api.gl.glBindVertexArray(renderer->vao);

    api.gl.glGenBuffers(1, &renderer->vbo_vertices);
    api.gl.glGenBuffers(1, &renderer->vbo_sprites);
    api.gl.glGenBuffers(1, &renderer->ibo);

    api.gl.glEnableVertexAttribArray(0);
    api.gl.glEnableVertexAttribArray(1);
    api.gl.glEnableVertexAttribArray(2);
    api.gl.glEnableVertexAttribArray(3);
    api.gl.glEnableVertexAttribArray(4);
    api.gl.glEnableVertexAttribArray(5);
    api.gl.glEnableVertexAttribArray(6);
    api.gl.glEnableVertexAttribArray(7);

    api.gl.glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_vertices);
    api.gl.glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
        GL_STATIC_DRAW);
    api.gl.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
        sizeof(struct sprite_vertex_data), (void*)0);
    api.gl.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        sizeof(struct sprite_vertex_data), (void*)8);

    api.gl.glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_sprites);
    api.gl.glBufferData(GL_ARRAY_BUFFER, sizeof(renderer->sprites),
        renderer->sprites, GL_DYNAMIC_DRAW);
    api.gl.glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct sprite_data), (void*)(sizeof(struct v4) * 0));
    api.gl.glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct sprite_data), (void*)(sizeof(struct v4) * 1));
    api.gl.glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct sprite_data), (void*)(sizeof(struct v4) * 2));
    api.gl.glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct sprite_data), (void*)(sizeof(struct v4) * 3));
    api.gl.glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct sprite_data), (void*)(sizeof(struct v4) * 4));
    api.gl.glVertexAttribIPointer(7, 1, GL_UNSIGNED_BYTE,
        sizeof(struct sprite_data), (void*)(sizeof(struct v4) * 5));

    api.gl.glVertexAttribDivisor(2, 1);
    api.gl.glVertexAttribDivisor(3, 1);
    api.gl.glVertexAttribDivisor(4, 1);
    api.gl.glVertexAttribDivisor(5, 1);
    api.gl.glVertexAttribDivisor(6, 1);
    api.gl.glVertexAttribDivisor(7, 1);

    api.gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
    api.gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        renderer->num_indices * sizeof(u32), indices, GL_STATIC_DRAW);

    api.gl.glBindVertexArray(0);

    renderer->initialized = !gl_check_error("sprite_renderer_init");
}

void sprite_renderer_add(struct sprite_renderer* renderer,
    struct sprite_data* data)
{
    if (renderer->initialized)
    {
        if (renderer->num_sprites < MAX_SPRITES)
        {
            renderer->sprites[renderer->num_sprites++] = *data;
        }
    }
}

void sprite_renderer_flush(struct sprite_renderer* renderer, struct m4* view,
    struct m4* projection)
{
    if (renderer->initialized && renderer->num_sprites)
    {
        api.gl.glBindVertexArray(renderer->vao);
        api.gl.glUseProgram(renderer->shader);

        u32 uniform_texture = api.gl.glGetUniformLocation(renderer->shader,
            "uniform_texture");
        u32 uniform_vp = api.gl.glGetUniformLocation(renderer->shader,
            "uniform_vp");

        struct m4 vp = m4_mul_m4(*view, *projection);

        api.gl.glUniform1i(uniform_texture, 0);
        api.gl.glUniformMatrix4fv(uniform_vp, 1, GL_FALSE, (GLfloat*)&vp);

        api.gl.glActiveTexture(GL_TEXTURE0);
        api.gl.glBindTexture(GL_TEXTURE_2D_ARRAY, renderer->texture);

        api.gl.glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_sprites);
        api.gl.glBufferSubData(GL_ARRAY_BUFFER, 0,
            renderer->num_sprites * sizeof(struct sprite_data),
            renderer->sprites);

        api.gl.glDrawElementsInstanced(GL_TRIANGLES, renderer->num_indices,
            GL_UNSIGNED_INT, NULL, renderer->num_sprites);

        api.gl.glUseProgram(0);
        api.gl.glBindVertexArray(0);

        renderer->num_sprites = 0;
    }
}
