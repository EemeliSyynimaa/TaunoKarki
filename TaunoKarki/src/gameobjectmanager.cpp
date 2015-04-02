#include "gameobjectmanager.h"
#include "gameobject.h"

#include <algorithm>

#include "transform.h"
#include "circlecollider.h"
#include "meshrenderer.h"
#include "rigidbody.h"
#include "playercontroller.h"
#include "boxcollider.h"
#include "staticbody.h"
#include "health.h"
#include "aicontroller.h"
#include "damage.h"

#include "pistol.h"
#include "machinegun.h"
#include "shotgun.h"

GameObjectManager::GameObjectManager(AssetManager& assetManager, b2World& world, Camera& camera) : assetManager(assetManager), world(world), camera(camera)
{
}

GameObjectManager::~GameObjectManager()
{
	for (auto gameObject : gameObjects)
	{
		delete gameObject;
	}

	for (auto gameObject : newObjects)
	{
		delete gameObject;
	}

	gameObjects.clear();
	newObjects.clear();
}

void GameObjectManager::update(float deltaTime)
{
	// TODO 2HAX PLS FIX
	std::list<GameObject*> gameObjectsToDelete;

	for (auto gameObject : gameObjects)
	{
		gameObject->update(deltaTime);
		if (!gameObject->isAlive()) gameObjectsToDelete.push_back(gameObject);
	}

	gameObjects.erase(std::remove_if(gameObjects.begin(), gameObjects.end(), [](GameObject *obj){ return !obj->isAlive(); }), gameObjects.end());

	for (auto gameObject : newObjects)
	{
		gameObjects.push_back(gameObject);
	}

	for (auto gameObject : gameObjectsToDelete)
	{

		delete gameObject;
	}

	gameObjectsToDelete.clear();
	newObjects.clear();
}

void GameObjectManager::draw()
{
	for (auto gameObject : gameObjects)
		gameObject->draw();
}

GameObject* GameObjectManager::createBullet(glm::vec3 position, glm::vec2 direction, unsigned int owner, float damage)
{
	GameObject* gameObject = createObject();

	gameObject->setType(owner);

	gameObject->addComponent(new Transform(gameObject, position.x + direction.x, position.y + direction.y, 0));
	gameObject->addComponent(new CircleCollider(gameObject, 0.05f));
	gameObject->addComponent(new RigidBody(gameObject, world));
	gameObject->addComponent(new Damage(gameObject, damage));
	gameObject->addDrawableComponent(new MeshRenderer(gameObject));

	gameObject->getComponent<Transform>()->setScale(glm::vec3(0.05f));
	gameObject->getDrawableComponent<MeshRenderer>()->setMesh(assetManager.sphereMesh);
	gameObject->getDrawableComponent<MeshRenderer>()->setTexture(assetManager.sphereTexture);
	gameObject->getDrawableComponent<MeshRenderer>()->setCamera(camera);
	gameObject->getDrawableComponent<MeshRenderer>()->setProgram(assetManager.shaderProgram);

	b2Body* body = gameObject->getComponent<RigidBody>()->getBody();
	body->SetBullet(true);
	b2Vec2 forceDir(direction.x, direction.y);

	forceDir *= 0.3f;
	body->ApplyLinearImpulse(forceDir, body->GetWorldCenter(), true);

	return gameObject;
}

GameObject* GameObjectManager::createPlayer(glm::vec3 position)
{
 	GameObject* gameObject = createObject();

	gameObject->setType(GAMEOBJECT_TYPES::PLAYER);

	gameObject->addComponent(new Transform(gameObject, position.x, position.y, position.z));
	gameObject->addComponent(new CircleCollider(gameObject, 0.5f));
	gameObject->addComponent(new RigidBody(gameObject, world));
	gameObject->addComponent(new PlayerController(gameObject));
	gameObject->addComponent(new Health(gameObject, 100));
	gameObject->addDrawableComponent(new MeshRenderer(gameObject));

	gameObject->getComponent<Transform>()->setScale(glm::vec3(0.5f, 0.5f, 0.75f));
	gameObject->getDrawableComponent<MeshRenderer>()->setMesh(assetManager.cubeMesh);
	gameObject->getDrawableComponent<MeshRenderer>()->setProgram(assetManager.shaderProgram);
	gameObject->getDrawableComponent<MeshRenderer>()->setCamera(camera);
	gameObject->getDrawableComponent<MeshRenderer>()->setTexture(assetManager.playerTexture);
	gameObject->getComponent<RigidBody>()->getBody()->SetFixedRotation(true);
	gameObject->getComponent<PlayerController>()->giveWeapon(new MachineGun(*this));
	
	return gameObject;
}

GameObject* GameObjectManager::createEnemy(glm::vec3 position)
{
	GameObject* gameObject = createObject();

	gameObject->setType(GAMEOBJECT_TYPES::ENEMY);

	gameObject->addComponent(new Transform(gameObject, position.x, position.y, position.z));
	gameObject->addComponent(new CircleCollider(gameObject, 0.5f));
	gameObject->addComponent(new RigidBody(gameObject, world));
	gameObject->addComponent(new Health(gameObject, 100));
	gameObject->addComponent(new AIController(gameObject));
	gameObject->addComponent(new Damage(gameObject, 50.0f));
	gameObject->addDrawableComponent(new MeshRenderer(gameObject));

	gameObject->getComponent<Transform>()->setScale(glm::vec3(0.5f, 0.5f, 0.75f));
	gameObject->getDrawableComponent<MeshRenderer>()->setMesh(assetManager.cubeMesh);
	gameObject->getDrawableComponent<MeshRenderer>()->setProgram(assetManager.shaderProgram);
	gameObject->getDrawableComponent<MeshRenderer>()->setCamera(camera);
	gameObject->getDrawableComponent<MeshRenderer>()->setTexture(assetManager.enemyTexture);
	gameObject->getComponent<RigidBody>()->getBody()->SetFixedRotation(true);
	gameObject->getComponent<AIController>()->giveWeapon(new Pistol(*this));

	return gameObject;
}

GameObject* GameObjectManager::getFirstObjectOfType(size_t type) const
{
	for (GameObject* gameObject : gameObjects)
	{
		if (gameObject->getType() == type) return gameObject;
	}

	return nullptr;
}

size_t GameObjectManager::getNumberOfObjectsOfType(size_t type) const
{
	size_t counter = 0;

	for (GameObject* gameObject : gameObjects)
	{
		if (gameObject->getType() == type) counter++;
	}

	return counter;
}

void GameObjectManager::interpolate(float alpha)
{
	for (GameObject* gameObject : gameObjects)
	{
		Transform* transform = gameObject->getComponent<Transform>();
		RigidBody* rigidBody = gameObject->getComponent<RigidBody>();

		// We are only interested in moving objects
		if (transform && rigidBody)
		{
			glm::vec3 position = transform->getPosition();
			b2Vec2 bodyPosition = rigidBody->getBody()->GetPosition();

			position.x = bodyPosition.x * alpha + position.x * (1.0f - alpha);
			position.y = bodyPosition.y * alpha + position.y * (1.0f - alpha);

			transform->setPosition(position);
		}
	}
}

GameObject* GameObjectManager::createObject()
{
	GameObject* gameObject = new GameObject(*this);
	newObjects.push_back(gameObject);

	return gameObject;
}