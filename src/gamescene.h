#ifndef GAMESCENE_H
#define GAMESCENE_H

#include "SDL/SDL_events.h"
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
	GameScene(game_state_t* state, int level, Weapon* weapon);
	~GameScene();

	void update(float deltaTime);
	void draw();
	void handleEvent(SDL_Event& event);
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