#include "staticbody.h"
#include "gameobject.h"
#include "circlecollider.h"
#include "boxcollider.h"

StaticBody::StaticBody(GameObject* owner, b2World& world) : Component(owner), world(world)
{
    transform = owner->getComponent<Transform>();
    assert(transform);

    collider = owner->getComponent<CircleCollider>();
    if (collider == nullptr) collider = owner->getComponent<BoxCollider>();
    assert(collider);

    b2BodyDef bodyDef;
    bodyDef.position = b2Vec2(transform->getPosition().x, transform->getPosition().y);
    bodyDef.type = b2_staticBody;
    body = world.CreateBody(&bodyDef);
    body->CreateFixture(&collider->getFixtureDef());
    body->SetUserData(owner);
}

StaticBody::~StaticBody()
{
    world.DestroyBody(body);
}

void StaticBody::update(game_input* input)
{
    (void)input;
}