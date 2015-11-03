#include "playercontroller.h"
#include "gameobject.h"
#include "rigidbody.h"
#include "SDL\SDL.h"

#include "health.h"

#include "pistol.h"
#include "machinegun.h"
#include "shotgun.h"

#include "glm\glm.hpp"
#include "glm\geometric.hpp"

PlayerController::PlayerController(GameObject* owner) : Component(owner), moveSpeed(GLOBALS::PLAYER_SPEED), playerAudioChannel(-1)
{
	RigidBody* rigidbody = owner->getComponent<RigidBody>();
	assert(rigidbody);
	body = rigidbody->getBody();

	transform = owner->getComponent<Transform>();
	assert(transform);
}

PlayerController::~PlayerController()
{
	delete weapon;
}

void PlayerController::update()
{
	b2Vec2 desiredVelocity(0.0f, 0.0f);
	b2Vec2 velocityChange(0.0f, 0.0f);
	b2Vec2 impulse(0.0f, 0.0f);
	b2Vec2 velocity = body->GetLinearVelocity();
	glm::vec3 mouseCoords(0.0f, 0.0f, 0.0f);
	int x = 0, y = 0;
	float halfX = 0.0f, halfY = 0.0f;
	float moveSpeedForReal = 0;
	float strafeSpeed = 0;
	Uint8 mouseState = SDL_GetRelativeMouseState(&x, &y);

	if (keyboardState[SDL_SCANCODE_A])
		strafeSpeed = moveSpeed;
	else if (keyboardState[SDL_SCANCODE_D])
		strafeSpeed = -moveSpeed;
	else
		strafeSpeed = 0.0f;

	if (keyboardState[SDL_SCANCODE_W])
		moveSpeedForReal = moveSpeed;
	else if (keyboardState[SDL_SCANCODE_S])
		moveSpeedForReal = -moveSpeed;
	else
		moveSpeedForReal = 0.0f;

	if (keyboardState[SDL_SCANCODE_R] && !weapon->isReloading())
		weapon->reload(); 

	glm::vec2 superTemp = owner->getComponent<Transform>()->getDirVec();
	glm::vec3 superDuperTemp(superTemp.x, superTemp.y, 0.0f);
	
	glm::vec3 temp = glm::cross(glm::vec3(0, 0, 1), superDuperTemp);

	velocityChange.x = owner->getComponent<Transform>()->getDirVec().x * moveSpeedForReal - velocity.x;
	velocityChange.y = owner->getComponent<Transform>()->getDirVec().y * moveSpeedForReal - velocity.y;
	velocityChange.x += temp.x * strafeSpeed;
	velocityChange.y += temp.y * strafeSpeed;

	impulse.x = body->GetMass() * velocityChange.x;
	impulse.y = body->GetMass() * velocityChange.y;

	body->ApplyLinearImpulse(impulse, body->GetWorldCenter(), true);

	// Lets check if we start or stop firing
	if (!weapon->isTriggerPulled() && mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
		weapon->pullTheTrigger();
	else if (!(mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)))
		weapon->releaseTheTrigger();

	weapon->update();

	owner->getComponent<Transform>()->rotate(-x / 3, glm::vec3(0, 0, 1));
	owner->gameObjectManager.getCamera().follow(glm::vec2(owner->getComponent<Transform>()->getPosition().x, owner->getComponent<Transform>()->getPosition().y), owner->getComponent<Transform>()->getDirVec());

	if (!Mix_Playing(playerAudioChannel))
	{
		playerAudioChannel = -1;
	}
}

void PlayerController::giveWeapon(Weapon* weapon, bool instantReload)
{
	if (this->weapon) delete this->weapon;

	this->weapon = weapon; 
	this->weapon->setOwner(this->owner);
	this->weapon->reload(instantReload);
}

void PlayerController::handleItem(COLLECTIBLES item)
{
	switch (item)
	{
	case COLLECTIBLES::PISTOL:
	{
		if (weapon->getType() == item) weapon->levelUp();
		else giveWeapon(new Pistol(owner->gameObjectManager));
		break;
	}
	case COLLECTIBLES::MACHINEGUN: {
		if (weapon->getType() == item) weapon->levelUp();
		else giveWeapon(new MachineGun(owner->gameObjectManager));
		break;
	}
	case COLLECTIBLES::SHOTGUN: {
		if (weapon->getType() == item) weapon->levelUp();
		else giveWeapon(new Shotgun(owner->gameObjectManager));
		break;
	}
	case COLLECTIBLES::HEALTHPACK: owner->getComponent<Health>()->change(GLOBALS::PLAYER_HEALTH_PER_PACK); break;
	default: break;
	}
}