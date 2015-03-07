#include "gameobjectmanager.h"
#include "gameobject.h"

#include "transform.h"
#include "circlecollider.h"
#include "meshrenderer.h"
#include "rigidbody.h"
#include "playercontroller.h"
#include "boxcollider.h"
#include "staticbody.h"

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
	for (auto gameObject : gameObjects)
	{
		gameObject->update(deltaTime);
	}

	for (auto gameObject : newObjects)
	{
		gameObjects.push_back(gameObject);
	}

	newObjects.clear();
}

GameObject* GameObjectManager::createBullet(glm::vec3 position, glm::vec2 direction)
{
	GameObject* gameObject = createObject();

	gameObject->addComponent(new Transform(gameObject, position.x + direction.x, position.y - direction.y, 0));
	gameObject->addComponent(new CircleCollider(gameObject, 0.1f));
	gameObject->addComponent(new MeshRenderer(gameObject));
	gameObject->addComponent(new RigidBody(gameObject, world));

	gameObject->getComponent<Transform>()->setScale(glm::vec3(0.1f));

	MeshRenderer* temp = gameObject->getComponent<MeshRenderer>();
	temp->setMesh(assetManager.sphereMesh);
	temp->setTexture(assetManager.sphereTexture);
	temp->setCamera(camera);
	temp->setProgram(assetManager.shaderProgram);

	b2Body* body = gameObject->getComponent<RigidBody>()->getBody();
	body->SetBullet(true);
	b2Vec2 forceDir(direction.x, -direction.y);

	forceDir *= 2.0f;
	body->ApplyLinearImpulse(forceDir, body->GetWorldCenter(), true);

	return gameObject;
}

GameObject* GameObjectManager::createWall(glm::vec3 position)
{
	GameObject* gameObject = createObject();

	gameObject->addComponent(new Transform(gameObject, position));
	gameObject->addComponent(new MeshRenderer(gameObject));
	gameObject->addComponent(new BoxCollider(gameObject, 1.0f, 1.0f));
	gameObject->addComponent(new StaticBody(gameObject, world));

	gameObject->getComponent<MeshRenderer>()->setMesh(assetManager.wallMesh);
	gameObject->getComponent<MeshRenderer>()->setProgram(assetManager.shaderProgram);
	gameObject->getComponent<MeshRenderer>()->setCamera(camera);
	gameObject->getComponent<MeshRenderer>()->setTexture(assetManager.wallTexture);

	return gameObject;
}

GameObject* GameObjectManager::createPlayer(glm::vec3 position)
{
	GameObject* gameObject = createObject();

	gameObject->addComponent(new Transform(gameObject, position.x, position.y, position.z));
	gameObject->addComponent(new CircleCollider(gameObject, 1.0f));
	gameObject->addComponent(new MeshRenderer(gameObject));
	gameObject->addComponent(new RigidBody(gameObject, world));
	gameObject->addComponent(new PlayerController(gameObject));

	gameObject->getComponent<MeshRenderer>()->setMesh(assetManager.wallMesh);
	gameObject->getComponent<MeshRenderer>()->setProgram(assetManager.shaderProgram);
	gameObject->getComponent<MeshRenderer>()->setCamera(camera);
	gameObject->getComponent<MeshRenderer>()->setTexture(assetManager.playerTexture);
	gameObject->getComponent<RigidBody>()->getBody()->SetFixedRotation(true);

	return gameObject;
}

GameObject* GameObjectManager::createObject()
{
	GameObject* gameObject = new GameObject(*this);
	newObjects.push_back(gameObject);

	return gameObject;
}