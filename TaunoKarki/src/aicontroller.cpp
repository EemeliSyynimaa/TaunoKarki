#include "aicontroller.h"
#include "gameobject.h"
#include "rigidbody.h"
#include "gameobjectmanager.h"
#include "tilemap.h"

#define randomInt std::uniform_int_distribution<int>

AIController::AIController(GameObject* owner, Tilemap* tilemap) : Component(owner), tilemap(tilemap), lastShot(SDL_GetTicks()), droppedItem(false), moveSpeed(5.0f), target(0.0f)
{
	transform = owner->getComponent<Transform>();
	RigidBody* rigidbody = owner->getComponent<RigidBody>();
	assert(transform && rigidbody);
	body = rigidbody->getBody();

	initWander();
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

	if (weapon)
		weapon->update();
}

void AIController::wander()
{
	float minDistance = 10.0f;

	GameObject* player = getOwner()->gameObjectManager.getFirstObjectOfType(GAMEOBJECT_TYPES::PLAYER);

	if (player && transform->distanceTo(player->getComponent<Transform>()->getPosition()) < minDistance)
		initAttack();
	else
	{
		if (transform->distanceTo(target) < 0.1f)
			getNewTarget();

		moveTo(target);
	}
		
}

void AIController::attack()
{
	float minDistance = 10.0f;

	GameObject* player = getOwner()->gameObjectManager.getFirstObjectOfType(GAMEOBJECT_TYPES::PLAYER);

	if (player)
	{ 
		moveTo(player->getComponent<Transform>()->getPosition());

		if (transform->distanceTo(player->getComponent<Transform>()->getPosition()) > minDistance)
		{
			initWander();
		}
		else if (weapon) shoot();
	}
	else initWander();
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

void AIController::initAttack()
{
	state = ATTACK;
}

void AIController::initWander()
{
	state = WANDER;

	body->SetLinearVelocity(b2Vec2_zero);

	if (weapon)
	{
		weapon->releaseTheTrigger();
		weapon->reload();
	}
	
	getNewTarget();
}

void AIController::initEscape()
{
	state = ESCAPE;
}

void AIController::initPursue()
{
	state = PURSUE;
}

void AIController::getNewTarget()
{
	target = tilemap->getRandomFreePosition();
}