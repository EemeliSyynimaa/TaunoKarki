#include "aicontroller.h"
#include "gameobject.h"
#include "rigidbody.h"
#include "gameobjectmanager.h"
#include "tilemap.h"
#include "glm/gtc/constants.hpp"
#include <iostream>

#define randomInt std::uniform_int_distribution<int>

AIController::AIController(GameObject* owner, Tilemap* tilemap, b2World* world) : Component(owner), tilemap(tilemap), world(world), lastShot(SDL_GetTicks()), droppedItem(false), moveSpeed(GLOBALS::ENEMY_SPEED), target(0.0f), playerLastPosition(0.0f)
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
	GameObject* player = getOwner()->gameObjectManager.getFirstObjectOfType(GAMEOBJECT_TYPES::PLAYER);

	if (player && isPlayerInSight(player))
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
	GameObject* player = getOwner()->gameObjectManager.getFirstObjectOfType(GAMEOBJECT_TYPES::PLAYER);

	if (player && isPlayerInSight(player))
	{ 
		playerLastPosition = player->getComponent<Transform>()->getPosition();
		moveTo(playerLastPosition);
	
		if (weapon) shoot();
	}
	else initPursue();
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
	GameObject* player = getOwner()->gameObjectManager.getFirstObjectOfType(GAMEOBJECT_TYPES::PLAYER);

	if (player && isPlayerInSight(player))
		initAttack();
	else
	{ 
		moveTo(playerLastPosition);

		if (transform->distanceTo(playerLastPosition) < 0.1f)
		{
			playerLastPosition = glm::vec3(0.0f);
			initWander();
		}
	}
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

	if (weapon) weapon->releaseTheTrigger();
	
	getNewTarget();
}

void AIController::initEscape()
{
	state = ESCAPE;
}

void AIController::initPursue()
{
	state = PURSUE;

	body->SetLinearVelocity(b2Vec2_zero);

	if (weapon) weapon->releaseTheTrigger();
}

void AIController::getNewTarget()
{
	target = tilemap->getRandomFreePosition();
}

bool AIController::isPlayerInSight(GameObject* player)
{
	Transform* plrTransform = player->getComponent<Transform>();

	if (transform->distanceTo(plrTransform->getPosition()) < GLOBALS::ENEMY_ACTIVATION_DISTANCE)
	{
		b2Vec2 AIPos;
		AIPos.x = transform->getPosition().x;
		AIPos.y = transform->getPosition().y;

		b2Vec2 plrPos;
		plrPos.x = plrTransform->getPosition().x;
		plrPos.y = plrTransform->getPosition().y;

		RayCastCallback callBack;

		world->RayCast(&callBack, AIPos, plrPos);

		if (callBack.playerIsVisible)
		{
			glm::vec2 AIDir = transform->getDirVec();
			glm::vec2 plrDir = glm::normalize(glm::vec2(plrPos.x - AIPos.x, plrPos.y - AIPos.y));

			float dotProduct = glm::dot(AIDir, plrDir);

			if (dotProduct > 1.0f) dotProduct = 1.0f;

			if (glm::degrees(glm::acos(dotProduct)) < GLOBALS::ENEMY_ANGLE_OF_VISION) return true;
		}
	}

	return false;
}

void AIController::gotShot(glm::vec3 from)
{
	transform->lookAt(from);

	initAttack();
}

float32 AIController::RayCastCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction)
{
	// Only walls dont have a pointer to a gameobject.
	// And this raycast should only fail when it hits a wall.
	// So we check if the body has a gameobject and act according to it.

	GameObject* gameObject = static_cast<GameObject*>(fixture->GetBody()->GetUserData());

	if (!gameObject)
	{
		playerIsVisible = false;
		return 0;
	}
	
	return -1;
}