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
	gameObjectA->handleCollisionWith(gameObjectB);
	gameObjectB->handleCollisionWith(gameObjectA);
}