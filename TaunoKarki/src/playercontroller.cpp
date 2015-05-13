#include "playercontroller.h"
#include "gameobject.h"
#include "rigidbody.h"
#include "SDL\SDL.h"

#include "health.h"

#include "pistol.h"
#include "machinegun.h"
#include "shotgun.h"

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
	Uint8 mouseState = SDL_GetMouseState(&x, &y);

	if (keyboardState[SDL_SCANCODE_A])
		desiredVelocity.x = -moveSpeed;
	else if (keyboardState[SDL_SCANCODE_D])
		desiredVelocity.x = moveSpeed;
	else
		desiredVelocity.x = 0.0f;

	if (keyboardState[SDL_SCANCODE_W])
		desiredVelocity.y = moveSpeed;
	else if (keyboardState[SDL_SCANCODE_S])
		desiredVelocity.y = -moveSpeed;
	else
		desiredVelocity.y = 0.0f;

	if (keyboardState[SDL_SCANCODE_R] && !weapon->isReloading())
		weapon->reload();

	velocityChange.x = desiredVelocity.x - velocity.x;
	velocityChange.y = desiredVelocity.y - velocity.y;

	impulse.x = body->GetMass() * velocityChange.x;
	impulse.y = body->GetMass() * velocityChange.y;

	body->ApplyLinearImpulse(impulse, body->GetWorldCenter(), true);

	// Lets check if we start or stop firing
	if (!weapon->isTriggerPulled() && mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
		weapon->pullTheTrigger();
	else if (!(mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)))
		weapon->releaseTheTrigger();

	weapon->update();

	halfX = (owner->gameObjectManager.getCamera().getWidth() / 2.0f - x) * -1;
	halfY = (owner->gameObjectManager.getCamera().getHeight() / 2.0f - y) * -1;
	mouseCoords = glm::vec3(halfX, -halfY, 0.0f);

	owner->getComponent<Transform>()->lookAt(owner->getComponent<Transform>()->getPosition() + mouseCoords);
	owner->gameObjectManager.getCamera().follow(glm::vec2(owner->getComponent<Transform>()->getPosition().x, owner->getComponent<Transform>()->getPosition().y));

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