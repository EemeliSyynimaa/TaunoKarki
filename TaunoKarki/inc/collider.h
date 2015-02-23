#ifndef COLLIDER_H
#define COLLIDER_H

#include "component.h"
#include "Box2D\Box2D.h"

class Collider : public Component
{
public:
	Collider(GameObject* owner) : Component(owner) {}
	~Collider() {};

	void update() {};
	b2FixtureDef& getFixtureDef() { return fixtureDef; }

protected:
	b2FixtureDef fixtureDef;
};

#endif
