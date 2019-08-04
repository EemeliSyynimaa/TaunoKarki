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
    b32 has_velocity;
    glm::vec3 velocity;
private:
    Transform* transform;
    Collider* collider;
};

#endif