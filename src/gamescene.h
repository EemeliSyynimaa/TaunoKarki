#ifndef GAMESCENE_H
#define GAMESCENE_H

#include "scene.h"
#include "tilemap.h"
#include "camera.h"
#include "Box2D\Box2D.h"
#include "collisionhandler.h"
#include "gameobjectmanager.h"
#include "weapon.h"

class GameScene : public Scene
{
public:
    GameScene(tk_game_state_t* state, int level, Weapon* weapon);
    ~GameScene();

    void update(float deltaTime, tk_state_player_input_t* input);
    void draw();
    void endGame();
private:
    b2World world;
    Tilemap* tilemap;
    CollisionHandler collisionHandler;
    Camera camera;
    GameObjectManager gameObjectManager;
    int level;

    bool gameEnding;
    int playerDyingChannel;
};



#endif