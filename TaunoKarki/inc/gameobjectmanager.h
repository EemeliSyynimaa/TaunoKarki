#ifndef GAMEOBJECTMANAGER_H
#define GAMEOBJECTMANAGER_H

#include <list>
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
	GameObject* createEnemy(glm::vec3 pos);
	GameObject* createBullet(glm::vec3 pos, glm::vec2 direction, unsigned int owner, float damage);
	GameObject* createWall(glm::vec3 pos);

	Camera& getCamera() { return camera; }

	GameObject* getFirstObjectOfType(size_t type) const;
	size_t getNumberOfObjectsOfType(size_t type) const;
private:
	GameObject* createObject();

	AssetManager& assetManager;
	b2World& world;
	Camera& camera;
	std::list<GameObject*> gameObjects;
	std::list<GameObject*> newObjects;
};

#endif