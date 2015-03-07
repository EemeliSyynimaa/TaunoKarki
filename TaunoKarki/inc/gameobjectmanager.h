#ifndef GAMEOBJECTMANAGER_H
#define GAMEOBJECTMANAGER_H

#include <vector>
#include "glm\glm.hpp"
#include "assetmanager.h"

class GameObject;

class GameObjectManager
{
public:
	GameObjectManager(AssetManager& assetManager);
	~GameObjectManager();

	void update(float deltaTime);
	GameObject* createPlayer(glm::vec3 pos);
	GameObject* createBullet(glm::vec3 pos);
	GameObject* createWall(glm::vec3 pos);
private:
	GameObject* createObject();

	AssetManager& assetManager;
	std::vector<GameObject*> gameObjects;
};

#endif