#include "rigidbody.h"
#include "gameobject.h"
#include "circlecollider.h"
#include "boxcollider.h"

Rigidbody::Rigidbody(GameObject* owner, b2World& world) : Component(owner), world(world)
{
	transform = owner->getComponent<Transform>();
	assert(transform);

	collider = owner->getComponent<CircleCollider>();
	if (collider == nullptr) collider = owner->getComponent<BoxCollider>();
	assert(collider);

	b2BodyDef bodyDef;
	bodyDef.position = b2Vec2(transform->getPosition().x, transform->getPosition().y);
	bodyDef.type = b2_dynamicBody;
	body = world.CreateBody(&bodyDef);
	body->CreateFixture(&collider->getFixtureDef());
}

Rigidbody::~Rigidbody()
{
}

void Rigidbody::update()
{
	transform->setPosition(glm::vec3(body->GetPosition().x, body->GetPosition().y, 0.0f));
	transform->setRotation(body->GetAngle(), glm::vec3(0.0f, 0.0f, 1.0f));
}