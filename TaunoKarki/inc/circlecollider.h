#ifndef CIRCLECOLLIDER_H
#define CIRCLECOLLIDER_H

#include "collider.h"

class CircleCollider : public Collider
{
public:
	CircleCollider(GameObject* owner, float radius);
	~CircleCollider();
private:
	b2CircleShape shape;
};

#endif