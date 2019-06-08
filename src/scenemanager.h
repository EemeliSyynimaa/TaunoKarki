#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <vector>
#include "SDL/SDL_events.h"
#include "scene.h"

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    void update(float deltaTime);
    void draw();

    void push(Scene* scene);
    void pop();
    void change(Scene* scene);
    void handleEvent(SDL_Event& event);
private:

    std::vector<Scene*> scenes;
};

#endif