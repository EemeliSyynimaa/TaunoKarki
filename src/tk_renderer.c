#define MAX_CIRCLE_VERTICES 128

void render_vertices(struct renderer* renderer, u8* vertices, u32 num_vertices,
    u32 vertex_size, u32* indices, u32 num_indices, struct v4 color)
{
    // Todo: use pre-created render buffers and update sub data only
    u32 vao;
    u32 vbo;
    u32 ibo;

    api.gl.glGenVertexArrays(1, &vao);
    api.gl.glBindVertexArray(vao);

    api.gl.glGenBuffers(1, &vbo);
    api.gl.glBindBuffer(GL_ARRAY_BUFFER, vbo);
    api.gl.glBufferData(GL_ARRAY_BUFFER, num_vertices * vertex_size, vertices,
        GL_DYNAMIC_DRAW);

    api.gl.glGenBuffers(1, &ibo);
    api.gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    api.gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(u32),
        indices, GL_DYNAMIC_DRAW);

    api.gl.glEnableVertexAttribArray(0);
    api.gl.glEnableVertexAttribArray(1);
    api.gl.glEnableVertexAttribArray(2);
    api.gl.glEnableVertexAttribArray(3);

    api.gl.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)OFFSET_OF(struct vertex, position));
    api.gl.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)OFFSET_OF(struct vertex, uv));
    api.gl.glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)OFFSET_OF(struct vertex, normal));
    api.gl.glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)OFFSET_OF(struct vertex, color));

    api.gl.glUseProgram(renderer->shader);

    u32 uniform_mvp = api.gl.glGetUniformLocation(renderer->shader, "MVP");
    u32 uniform_color = api.gl.glGetUniformLocation(renderer->shader,
        "uniform_color");

    api.gl.glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE,
        (GLfloat*)&renderer->view_projection);
    api.gl.glUniform4fv(uniform_color, 1, (GLfloat*)&color);

    api.gl.glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, NULL);

    api.gl.glUseProgram(0);
    api.gl.glBindVertexArray(0);
}

void render_circle(struct renderer* renderer, f32 x, f32 y, f32 radius,
    struct v4 color, u32 num_fans)
{
    // Todo: just experimental code... :-)
    u32 num_vertices = num_fans + 1;
    u32 num_indices = num_fans * 3;

    struct vertex vertices[MAX_CIRCLE_VERTICES] = { 0 };
    u32 indices[MAX_CIRCLE_VERTICES * 3] = { 0 };

    f32 step = (F32_PI * 2) / num_fans;

    vertices[0].position.x = x;
    vertices[0].position.y = y;
    vertices[0].color = color;

    for (u32 i = 0; i < num_fans && i < MAX_CIRCLE_VERTICES; i++)
    {
        u32 index = i + 1;
        f32 dx = f32_cos(step * index);
        f32 dy = f32_sin(step * index);

        vertices[index].position.x = x + radius * dx;
        vertices[index].position.y = y + radius * dy;
        vertices[index].color = color;

        indices[i * 3 + 0] = 0;
        indices[i * 3 + 1] = index;
        indices[i * 3 + 2] = (index % num_fans) + 1;
    }

    render_vertices(renderer, (u8*)vertices, num_vertices,
        sizeof(struct vertex), indices, num_indices, color);
}

void render_aabb(struct renderer* renderer, f32 x, f32 y, f32 half_width,
    f32 half_height, struct v4 color)
{
    // Todo: just experimental code... :-)
    struct vertex vertices[] =
    {
        { { x - half_width, y - half_height, 0 }, v2_zero, v3_zero, color },
        { { x + half_width, y + half_height, 0 }, v2_zero, v3_zero, color },
        { { x - half_width, y + half_height, 0 }, v2_zero, v3_zero, color },
        { { x + half_width, y - half_height, 0 }, v2_zero, v3_zero, color },
    };

    u32 indices[6] = { 0, 1, 2, 0, 3, 1 };

    render_vertices(renderer, (u8*)vertices, ARRAY_SIZE(vertices),
        sizeof(struct vertex), indices, ARRAY_SIZE(indices), color);
}
