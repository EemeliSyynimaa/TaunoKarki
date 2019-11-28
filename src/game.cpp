#include <cassert>
#include <iostream>
#include <algorithm>
#include "locator.h"

typedef struct game_state
{
    AssetManager assets;
    cube_renderer_t cube_renderer;
    Tilemap* tilemap;
    Camera* camera;
    GameObjectManager* game_object_manager;
    b32 game_ending;
    s32 level;
    s32 player_dying_channel;
    f32 accumulator;
    // u32 vao;
} game_state;

game_state state;

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

    state.level = 3;
    state.camera = new Camera();
    state.game_object_manager = new GameObjectManager(
        *Locator::getAssetManager(), *state.camera);

    std::random_device randomDevice;
    std::default_random_engine randomGenerator(randomDevice());

    printf("GAMESCENE ALIVE - entering level %d\n ", state.level);

    state.camera->createNewPerspectiveMatrix(60.0f, (float)screen_width, 
        (float)screen_height, 0.1f, 100.0f);
    state.camera->createNewOrthographicMatrix((float)screen_width,
        (float)screen_height);
    state.camera->setPosition(glm::vec3(0.0f, 0.0f, 20.0f));
    state.camera->setOffset(0.0f, -7.5f, 0.0f);

    for (;;)
    {
        state.tilemap = new Tilemap(glm::vec3(0.0f), 
            *Locator::getAssetManager(), *state.camera);
        state.tilemap->generate(7 + state.level * 4, 7 + state.level * 4);

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

    create_player(position, state.assets.playerTexture->getID());

    cube_renderer_t* cube = &state.cube_renderer;

    cube->program = state.assets.shaderProgram->getID();
    cube->num_vertices = state.assets.cubeMesh->getVertices().size();
    cube->num_indices = state.assets.cubeMesh->getIndices().size();
    cube->vertices = state.assets.cubeMesh->getVertices().data();
    cube->indices = state.assets.cubeMesh->getIndices().data();

    system_cube_renderer_init(cube);

    state.camera->follow(glm::vec2(position.x, position.y));
    
    while (state.tilemap->getNumberOfStartingPositions() > 0)
    {
        state.game_object_manager->createEnemy(
            state.tilemap->getStartingPosition(), state.level, 
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
    
    if (input->back.key_down)
    {
        // Todo: send end signal?
    }
    else
    {
        f32 step = 1.0f / 60.0f;
        state.accumulator += input->delta_time;

        while (state.accumulator >= step)
        {
            state.accumulator -= step;

            state.game_object_manager->update(input);


            glm::vec2 velocity(0.0f);
            f32 move_speed = GLOBALS::PLAYER_SPEED;

            if (input->move_left.key_down)
            {
                velocity.x -= move_speed;
            }
            
            if (input->move_right.key_down)
            {
                velocity.x += move_speed;
            }

            if (input->move_up.key_down)
            {
                velocity.y += move_speed;
            }
            
            if (input->move_down.key_down)
            {
                velocity.y -= move_speed;
            }

            transforms[0].position.x += velocity.x;
            transforms[0].position.y += velocity.y;
        }

        state.game_object_manager->interpolate(state.accumulator / step);

        if (!state.game_ending && 
            state.game_object_manager->getNumberOfObjectsOfType(
                GAMEOBJECT_TYPES::PLAYER) == 0)
        {
            // printf("PLAYER LOST - died on level %d\n ", state.level);

            state.game_ending = 1;
        }
        else if (state.game_ending)
        {
            // Todo: send end signal
        }
        else if (state.game_object_manager->getNumberOfObjectsOfType(
            GAMEOBJECT_TYPES::ENEMY) == 0)
        {
            // printf("PLAYER WON - cleared level ", state.level);
            // Weapon* weapon = 
            //     state.game_object_manager.getFirstObjectOfType(
            //     GAMEOBJECT_TYPES::PLAYER)->getComponent<PlayerController>()
            // ->getWeapon()->getCopy();
            
            // Todo: move to next level
            // state.scenes.change(new GameScene(state, level + 1, weapon));
        }
    }

    state.camera->follow(glm::vec2(transforms[0].position.x, transforms[0].position.y));
    state.tilemap->draw();
    state.game_object_manager->draw();
    system_cube_renderer_update(&state.cube_renderer, &state.camera->getViewMatrix(), &state.camera->getPerspectiveMatrix());
}