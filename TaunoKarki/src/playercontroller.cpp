#include "playercontroller.h"
#include "gameobject.h"
#include "rigidbody.h"
#include "SDL\SDL.h"

#include <iostream>

PlayerController::PlayerController(GameObject* owner) : Component(owner), moveSpeed(10.0f), weaponFired(false)
{
	RigidBody* rigidbody = owner->getComponent<RigidBody>();
	assert(rigidbody);
	body = rigidbody->getBody();

	transform = owner->getComponent<Transform>();
	assert(transform);
}

PlayerController::~PlayerController()
{
}

void PlayerController::update(float deltaTime)
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

	velocityChange.x = desiredVelocity.x - velocity.x;
	velocityChange.y = desiredVelocity.y - velocity.y;

	impulse.x = body->GetMass() * velocityChange.x;
	impulse.y = body->GetMass() * velocityChange.y;

	body->ApplyLinearImpulse(impulse, body->GetWorldCenter(), true);

	if (!weaponFired && mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
	{
		owner->gameObjectManager.createBullet(owner->getComponent<Transform>()->getPosition(), owner->getComponent<Transform>()->getDirVec(), GAMEOBJECT_TYPES::PLAYER_BULLET);
		weaponFired = true;
	}
	else if (!mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
		weaponFired = false;

	halfX = (owner->gameObjectManager.getCamera().getWidth() / 2.0f - x) * -1;
	halfY = (owner->gameObjectManager.getCamera().getHeight() / 2.0f - y) * -1;
	mouseCoords = glm::vec3(halfX, -halfY, 0.0f);

	std::cout << halfX << ", " << halfY << ": " << owner->getComponent<Transform>()->getPosition().x << "," << owner->getComponent<Transform>()->getPosition().y << std::endl;

	owner->getComponent<Transform>()->lookAt(owner->getComponent<Transform>()->getPosition() + mouseCoords);
	owner->gameObjectManager.getCamera().follow(glm::vec2(owner->getComponent<Transform>()->getPosition().x, owner->getComponent<Transform>()->getPosition().y));
}