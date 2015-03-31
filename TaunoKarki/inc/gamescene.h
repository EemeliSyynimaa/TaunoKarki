#ifndef GAMESCENE_H
#define GAMESCENE_H

#include "SDL/SDL_events.h"
#include "scene.h"
#include "tilemap.h"
#include "camera.h"
#include "Box2D\Box2D.h"
#include "assetmanager.h"
#include "collisionhandler.h"
#include "gameobjectmanager.h"

class GameScene : public Scene
{
public:
	GameScene(Game& game, int level);
	~GameScene();

	void update(float deltaTime);
	void draw();
	void handleEvent(SDL_Event& event);
private:
	AssetManager assetManager;
	b2World world;
	Tilemap* tilemap;
	CollisionHandler collisionHandler;
	Camera camera;
	GameObjectManager gameObjectManager;
	int level;

	// Variables for fixed timestep
	float accumulator;
	float step;
};

#endif