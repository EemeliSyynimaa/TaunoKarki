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

	void update( );
	void draw();
	GameObject* createPlayer(glm::vec3 pos);
	GameObject* createEnemy(glm::vec3 pos);
	GameObject* createBullet(glm::vec3 pos, glm::vec2 direction, unsigned int owner, float damage, float speed);
	GameObject* createPlayerHealthBar(glm::vec3 pos, glm::vec2 size, Texture* texture);
	GameObject* createPlayerAmmoBar(glm::vec3 pos, glm::vec2 size, Texture* texture);

	Camera& getCamera() { return camera; }

	GameObject* getFirstObjectOfType(size_t type) const;
	size_t getNumberOfObjectsOfType(size_t type) const;

	void interpolate(float alpha);
private:
	GameObject* createObject();

	AssetManager& assetManager;
	b2World& world;
	Camera& camera;
	std::list<GameObject*> gameObjects;
	std::list<GameObject*> newObjects;
};

#endif