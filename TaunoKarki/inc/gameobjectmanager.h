#ifndef GAMEOBJECTMANAGER_H
#define GAMEOBJECTMANAGER_H

#include <vector>
#include "glm\glm.hpp"
#include "assetmanager.h"
#include "Box2D\Box2D.h"
#include "camera.h"

class GameObject;

class GameObjectManager
{
public:
	GameObjectManager(AssetManager& assetManager, b2World& world, Camera& camera);
	~GameObjectManager();

	void update(float deltaTime);
	GameObject* createPlayer(glm::vec3 pos);
	GameObject* createBullet(glm::vec3 pos, glm::vec2 direction);
	GameObject* createWall(glm::vec3 pos);

	Camera& getCamera() { return camera; }
private:
	GameObject* createObject();

	AssetManager& assetManager;
	b2World& world;
	Camera& camera;
	std::vector<GameObject*> gameObjects;
	std::vector<GameObject*> newObjects;
};

#endif