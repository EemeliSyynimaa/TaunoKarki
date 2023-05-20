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

void cube_renderer_init(struct cube_renderer* renderer, u32 shader, u32 texture)
{
    renderer->shader = shader;
    renderer->texture = texture;

    struct cube_vertex_data vertices[] =
    {
        // Top right
        {
            { 1.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f },
            { 0.0f, 1.0f }
        },
        // Top left
        {
            { -1.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f },
            { 0.0f, 0.0f }
        },
        // Bottom left
        {
            { -1.0f, -1.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f },
            { 1.0f, 0.0f }
        },
        // Bottom right
        {
            { 1.0f, -1.0f, 1.0f },
            { 0.0f, 0.0f, 1.0f },
            { 1.0f, 1.0f }
        },
        // Top right
        {
            { -1.0f, 1.0f, -1.0f },
            { 0.0f, 0.0f, -1.0f },
            { 0.0f, 1.0f }
        },
        // Top left
        {
            { 1.0f, 1.0f, -1.0f },
            { 0.0f, 0.0f, -1.0f },
            { 0.0f, 0.0f }
        },
        // Bottom left
        {
            { 1.0f, -1.0f, -1.0f },
            { 0.0f, 0.0f, -1.0f },
            { 1.0f, 0.0f }
        },
        // Bottom right
        {
            { -1.0f, -1.0f, -1.0f },
            { 0.0f, 0.0f, -1.0f },
            { 1.0f, 1.0f }
        },
        // Top right
        {
            { -1.0f, -1.0f, 1.0f },
            { -1.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f }
        },
        // Top left
        {
            { -1.0f, 1.0f, 1.0f },
            { -1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f }
        },
        // Bottom left
        {
            { -1.0f, 1.0f, -1.0f },
            { -1.0f, 0.0f, 0.0f },
            { 1.0f, 0.0f }
        },
        // Bottom right
        {
            { -1.0f, -1.0f, -1.0f },
            { -1.0f, 0.0f, 0.0f },
            { 1.0f, 1.0f }
        },
        // Top right
        {
            { 1.0f, 1.0f, 1.0f },
            { 1.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f }
        },
        // Top left
        {
            { 1.0f, -1.0f, 1.0f },
            { 1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f }
        },
        // Bottom left
        {
            { 1.0f, -1.0f, -1.0f },
            { 1.0f, 0.0f, 0.0f },
            { 1.0f, 0.0f }
        },
        // Bottom right
        {
            { 1.0f, 1.0f, -1.0f },
            { 1.0f, 0.0f, 0.0f },
            { 1.0f, 1.0f }
        },
        // Top right
        {
            { -1.0f, 1.0f, 1.0f },
            { 0.0f, 1.0f, 0.0f },
            { 0.0f, 1.0f }
        },
        // Top left
        {
            { 1.0f, 1.0f, 1.0f },
            { 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f }
        },
        // Bottom left
        {
            { 1.0f, 1.0f, -1.0f },
            { 0.0f, 1.0f, 0.0f },
            { 1.0f, 0.0f }
        },
        // Bottom right
        {
            { -1.0f, 1.0f, -1.0f },
            { 0.0f, 1.0f, 0.0f },
            { 1.0f, 1.0f }
        },
        // Top right
        {
            { 1.0f, -1.0f, 1.0f },
            { 0.0f, -1.0f, 0.0f },
            { 0.0f, 1.0f }
        },
        // Top left
        {
            { -1.0f, -1.0f, 1.0f },
            { 0.0f, -1.0f, 0.0f },
            { 0.0f, 0.0f }
        },
        // Bottom left
        {
            { -1.0f, -1.0f, -1.0f },
            { 0.0f, -1.0f, 0.0f },
            { 1.0f, 0.0f }
        },
        // Bottom right
        {
            { 1.0f, -1.0f, -1.0f },
            { 0.0f, -1.0f, 0.0f },
            { 1.0f, 1.0f }
        }
    };

    u32 indices[] =
    {
         0,  1,  2,  0,  2,  3, // Top
         4,  5,  6,  4,  6,  7, // Bottom
         8,  9, 10,  8, 10, 11, // Left
        12, 13, 14, 12, 14, 15, // Right
        16, 17, 18, 16, 18, 19, // Up
        20, 21, 22, 20, 22, 23, // Down
    };

    renderer->num_indices = 36;

    api.gl.glGenVertexArrays(1, &renderer->vao);
    api.gl.glBindVertexArray(renderer->vao);

    api.gl.glGenBuffers(1, &renderer->vbo_vertices);
    api.gl.glGenBuffers(1, &renderer->vbo_cubes);
    api.gl.glGenBuffers(1, &renderer->ibo);
    api.gl.glGenBuffers(1, &renderer->ubo);

    api.gl.glEnableVertexAttribArray(0);
    api.gl.glEnableVertexAttribArray(1);
    api.gl.glEnableVertexAttribArray(2);
    api.gl.glEnableVertexAttribArray(3);
    api.gl.glEnableVertexAttribArray(4);
    api.gl.glEnableVertexAttribArray(5);
    api.gl.glEnableVertexAttribArray(6);
    api.gl.glEnableVertexAttribArray(7);
    api.gl.glEnableVertexAttribArray(8);
    api.gl.glEnableVertexAttribArray(9);
    api.gl.glEnableVertexAttribArray(10);
    api.gl.glEnableVertexAttribArray(11);
    api.gl.glEnableVertexAttribArray(12);

    api.gl.glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_vertices);
    api.gl.glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
        GL_STATIC_DRAW);
    api.gl.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct cube_vertex_data), (void*)0);
    api.gl.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct cube_vertex_data), (void*)12);
    api.gl.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
        sizeof(struct cube_vertex_data), (void*)24);

    api.gl.glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_cubes);
    api.gl.glBufferData(GL_ARRAY_BUFFER, sizeof(renderer->cubes),
        renderer->cubes, GL_DYNAMIC_DRAW);
    api.gl.glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct cube_data), (void*)0);
    api.gl.glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct cube_data), (void*)sizeof(struct v4));
    api.gl.glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct cube_data), (void*)(sizeof(struct v4) * 2));
    api.gl.glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct cube_data), (void*)(sizeof(struct v4) * 3));
    api.gl.glVertexAttribIPointer(7, 3, GL_UNSIGNED_BYTE,
        sizeof(struct cube_data), (void*)(sizeof(struct v4) * 4));
    api.gl.glVertexAttribIPointer(8, 3, GL_UNSIGNED_BYTE,
        sizeof(struct cube_data), (void*)(sizeof(struct v4) * 4 + 3));
    api.gl.glVertexAttribIPointer(9, 3, GL_UNSIGNED_BYTE,
        sizeof(struct cube_data), (void*)(sizeof(struct v4) * 4 + 6));
    api.gl.glVertexAttribIPointer(10, 3, GL_UNSIGNED_BYTE,
        sizeof(struct cube_data), (void*)(sizeof(struct v4) * 4 + 9));
    api.gl.glVertexAttribIPointer(11, 3, GL_UNSIGNED_BYTE,
        sizeof(struct cube_data), (void*)(sizeof(struct v4) * 4 + 12));
    api.gl.glVertexAttribIPointer(12, 3, GL_UNSIGNED_BYTE,
        sizeof(struct cube_data), (void*)(sizeof(struct v4) * 4 + 15));

    api.gl.glVertexAttribDivisor(3, 1);
    api.gl.glVertexAttribDivisor(4, 1);
    api.gl.glVertexAttribDivisor(5, 1);
    api.gl.glVertexAttribDivisor(6, 1);
    api.gl.glVertexAttribDivisor(7, 1);
    api.gl.glVertexAttribDivisor(8, 1);
    api.gl.glVertexAttribDivisor(9, 1);
    api.gl.glVertexAttribDivisor(10, 1);
    api.gl.glVertexAttribDivisor(11, 1);
    api.gl.glVertexAttribDivisor(12, 1);

    api.gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
    api.gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        renderer->num_indices * sizeof(u32), indices, GL_STATIC_DRAW);

    u32 uniform_block_index = api.gl.glGetUniformBlockIndex(renderer->shader,
        "uniform_colors");
    api.gl.glUniformBlockBinding(renderer->shader, uniform_block_index, 0);

    api.gl.glBindBuffer(GL_UNIFORM_BUFFER, renderer->ubo);
    api.gl.glBufferData(GL_UNIFORM_BUFFER, sizeof(renderer->colors), NULL,
        GL_STATIC_DRAW);

    api.gl.glBindBufferRange(GL_UNIFORM_BUFFER, 0, renderer->ubo, 0,
        sizeof(renderer->colors));

    api.gl.glBindVertexArray(0);

    renderer->initialized = !gl_check_error("cube_renderer_init");
}

