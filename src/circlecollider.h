#ifndef CIRCLECOLLIDER_H
#define CIRCLECOLLIDER_H

#include "collider.h"

class CircleCollider : public Collider
{
public:
    CircleCollider(GameObject* owner, float radius, uint16 categoryBits, uint16 maskBits);
    ~CircleCollider();
private:
    b2CircleShape shape;
};

#endif