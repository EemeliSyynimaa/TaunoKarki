#include "aicontroller.h"
#include "gameobject.h"
#include "gameobjectmanager.h"

AIController::AIController(GameObject* owner) : Component(owner)
{
	state = WANDER;

	transform = owner->getComponent<Transform>();
	assert(transform);
}

AIController::~AIController()
{
}

void AIController::update(float deltaTime)
{
	switch (state)
	{
	case WANDER: wander(); break;
	case ATTACK: attack(); break;
	case PURSUE: pursue(); break;
	case ESCAPE: escape(); break;
	default: break;
	}
}

void AIController::wander()
{
	float minDistance = 10.0f;

	GameObject* player = getOwner()->gameObjectManager.getFirstObjectOfType(GAMEOBJECT_TYPES::PLAYER);

	if (player && sqrt(powf(player->getComponent<Transform>()->getPosition().x - transform->getPosition().x, 2) + powf(player->getComponent<Transform>()->getPosition().y - transform->getPosition().y, 2)) < minDistance) state = ATTACK;
}

void AIController::attack()
{
	float minDistance = 10.0f;

	GameObject* player = getOwner()->gameObjectManager.getFirstObjectOfType(GAMEOBJECT_TYPES::PLAYER);

	if (player)
	{ 
		transform->lookAt(player->getComponent<Transform>()->getPosition());

		if (sqrt(powf(player->getComponent<Transform>()->getPosition().x - transform->getPosition().x, 2) + powf(player->getComponent<Transform>()->getPosition().y - transform->getPosition().y, 2)) > minDistance) state = WANDER;
	}
}

void AIController::pursue()
{
}

void AIController::escape()
{
}