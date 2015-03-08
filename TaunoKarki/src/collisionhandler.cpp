#include "collisionhandler.h"
#include "gameobject.h"
#include "health.h"

CollisionHandler::CollisionHandler()
{
}

CollisionHandler::~CollisionHandler()
{
}

void CollisionHandler::BeginContact(b2Contact* contact)
{
	handleCollision(static_cast<GameObject*>(contact->GetFixtureA()->GetBody()->GetUserData()), static_cast<GameObject*>(contact->GetFixtureB()->GetBody()->GetUserData()));
}

void CollisionHandler::EndContact(b2Contact* contact)
{
}

void CollisionHandler::handleCollision(GameObject* gameObjectA, GameObject* gameObjectB)
{
	// TODO 2HAX :D
	// Lets handle the collisions
	// Bullet hits a wall
	if ((gameObjectA->getType() == GAMEOBJECT_TYPES::ENEMY_BULLET || gameObjectA->getType() == GAMEOBJECT_TYPES::PLAYER_BULLET)
		&& gameObjectB->getType() == GAMEOBJECT_TYPES::WALL)
		gameObjectA->kill();

	if ((gameObjectB->getType() == GAMEOBJECT_TYPES::ENEMY_BULLET || gameObjectB->getType() == GAMEOBJECT_TYPES::PLAYER_BULLET)
		&& gameObjectA->getType() == GAMEOBJECT_TYPES::WALL)
		gameObjectB->kill();

	// Player bullet hits an enemy
	if ((gameObjectA->getType() == GAMEOBJECT_TYPES::PLAYER_BULLET)
		&& gameObjectB->getType() == GAMEOBJECT_TYPES::ENEMY)
	{
		gameObjectA->kill();
		gameObjectB->getComponent<Health>()->change(-25);
	}

	if ((gameObjectB->getType() == GAMEOBJECT_TYPES::PLAYER_BULLET)
		&& gameObjectA->getType() == GAMEOBJECT_TYPES::ENEMY)
	{
		gameObjectA->getComponent<Health>()->change(-25);
		gameObjectB->kill();
	}

	// Enemy hits the player
	if ((gameObjectB->getType() == GAMEOBJECT_TYPES::PLAYER)
		&& gameObjectA->getType() == GAMEOBJECT_TYPES::ENEMY)
	{
		gameObjectB->getComponent<Health>()->change(-25);
	}

	if ((gameObjectA->getType() == GAMEOBJECT_TYPES::PLAYER)
		&& gameObjectB->getType() == GAMEOBJECT_TYPES::ENEMY)
	{
		gameObjectA->getComponent<Health>()->change(-25);
	}
}