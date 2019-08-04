#include "rigidbody.h"
#include "gameobject.h"
#include "circlecollider.h"
#include "boxcollider.h"

RigidBody::RigidBody(GameObject* owner) : 
    Component(owner),
    has_velocity(0),
    velocity(0)
{
    transform = owner->getComponent<Transform>();
    assert(transform);

    collider = owner->getComponent<Collider>();
    assert(collider);
}

void RigidBody::update(game_input* input)
{
    (void)input;

    if (has_velocity)
    {
        transform->setPosition(transform->getPosition() + velocity);
    }
}