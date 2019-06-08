#ifndef MENUSCENE_H
#define MENUSCENE_H

#include "SDL/SDL_events.h"
#include "scene.h"
#include "gameobjectmanager.h"

class MenuScene : public Scene
{
public:
    MenuScene(game_state_t* state);
    ~MenuScene();

    void handleEvent(SDL_Event& event);
    void update(float deltaTime);
    void draw();
private:
    void writeText(char* text, int width, int height, glm::vec2 position, bool title, int id = 0);
    GameObjectManager gameObjectManager;
    Camera camera;
};

#endif