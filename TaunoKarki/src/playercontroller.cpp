#include "playercontroller.h"
#include "gameobject.h"
#include "rigidbody.h"

PlayerController::PlayerController(GameObject* owner) : Component(owner), speed(10.0f)
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
	b2Vec2 desiredVel(0.0f, 0.0f);
	b2Vec2 vel = body->GetLinearVelocity();

	if (keyboardState[SDL_SCANCODE_A])
		desiredVel.x = -speed;
	else if (keyboardState[SDL_SCANCODE_D])
		desiredVel.x = speed;
	else
		desiredVel.x = 0.0f;

	if (keyboardState[SDL_SCANCODE_W])
		desiredVel.y = speed;
	else if (keyboardState[SDL_SCANCODE_S])
		desiredVel.y = -speed;
	else
		desiredVel.y = 0.0f;

	b2Vec2 velChange(0.0f, 0.0f);
	velChange.x = desiredVel.x - vel.x;
	velChange.y = desiredVel.y - vel.y;

	b2Vec2 impulse(0.0f, 0.0f);
	impulse.x = body->GetMass() * velChange.x;
	impulse.y = body->GetMass() * velChange.y;

	body->ApplyLinearImpulse(impulse, body->GetWorldCenter(), true);

	if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
	{
		// TODO SHOOTING
	}
}