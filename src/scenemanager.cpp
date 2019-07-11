#include <algorithm>
#include "scenemanager.h"
#include "scene.h"

SceneManager::SceneManager()
{
}

SceneManager::~SceneManager()
{
    for (auto& scene : scenes)
    {
        delete scene;
    }

    scenes.clear();
}

void SceneManager::update(float deltaTime, tk_state_player_input_t* input)
{
    scenes.back()->update(deltaTime, input);
}

void SceneManager::draw()
{
    scenes.back()->draw();
}

void SceneManager::push(Scene* scene)
{
    scenes.push_back(scene);
}

void SceneManager::pop()
{
    delete scenes.back();
    scenes.pop_back();
}

void SceneManager::change(Scene* scene)
{
    if (!scenes.empty())
    {
        delete scenes.back();
        scenes.pop_back();
    }
        
    scenes.push_back(scene);
}
