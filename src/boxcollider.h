#ifndef BOXCOLLIDER_H
#define BOXCOLLIDER_H

#include "collider.h"

class BoxCollider : public Collider
{
public:
    BoxCollider(GameObject* owner, float width, float height, u16 categoryBits, u16 maskBits);
    ~BoxCollider();
private:
    // b2PolygonShape shape;
};

#endif