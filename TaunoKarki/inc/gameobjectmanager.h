#ifndef GAMEOBJECTMANAGER_H
#define GAMEOBJECTMANAGER_H

#include <list>
#include <functional>
#include "glm\glm.hpp"
#include "assetmanager.h"
#include "Box2D\Box2D.h"
#include "camera.h"

class Weapon;
class GameObject;

class GameObjectManager
{
public:
	GameObjectManager(AssetManager& assetManager, Camera& camera, b2World* world = nullptr);
	~GameObjectManager();

	void update();
	void draw();
	void createObjects();
	void deleteObjects();
	void addNewObject(std::function< void(void)> gameObject);
	GameObject* createPlayer(glm::vec3 pos, Weapon* weapon);
	GameObject* createEnemy(glm::vec3 pos, int level = 1);
	GameObject* createBullet(glm::vec3 pos, glm::vec2 direction, unsigned int owner, float damage, float speed);
	GameObject* createPlayerHealthBar(glm::vec3 pos, glm::vec3 size);
	GameObject* createPlayerAmmoBar(glm::vec3 pos, glm::vec3 size);
	GameObject* createRandomItem(glm::vec3 position);
	GameObject* createMenuBlock(glm::vec3 position);

	Camera& getCamera() { return camera; }

	GameObject* getFirstObjectOfType(size_t type) const;
	size_t getNumberOfObjectsOfType(size_t type) const;

	void interpolate(float alpha);
private:
	GameObject* createObject();

	AssetManager& assetManager;
	b2World* world;
	Camera& camera;
	std::list<GameObject*> gameObjects;
	std::list<GameObject*> deadObjects;

	std::vector<std::function<void(void)>> newObjects;
};

#endif