#include <cassert>
#include <iostream>
#include <algorithm>
#include "locator.h"

typedef struct game_state
{
    b2World* world;
    Tilemap* tilemap;
    CollisionHandler* collision_handler;
    Camera* camera;
    GameObjectManager* game_object_manager;
    b32 game_ending;
    s32 level;
    s32 player_dying_channel;
    f32 accumulator;
} game_state;

game_state state;

int32_t tk_current_time_get()
{
    int32_t result = SDL_GetTicks();

    return result;
}

int32_t tk_sound_play(tk_sound_sample* sample, int32_t channel, 
    int32_t loops)
{
    Mix_Chunk* chunk = (Mix_Chunk*)sample;
    return Mix_PlayChannel(channel, chunk, loops);
}

int32_t tk_sound_is_playing(int32_t channel)
{
    return Mix_Playing(channel);
}

void init_game(s32 screen_width, s32 screen_height)
{
    state.level = 10;
    state.world = new b2World(b2Vec2(0.0f, 0.0f));
    state.collision_handler = new CollisionHandler();
    state.camera = new Camera();
    state.game_object_manager = new GameObjectManager(
        *Locator::getAssetManager(), *state.camera, state.world);

    std::random_device randomDevice;
    std::default_random_engine randomGenerator(randomDevice());

    tk_sound_play(Locator::getAssetManager()->ambienceSound, 0, -1);

    printf("GAMESCENE ALIVE - entering level %d\n ", state.level);

    state.world->SetContactListener(state.collision_handler);

    state.camera->createNewPerspectiveMatrix(60.0f, (float)screen_width, 
        (float)screen_height, 0.1f, 100.0f);
    state.camera->createNewOrthographicMatrix((float)screen_width,
        (float)screen_height);
    state.camera->setPosition(glm::vec3(0.0f, 0.0f, 20.0f));
    state.camera->setOffset(0.0f, -7.5f, 0.0f);

    for (;;)
    {
        state.tilemap = new Tilemap(glm::vec3(0.0f), 
            *Locator::getAssetManager(), *state.camera, *state.world);
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

    state.game_object_manager->createPlayer(
        state.tilemap->getStartingPosition(), nullptr);
    
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
    if (input->back.key_down)
    {
        Mix_HaltChannel(-1);
        // Todo: send end signal
    }
    else
    {
        f32 step = 1.0f / 60.0f;
        state.accumulator += input->delta_time;

        while (state.accumulator >= step)
        {
            state.accumulator -= step;

            state.world->Step(step, 8, 3);

            state.game_object_manager->update(input);
        }

        state.game_object_manager->interpolate(state.accumulator / step);

        if (!state.game_ending && 
            state.game_object_manager->getNumberOfObjectsOfType(
                GAMEOBJECT_TYPES::PLAYER) == 0)
        {
            // printf("PLAYER LOST - died on level %d\n ", state.level);

            state.player_dying_channel = tk_sound_play(
                Locator::getAssetManager()->playerDeadSound);
            
            state.game_ending = 1;
        }
        else if (state.game_ending)
        {
            if (!tk_sound_is_playing(state.player_dying_channel))
            {
                Mix_HaltChannel(-1);
                // Todo: send end signal
            }
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

    state.tilemap->draw();
    state.game_object_manager->draw();
}