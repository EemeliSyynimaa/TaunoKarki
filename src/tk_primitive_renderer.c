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

void mesh_render(struct mesh_render_info* info, struct m4* mvp)
{
    api.gl.glBindVertexArray(info->mesh->vao);

    api.gl.glUseProgram(info->shader);

    u32 uniform_mvp = api.gl.glGetUniformLocation(info->shader, "MVP");
    u32 uniform_texture = api.gl.glGetUniformLocation(info->shader, "texture");
    u32 uniform_color = api.gl.glGetUniformLocation(info->shader,
        "uniform_color");

    api.gl.glUniform1i(uniform_texture, 0);
    api.gl.glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, (GLfloat*)mvp);
    api.gl.glUniform4fv(uniform_color, 1, (GLfloat*)&info->color);

    api.gl.glActiveTexture(GL_TEXTURE0);
    api.gl.glBindTexture(GL_TEXTURE_2D, info->texture);

    api.gl.glDrawElements(GL_TRIANGLES, info->mesh->num_indices,
        GL_UNSIGNED_INT, NULL);

    api.gl.glUseProgram(0);
    api.gl.glBindVertexArray(0);
}

void line_render(struct mesh_render_info* info, struct v2 start, struct v2 end,
    f32 depth, f32 thickness, struct m4 projection, struct m4 view)
{
    struct v2 direction = v2_direction(start, end);

    f32 length = v2_length(direction) * 0.5f;
    f32 angle = f32_atan(direction.x, direction.y);

    struct m4 transform = m4_translate(
        start.x + direction.x * 0.5f,
        start.y + direction.y * 0.5f,
        depth);

    struct m4 rotation = m4_rotate_z(-angle);
    struct m4 scale = m4_scale_xyz(thickness, length, 0.01f);

    struct m4 model = m4_mul_m4(scale, rotation);
    model = m4_mul_m4(model, transform);

    struct m4 mvp = m4_mul_m4(model, view);
    mvp = m4_mul_m4(mvp, projection);

    mesh_render(info, &mvp);
}

void gui_rect_render(struct mesh_render_info* info, f32 x, f32 y, f32 width,
    f32 height, f32 angle, struct m4 projection)
{
    f32 half_width = width * 0.5f;
    f32 half_height = height * 0.5f;
    struct m4 transform = m4_translate(x + half_width, y + half_height, 0.0f);
    struct m4 rotation = m4_rotate_z(angle);
    struct m4 scale = m4_scale_xyz(half_width, half_height, 1.0f);

    struct m4 model = m4_mul_m4(scale, rotation);
    model = m4_mul_m4(model, transform);

    struct m4 mp = m4_mul_m4(model, projection);

    mesh_render(info, &mp);
}

void health_bar_render(struct mesh_render_info* info, struct camera* camera,
    struct v2 position, f32 health, f32 health_max)
{
    f32 bar_length_max = 70.0f;
    f32 bar_length = health / health_max * bar_length_max;

    struct v2 screen_pos = calculate_screen_pos(position.x, position.y + 0.5f,
        0.5f, camera);

    f32 x = screen_pos.x - bar_length_max * 0.5f;
    f32 y = screen_pos.y + 12.0f;
    f32 width = bar_length;
    f32 height = 10.0f;
    f32 angle = 0.0f;

    gui_rect_render(info, x, y, width, height, angle, camera->ortho);
}

void ammo_bar_render(struct mesh_render_info* info, struct camera* camera,
    struct v2 position, f32 ammo, f32 ammo_max)
{
    f32 bar_max_size = 20;

    if (ammo_max > bar_max_size)
    {
        ammo *= bar_max_size / ammo_max;
        ammo_max = bar_max_size;
    }

    f32 bar_length_max = 70.0f;
    f32 bar_length = bar_length_max / ammo_max;

    struct v2 screen_pos = calculate_screen_pos(position.x, position.y + 0.5f,
        0.5f, camera);

    f32 x = screen_pos.x - bar_length_max * 0.5f;
    f32 y = screen_pos.y;
    f32 width = bar_length - 1.0f;
    f32 height = 10.0f;
    f32 angle = 0.0f;

    for (u32 i = 0; i < ammo; i++, x += bar_length)
    {
        gui_rect_render(info, x, y, width, height, angle, camera->ortho);
    }
}

void weapon_level_bar_render(struct mesh_render_info* info,
    struct camera* camera, struct v2 position, u32 level, u32 level_max)
{
    f32 bar_length_max = 70.0f;
    f32 bar_length = bar_length_max / level_max;

    struct v2 screen_pos = calculate_screen_pos(position.x, position.y + 0.5f,
        0.5f, camera);

    f32 x = screen_pos.x - bar_length_max * 0.5f;
    f32 y = screen_pos.y - 12.0f;
    f32 width = bar_length - 1.0f;
    f32 height = 10.0f;
    f32 angle = 0.0f;

    for (u32 i = 0; i < level; i++, x += bar_length)
    {
        gui_rect_render(info, x, y, width, height, angle, camera->ortho);
    }
}

void sphere_render(struct mesh_render_info* info, struct v2 position,
    f32 radius, f32 depth, struct m4 projection, struct m4 view)
{
    struct m4 transform = m4_translate(position.x, position.y, depth);
    struct m4 rotation = m4_rotate_z(0);
    struct m4 scale = m4_scale_xyz(radius, radius, radius);

    struct m4 model = m4_mul_m4(scale, rotation);
    model = m4_mul_m4(model, transform);

    struct m4 mvp = m4_mul_m4(model, view);
    mvp = m4_mul_m4(mvp, projection);

    mesh_render(info, &mvp);
}

void triangle_render(struct mesh_render_info* info, struct v2 a, struct v2 b,
    struct v2 c, f32 depth, struct m4 projection, struct m4 view)
{
    // Todo: clean
    u32 vao;
    u32 vbo;
    u32 ibo;
    u32 num_vertices = 3;
    u32 num_indices = 3;
    u32 indices[] = { 0, 1, 2 };
    u32 texture = 0;
    struct vertex vertices[] =
    {
        {{ a.x, a.y, depth }, v2_zero, v3_zero, info->color },
        {{ b.x, b.y, depth }, v2_zero, v3_zero, info->color },
        {{ c.x, c.y, depth }, v2_zero, v3_zero, info->color }
    };

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

    api.gl.glUseProgram(info->shader);

    u32 uniform_mvp = api.gl.glGetUniformLocation(info->shader, "MVP");
    u32 uniform_texture = api.gl.glGetUniformLocation(info->shader,
        "texture");
    u32 uniform_color = api.gl.glGetUniformLocation(info->shader,
        "uniform_color");

    struct m4 vp = m4_mul_m4(view, projection);

    api.gl.glUniform1i(uniform_texture, 0);
    api.gl.glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, (GLfloat*)&vp);
    api.gl.glUniform4fv(uniform_color, 1, (GLfloat*)&info->color);

    api.gl.glActiveTexture(GL_TEXTURE0);
    api.gl.glBindTexture(GL_TEXTURE_2D, texture);

    api.gl.glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, NULL);

    api.gl.glUseProgram(0);
    api.gl.glBindVertexArray(0);
}
