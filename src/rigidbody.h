#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "component.h"
#include "glm\glm.hpp"
#include "transform.h"
#include "collider.h"

class RigidBody : public Component
{
public:
    RigidBody(GameObject* owner);

    void update(game_input* input);
    // b2Body* getBody() const { return body; }
private:
    // b2World& world;
    // b2Body* body;
    Transform* transform;
    Collider* collider;
};

#endif