void cube_renderer_add(struct cube_renderer* renderer, struct cube_data* data)
{
    if (renderer->initialized)
    {
        if (renderer->num_cubes < MAX_CUBES)
        {
            renderer->cubes[renderer->num_cubes++] = *data;
        }
    }
}

void cube_renderer_flush(struct cube_renderer* renderer, struct m4* view,
    struct m4* projection)
{
    if (renderer->initialized && renderer->num_cubes)
    {
        api.gl.glBindVertexArray(renderer->vao);
        api.gl.glUseProgram(renderer->shader);

        u32 uniform_texture = api.gl.glGetUniformLocation(renderer->shader,
            "uniform_texture");
        u32 uniform_vp = api.gl.glGetUniformLocation(renderer->shader,
            "uniform_vp");

        if (renderer->update_color_data)
        {
            api.gl.glBufferData(GL_UNIFORM_BUFFER, sizeof(renderer->colors),
                &renderer->colors, GL_STATIC_DRAW);

            renderer->update_color_data = false;
        }

        struct m4 vp = m4_mul_m4(*view, *projection);

        api.gl.glUniform1i(uniform_texture, 0);
        api.gl.glUniformMatrix4fv(uniform_vp, 1, GL_FALSE, (GLfloat*)&vp);

        api.gl.glActiveTexture(GL_TEXTURE0);
        api.gl.glBindTexture(GL_TEXTURE_2D_ARRAY, renderer->texture);

        api.gl.glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_cubes);
        api.gl.glBufferSubData(GL_ARRAY_BUFFER, 0,
            renderer->num_cubes * sizeof(struct cube_data), renderer->cubes);

        api.gl.glDrawElementsInstanced(GL_TRIANGLES, renderer->num_indices,
            GL_UNSIGNED_INT, NULL, renderer->num_cubes);

        api.gl.glUseProgram(0);
        api.gl.glBindVertexArray(0);

        renderer->num_cubes = 0;
    }
}

s32 cube_renderer_color_add(struct cube_renderer* renderer, struct v4 color)
{
    s32 result = -1;

    if (renderer->initialized)
    {
        // Todo: we should figure out what to do when the color buffer is full
        for (u32 i = 0; i < renderer->num_colors; i++)
        {
            if (v4_equals(renderer->colors[i], color))
            {
                result = i;
                renderer->update_color_data = true;
                break;
            }
        }

        if (result == -1 && renderer->num_colors < MAX_CUBE_COLORS)
        {
            result = renderer->num_colors++;
            renderer->colors[result] = color;
            renderer->update_color_data = true;
        }
    }

    return result;
}

void cube_data_color_update(struct cube_data* cube, u32 color)
{
    for (u32 i = 0; i < 6; i++)
    {
        cube->faces[i].color = color;
    }
}
