#ifndef MENUSCENE_H
#define MENUSCENE_H

#include "scene.h"
#include "gameobjectmanager.h"

class MenuScene : public Scene
{
public:
    MenuScene(tk_game_state_t* state);
    ~MenuScene();

    void update(float deltaTime, tk_state_player_input_t* input);
    void draw();
private:
    void writeText(char* text, int width, int height, glm::vec2 position, bool title, int id = 0);
    GameObjectManager gameObjectManager;
    Camera camera;
};

#endif