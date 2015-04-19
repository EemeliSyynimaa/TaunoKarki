#include "aicontroller.h"
#include "gameobject.h"
#include "rigidbody.h"
#include "gameobjectmanager.h"

AIController::AIController(GameObject* owner) : Component(owner), lastShot(SDL_GetTicks()), droppedItem(false), moveSpeed(5.0f)
{
	state = WANDER;

	transform = owner->getComponent<Transform>();
	RigidBody* rigidbody = owner->getComponent<RigidBody>();
	assert(transform && rigidbody);

	body = rigidbody->getBody();
}

AIController::~AIController()
{
	delete weapon;
}

void AIController::update()
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
		moveTo(player->getComponent<Transform>()->getPosition());

		if (sqrt(powf(player->getComponent<Transform>()->getPosition().x - transform->getPosition().x, 2) + powf(player->getComponent<Transform>()->getPosition().y - transform->getPosition().y, 2)) > minDistance)
		{
			state = WANDER;

			body->SetLinearVelocity(b2Vec2_zero);

			if (weapon) weapon->releaseTheTrigger();
		}

		if (weapon) shoot();
	}
}

void AIController::shoot()
{
	switch (weapon->getType())
	{
	case COLLECTIBLES::PISTOL:
	{
		Uint32 time = SDL_GetTicks();
		if (!weapon->isTriggerPulled() && (time - lastShot) > 450)
		{
			weapon->pullTheTrigger();
			lastShot = time;
		}
		else weapon->releaseTheTrigger();
		break;
	}
	case COLLECTIBLES::SHOTGUN:
	{
		if (!weapon->isTriggerPulled()) weapon->pullTheTrigger();
		else weapon->releaseTheTrigger();
		break;
	}
	case COLLECTIBLES::MACHINEGUN: weapon->pullTheTrigger(); break;
	default: break;
	}

	weapon->update();
}

void AIController::moveTo(glm::vec3 position)
{
	b2Vec2 impulse(0.0f, 0.0f);
	b2Vec2 velocity = body->GetLinearVelocity();
	int x = 0, y = 0;

	transform->lookAt(position);

	impulse.x = body->GetMass() * transform->getDirVec().x * moveSpeed - velocity.x;
	impulse.y = body->GetMass() * transform->getDirVec().y * moveSpeed - velocity.y;

	body->ApplyLinearImpulse(impulse, body->GetWorldCenter(), true);
}

void AIController::pursue()
{
}

void AIController::escape()
{
}