#include <cassert>
#include <iostream>
#include <algorithm>
#include "locator.h"

typedef struct game_player
{
    f32 x;
    f32 y;
    f32 angle;
} game_player;

typedef struct game_state
{
    AssetManager assets;
    game_player player;
    Tilemap* tilemap;
    Camera* camera;
    GameObjectManager* game_object_manager;
    f32 accumulator;
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
} game_state;

game_state state;

void player_renderer_init()
{
    glGenVertexArrays(1, &state.vao);
    glBindVertexArray(state.vao);

    glGenBuffers(1, &state.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, state.vbo);
    glBufferData(GL_ARRAY_BUFFER, state.num_vertices * sizeof(Vertex), state.vertices, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &state.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, state.num_indices * sizeof(u32), state.indices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glUseProgram(state.program);

    state.uniform_mvp = glGetUniformLocation(state.program, "MVP");
    state.uniform_texture = glGetUniformLocation(state.program, "texture");

    glUseProgram(0);
}

void player_renderer_draw()
{
    glBindVertexArray(state.vao);

    glUseProgram(state.program);

    glm::mat4 transform = glm::translate(glm::vec3(state.player.x, state.player.y, 0.0f));
    glm::mat4 rotation = glm::rotate(state.player.angle, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 scale = glm::scale(glm::vec3(0.5f, 0.5f, 0.75f));

    glm::mat4 model = transform * rotation * scale;

    glm::mat4 mvp = state.camera->getPerspectiveMatrix() * state.camera->getViewMatrix() * model;

    glUniform1i(state.uniform_texture, 0);
    glUniformMatrix4fv(state.uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, state.assets.playerTexture->getID());

    glDrawElements(GL_TRIANGLES, state.num_indices, GL_UNSIGNED_INT, NULL);

    glUseProgram(0);
}

void player_move(game_input* input)
{
    f32 velocity_x = 0.0f;
    f32 velocity_y = 0.0f;
    f32 move_speed = GLOBALS::PLAYER_SPEED;

    if (input->move_left.key_down)
    {
        velocity_x -= move_speed;
    }
    
    if (input->move_right.key_down)
    {
        velocity_x += move_speed;
    }

    if (input->move_up.key_down)
    {
        velocity_y += move_speed;
    }
    
    if (input->move_down.key_down)
    {
        velocity_y -= move_speed;
    }

    state.player.x += velocity_x;
    state.player.y += velocity_y;

    f32 mouse_x = (state.camera->getWidth() / 2.0f - input->mouse_x) * -1;
    f32 mouse_y = (state.camera->getHeight() / 2.0f - input->mouse_y);

    state.player.angle = glm::atan(mouse_y, mouse_x); 
}

void init_game(s32 screen_width, s32 screen_height)
{

    s32 version_major = 0;
    s32 version_minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &version_major);
    glGetIntegerv(GL_MINOR_VERSION, &version_minor);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    fprintf(stderr, "OpenGL %i.%i\n", version_major, version_minor);

    Locator::init();
    Locator::provideAssetManager(&state.assets);

    state.assets.loadAssets();

    state.camera = new Camera();
    state.game_object_manager = new GameObjectManager(
        *Locator::getAssetManager(), *state.camera);

    std::random_device randomDevice;
    std::default_random_engine randomGenerator(randomDevice());

    state.camera->createNewPerspectiveMatrix(60.0f, (float)screen_width, 
        (float)screen_height, 0.1f, 100.0f);
    state.camera->createNewOrthographicMatrix((float)screen_width,
        (float)screen_height);
    state.camera->setPosition(glm::vec3(0.0f, 0.0f, 20.0f));
    state.camera->setOffset(0.0f, -7.5f, 0.0f);

    s32 level = 3;

    for (;;)
    {
        state.tilemap = new Tilemap(glm::vec3(0.0f), 
            *Locator::getAssetManager(), *state.camera);
        state.tilemap->generate(7 + level * 4, 7 + level * 4);

        if (state.tilemap->getNumberOfStartingPositions() > 1)
        {
            break;
        }
        else 
        {
            delete state.tilemap;
        }
    }

    glm::vec3 position = state.tilemap->getStartingPosition();

    state.player.x = position.x;
    state.player.y = position.y;
    state.player.angle = 0;

    state.program = state.assets.shaderProgram->getID();
    state.num_vertices = state.assets.cubeMesh->getVertices().size();
    state.num_indices = state.assets.cubeMesh->getIndices().size();
    state.vertices = state.assets.cubeMesh->getVertices().data();
    state.indices = state.assets.cubeMesh->getIndices().data();

    player_renderer_init();

    state.camera->follow(glm::vec2(position.x, position.y));
    
    while (state.tilemap->getNumberOfStartingPositions() > 0)
    {
        state.game_object_manager->createEnemy(
            state.tilemap->getStartingPosition(), level, 
            state.tilemap);
    }

    state.game_object_manager->createPlayerAmmoBar(
        glm::vec3(10.0f, -7.5f, 5.0f), glm::vec3(9.0f, 0.5f, 0.5f));
    state.game_object_manager->createPlayerHealthBar(
        glm::vec3(-10.0f, -7.5f, 5.0f), glm::vec3(9.0f, 0.5f, 0.5f));
}

void update_game(game_input* input)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    f32 step = 1.0f / 60.0f;
    state.accumulator += input->delta_time;

    while (state.accumulator >= step)
    {
        state.accumulator -= step;
        state.game_object_manager->update(input);

        player_move(input);
    }

    state.game_object_manager->interpolate(state.accumulator / step);
    state.camera->follow({state.player.x, state.player.y});
    state.tilemap->draw();
    state.game_object_manager->draw();
    player_renderer_draw();
}