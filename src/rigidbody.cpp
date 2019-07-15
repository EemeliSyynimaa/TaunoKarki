#include "rigidbody.h"
#include "gameobject.h"
#include "circlecollider.h"
#include "boxcollider.h"

RigidBody::RigidBody(GameObject* owner, b2World& world) : Component(owner), world(world)
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
    body->SetUserData(owner);
}

RigidBody::~RigidBody()
{
    world.DestroyBody(body);
}

void RigidBody::update(game_input* input)
{
    (void)input;
    transform->setPosition(glm::vec3(body->GetPosition().x, body->GetPosition().y, 0.0f));
}