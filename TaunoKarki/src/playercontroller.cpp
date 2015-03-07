#include "playercontroller.h"
#include "gameobject.h"
#include "rigidbody.h"
#include "SDL\SDL.h"

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
	b2Vec2 velChange(0.0f, 0.0f);
	b2Vec2 impulse(0.0f, 0.0f);
	b2Vec2 vel = body->GetLinearVelocity();
	b2Body* plrBody = owner->getComponent<RigidBody>()->getBody();
	glm::vec3 mouseInDaWorld(0.0f, 0.0f, 0.0f);
	int x = 0, y = 0;
	float halfX = 0.0f, halfY = 0.0f;

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

	velChange.x = desiredVel.x - vel.x;
	velChange.y = desiredVel.y - vel.y;

	impulse.x = body->GetMass() * velChange.x;
	impulse.y = body->GetMass() * velChange.y;

	body->ApplyLinearImpulse(impulse, body->GetWorldCenter(), true);

	if (SDL_GetMouseState(&x, &y) & SDL_BUTTON(SDL_BUTTON_LEFT))
	{
		owner->gameObjectManager.createBullet(owner->getComponent<Transform>()->getPosition(), owner->getComponent<Transform>()->getDirVec());
	}

	halfX = (owner->gameObjectManager.getCamera().getWidth() / 2.0f - x) * -1;
	halfY = (owner->gameObjectManager.getCamera().getHeight() / 2.0f - y) * -1;
	mouseInDaWorld = glm::vec3(halfX, halfY, 0.0f);

	owner->getComponent<Transform>()->lookAt(owner->getComponent<Transform>()->getPosition() - mouseInDaWorld);
	owner->gameObjectManager.getCamera().follow(glm::vec2(owner->getComponent<Transform>()->getPosition().x, owner->getComponent<Transform>()->getPosition().y));
}