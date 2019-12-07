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

typedef struct game_bullet
{
    f32 x;
    f32 y;
    f32 velocity_x;
    f32 velocity_y;
    f32 angle;
} game_bullet;

typedef struct game_enemy
{
    f32 x;
    f32 y;
    f32 angle;
} game_enemy;

typedef struct mesh
{
    void* vertices;
    void* indices;
    u32 vao;
    u32 vbo;
    u32 ibo;    
    u32 num_vertices;
    u32 num_indices;
} mesh;

#define MAX_BULLETS 8
#define MAX_ENEMIES 4

typedef struct game_state
{
    game_player player;
    game_bullet bullets[MAX_BULLETS];
    game_enemy enemies[MAX_ENEMIES];
    mesh cube;
    mesh sphere;
    AssetManager assets;
    Tilemap* tilemap;
    Camera* camera;
    GameObjectManager* game_object_manager;
    b32 fired;
    f32 accumulator;
    u32 free_bullet;
    u32 num_enemies;
    s32 uniform_mvp;
    s32 uniform_texture;

} game_state;

game_state state;

void generate_vertex_array(mesh* mesh)
{
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    glGenBuffers(1, &mesh->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh->num_vertices * sizeof(Vertex), mesh->vertices, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &mesh->ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->num_indices * sizeof(u32), mesh->indices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}

void mesh_render(mesh* mesh, glm::mat4* mvp, u32 texture)
{
    glBindVertexArray(mesh->vao);

    u32 program = state.assets.shaderProgram->getID();

    glUseProgram(program);

    u32 uniform_mvp = glGetUniformLocation(program, "MVP");
    u32 uniform_texture = glGetUniformLocation(program, "texture");

    glUniform1i(uniform_texture, 0);
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(*mvp));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, NULL);

    glUseProgram(0);
}

void enemies_update(game_input* input)
{

}

void enemies_render()
{
    for (u32 i = 0; i < MAX_ENEMIES; i++)
    {
        game_enemy* enemy = &state.enemies[i];

        glm::mat4 transform = glm::translate(glm::vec3(enemy->x, enemy->y, 0.0f));
        glm::mat4 rotation = glm::rotate(enemy->angle, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 scale = glm::scale(glm::vec3(glm::vec3(0.5f, 0.5f, 0.75f)));

        glm::mat4 model = transform * rotation * scale;

        glm::mat4 mvp = state.camera->getPerspectiveMatrix() * state.camera->getViewMatrix() * model;

        mesh_render(&state.cube, &mvp, state.assets.enemyTexture->getID());
    }
}

void bullets_update(game_input* input)
{
    for (u32 i = 0; i < MAX_BULLETS; i++)
    {
        game_bullet* bullet = &state.bullets[i];

        bullet->x += bullet->velocity_x;
        bullet->y += bullet->velocity_y;
    }
}

void bullets_render()
{
    for (u32 i = 0; i < MAX_BULLETS; i++)
    {
        game_bullet* bullet = &state.bullets[i];

        glm::mat4 transform = glm::translate(glm::vec3(bullet->x, bullet->y, 0.0f));
        glm::mat4 rotation = glm::rotate(bullet->angle, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 scale = glm::scale(glm::vec3(GLOBALS::PROJECTILE_SIZE));

        glm::mat4 model = transform * rotation * scale;

        glm::mat4 mvp = state.camera->getPerspectiveMatrix() * state.camera->getViewMatrix() * model;

        mesh_render(&state.sphere, &mvp, state.assets.sphereTexture->getID());
    }
}

void player_update(game_input* input)
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

    if (input->shoot.key_down)
    {
        if (!state.fired)
        {
            if (++state.free_bullet == MAX_BULLETS)
            {
                state.free_bullet = 0;
            }

            game_bullet* bullet = &state.bullets[state.free_bullet];

            f32 dir_x = glm::cos(state.player.angle);
            f32 dir_y = glm::sin(state.player.angle);
            f32 speed = GLOBALS::PISTOL_BULLET_SPEED;

            bullet->x = state.player.x;
            bullet->y = state.player.y;
            bullet->velocity_x = dir_x * speed;
            bullet->velocity_y = dir_y * speed;

            state.fired = true;
        }
    }
    else
    {
        state.fired = false;
    }
}

void player_render()
{
    glm::mat4 transform = glm::translate(glm::vec3(state.player.x, state.player.y, 0.0f));
    glm::mat4 rotation = glm::rotate(state.player.angle, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 scale = glm::scale(glm::vec3(0.5f, 0.5f, 0.75f));

    glm::mat4 model = transform * rotation * scale;

    glm::mat4 mvp = state.camera->getPerspectiveMatrix() * state.camera->getViewMatrix() * model;

    mesh_render(&state.cube, &mvp, state.assets.playerTexture->getID());
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

    state.camera->createNewPerspectiveMatrix(60.0f, (float)screen_width, 
        (float)screen_height, 0.1f, 100.0f);
    state.camera->createNewOrthographicMatrix((float)screen_width,
        (float)screen_height);
    state.camera->setPosition(glm::vec3(0.0f, 0.0f, 20.0f));
    state.camera->setOffset(0.0f, 0.0f, 0.0f);

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

    state.cube.num_vertices = state.assets.cubeMesh->getVertices().size();
    state.cube.num_indices = state.assets.cubeMesh->getIndices().size();
    state.cube.vertices = state.assets.cubeMesh->getVertices().data();
    state.cube.indices = state.assets.cubeMesh->getIndices().data();

    generate_vertex_array(&state.cube);

    state.sphere.num_vertices = state.assets.sphereMesh->getVertices().size();
    state.sphere.num_indices = state.assets.sphereMesh->getIndices().size();
    state.sphere.vertices = state.assets.sphereMesh->getVertices().data();
    state.sphere.indices = state.assets.sphereMesh->getIndices().data();

    generate_vertex_array(&state.sphere);

    state.camera->follow(glm::vec2(position.x, position.y));
    
    while (state.tilemap->getNumberOfStartingPositions() > 0)
    {
        if (state.num_enemies < MAX_ENEMIES)
        {
            game_enemy* enemy = &state.enemies[state.num_enemies++];
            
            glm::vec3 position = state.tilemap->getStartingPosition();

            enemy->x = position.x;
            enemy->y = position.y;
        }
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

        player_update(input);
        enemies_update(input);
        bullets_update(input);
    }

    state.game_object_manager->interpolate(state.accumulator / step);
    state.camera->follow({state.player.x, state.player.y});
    state.tilemap->draw();
    state.game_object_manager->draw();

    player_render();
    enemies_render();
    bullets_render();
}