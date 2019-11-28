
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"

struct Vertex
{
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
};

struct cube_renderer_t
{
    u32 vao;
    u32 vbo;
    u32 ibo;
    u32 program;
    u32 num_vertices;
    u32 num_indices;
    s32 uniform_mvp;
    s32 uniform_texture;
    void* vertices;
    void* indices;
};

void system_cube_renderer_init(cube_renderer_t* state)
{
    glGenVertexArrays(1, &state->vao);
    glBindVertexArray(state->vao);

    glGenBuffers(1, &state->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, state->vbo);
    glBufferData(GL_ARRAY_BUFFER, state->num_vertices * sizeof(Vertex), state->vertices, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &state->ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, state->num_indices * sizeof(u32), state->indices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glUseProgram(state->program);

    state->uniform_mvp = glGetUniformLocation(state->program, "MVP");
    state->uniform_texture = glGetUniformLocation(state->program, "texture");

    glUseProgram(0);
}

void system_cube_renderer_update(cube_renderer_t* state, glm::mat4* view, glm::mat4* projection)
{
    glBindVertexArray(state->vao);

    for (u32 i = 0; i < num_cubes; i++)
    {
        glBindVertexArray(state->vao);

        glUseProgram(state->program);

        glm::mat4 transform = glm::translate(transforms[i].position);
        glm::mat4 rotation = glm::rotate(transforms[i].angle, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 scale = glm::scale(glm::vec3(0.5f, 0.5f, 0.75f));

        glm::mat4 model = transform * rotation * scale;

        glm::mat4 mvp = *projection * *view * model;

        glUniform1i(state->uniform_texture, 0);
        glUniformMatrix4fv(state->uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cubes[i].texture);

        glDrawElements(GL_TRIANGLES, state->num_indices, GL_UNSIGNED_INT, NULL);

        glUseProgram(0);
    }
}