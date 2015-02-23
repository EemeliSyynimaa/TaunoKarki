#include "rigidbody.h"
#include "gameobject.h"

Rigidbody::Rigidbody(GameObject* owner, b2World& world) : Component(owner), world(world)
{
	transform = owner->getComponent<Transform>();
	assert(transform);

	b2BodyDef bodyDef;
	bodyDef.position = b2Vec2(transform->getPosition().x, transform->getPosition().y);
	bodyDef.type = b2_dynamicBody;
	body = world.CreateBody(&bodyDef);

	b2PolygonShape shape;
	shape.SetAsBox(1.0f, 1.0f);

	b2FixtureDef fixtureDef;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.0f;
	fixtureDef.shape = &shape;
	body->CreateFixture(&fixtureDef);
}

Rigidbody::~Rigidbody()
{
}

void Rigidbody::update()
{
	transform->setPosition(glm::vec3(body->GetPosition().x, body->GetPosition().y, 0.0f));
	transform->setRotation(body->GetAngle(), glm::vec3(0.0f, 0.0f, 1.0f));
}