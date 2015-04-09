#ifndef STATICBODY_H
#define STATICBODY_H

#include "component.h"
#include "Box2D\Box2D.h"
#include "transform.h"
#include "collider.h"

class StaticBody : public Component
{
public:
	StaticBody(GameObject* owner, b2World& world);
	~StaticBody();

	void update( );
private:
	b2Body* body;
	b2World& world;
	Transform* transform;
	Collider* collider;
};

#endif