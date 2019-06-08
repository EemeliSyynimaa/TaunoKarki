#ifndef BOXCOLLIDER_H
#define BOXCOLLIDER_H

#include "collider.h"

class BoxCollider : public Collider
{
public:
    BoxCollider(GameObject* owner, float width, float height, uint16 categoryBits, uint16 maskBits);
    ~BoxCollider();
private:
    b2PolygonShape shape;
};

#endif