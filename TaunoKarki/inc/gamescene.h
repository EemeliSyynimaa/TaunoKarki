#ifndef GAMESCENE_H
#define GAMESCENE_H

#include "SDL/SDL_events.h"
#include "scene.h"
#include "gameobject.h"
#include "tilemap.h"
#include "camera.h"
#include "Box2D\Box2D.h"
#include "assetmanager.h"

class GameScene : public Scene
{
public:
	GameScene(Game& game);
	~GameScene();

	void update(float deltaTime);
	void draw();
	void handleEvent(SDL_Event& event);
private:
	AssetManager assetManager;
	Tilemap* tilemap;

	b2World world;
	Camera camera;

	GameObjectManager gameObjectManager;
	GameObject* plr;
};

#endif