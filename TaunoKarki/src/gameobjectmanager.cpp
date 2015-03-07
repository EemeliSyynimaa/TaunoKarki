#include "gameobjectmanager.h"
#include "gameobject.h"

GameObjectManager::GameObjectManager(AssetManager& assetManager) : assetManager(assetManager)
{
}

GameObjectManager::~GameObjectManager()
{
	for (auto gameObject : gameObjects)
	{
		delete gameObject;
	}

	gameObjects.clear();
}

void GameObjectManager::update(float deltaTime)
{
	for (auto gameObject : gameObjects)
	{
		gameObject->update(deltaTime);
	}
}

GameObject* GameObjectManager::createBullet(glm::vec3 position)
{
	GameObject* gameObject = createObject();
	return gameObject;
}

GameObject* GameObjectManager::createWall(glm::vec3 position)
{
	GameObject* gameObject = createObject();
	return gameObject;
}

GameObject* GameObjectManager::createPlayer(glm::vec3 position)
{
	GameObject* gameObject = createObject();
	return gameObject;
}

GameObject* GameObjectManager::createObject()
{
	GameObject* gameObject = new GameObject(*this);
	gameObjects.push_back(gameObject);

	return gameObject;
}