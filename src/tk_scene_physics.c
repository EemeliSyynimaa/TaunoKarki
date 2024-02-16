// Todo: globals for now
u32 shader = 0;
struct m4 view_projection = { 0 };

#define ARRAY_SIZE(a) sizeof(a) / sizeof(a[0])
#define MAX_VERTICES 128

void render_circle(f32 x, f32 y, f32 radius)
{
    // Todo: just experimental code... :-)
    u32 vao;
    u32 vbo;
    u32 ibo;
    u32 texture = 0;
    u32 num_fans = 36;
    u32 num_vertices = num_fans + 1;
    u32 num_indices = num_fans * 3;

    struct v4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    struct vertex vertices[MAX_VERTICES] = { 0 };
    u32 indices[MAX_VERTICES * 3] = { 0 };

    f32 step = (F32_PI * 2) / num_fans;

    vertices[0].position.x = x;
    vertices[0].position.y = y;
    vertices[0].color = color;

    for (u32 i = 0; i < num_fans && i < MAX_VERTICES; i++)
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

    api.gl.glGenVertexArrays(1, &vao);
    api.gl.glBindVertexArray(vao);

    api.gl.glGenBuffers(1, &vbo);
    api.gl.glBindBuffer(GL_ARRAY_BUFFER, vbo);
    api.gl.glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(struct vertex),
        vertices, GL_DYNAMIC_DRAW);

    api.gl.glGenBuffers(1, &ibo);
    api.gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    api.gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(u32),
        indices, GL_DYNAMIC_DRAW);

    api.gl.glEnableVertexAttribArray(0);
    api.gl.glEnableVertexAttribArray(1);
    api.gl.glEnableVertexAttribArray(2);
    api.gl.glEnableVertexAttribArray(3);

    // Todo: implement offsetof
    api.gl.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)0);
    api.gl.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)12);
    api.gl.glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)20);
    api.gl.glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)32);

    api.gl.glUseProgram(shader);

    u32 uniform_mvp = api.gl.glGetUniformLocation(shader, "MVP");
    u32 uniform_color = api.gl.glGetUniformLocation(shader,
        "uniform_color");

    api.gl.glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE,
        (GLfloat*)&view_projection);
    api.gl.glUniform4fv(uniform_color, 1, (GLfloat*)&color);

    api.gl.glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, NULL);

    api.gl.glUseProgram(0);
    api.gl.glBindVertexArray(0);
}

void render_aabb(f32 x, f32 y, f32 half_width, f32 half_height)
{
    // Todo: just experimental code... :-)
    u32 vao;
    u32 vbo;
    u32 ibo;
    u32 texture = 0;

    struct v4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    struct vertex vertices[] =
    {
        { { x - half_width, y - half_height, 0 }, v2_zero, v3_zero, color },
        { { x + half_width, y + half_height, 0 }, v2_zero, v3_zero, color },
        { { x - half_width, y + half_height, 0 }, v2_zero, v3_zero, color },
        { { x + half_width, y - half_height, 0 }, v2_zero, v3_zero, color },
    };

    u32 indices[6] = { 0, 1, 2, 0, 3, 1 };

    api.gl.glGenVertexArrays(1, &vao);
    api.gl.glBindVertexArray(vao);

    api.gl.glGenBuffers(1, &vbo);
    api.gl.glBindBuffer(GL_ARRAY_BUFFER, vbo);
    api.gl.glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
        GL_DYNAMIC_DRAW);

    api.gl.glGenBuffers(1, &ibo);
    api.gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    api.gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
        GL_DYNAMIC_DRAW);

    api.gl.glEnableVertexAttribArray(0);
    api.gl.glEnableVertexAttribArray(1);
    api.gl.glEnableVertexAttribArray(2);
    api.gl.glEnableVertexAttribArray(3);

    // Todo: implement offsetof
    api.gl.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)0);
    api.gl.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)12);
    api.gl.glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)20);
    api.gl.glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct vertex), (void*)32);

    api.gl.glUseProgram(shader);

    u32 uniform_mvp = api.gl.glGetUniformLocation(shader, "MVP");
    u32 uniform_color = api.gl.glGetUniformLocation(shader,
        "uniform_color");

    api.gl.glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE,
        (GLfloat*)&view_projection);
    api.gl.glUniform4fv(uniform_color, 1, (GLfloat*)&color);

    api.gl.glDrawElements(GL_TRIANGLES, ARRAY_SIZE(indices), GL_UNSIGNED_INT,
        NULL);

    api.gl.glUseProgram(0);
    api.gl.glBindVertexArray(0);
}

void scene_physics_init(struct scene_physics* data)
{
    // Todo: maybe this function should return the initialized struct instead
    data->circle_x = data->screen_width / 2.0f;
    data->circle_y = data->screen_height / 2.0f;
}

void scene_physics_update(struct scene_physics* data, struct game_input* input,
    f32 step)
{
    // Todo: these are only temporary here as they die after hot reload
    shader = data->shader;
    view_projection = m4_orthographic(0.0f, data->screen_width, 0.0f,
        data->screen_height, 0.0f, 1.0f);;
}

void scene_physics_render(struct scene_physics* data)
{
    render_circle(200.0f, 200.0f, 50.0f);
    render_aabb(400.0f, 200.0f, 100.0f, 50.0f);
}
