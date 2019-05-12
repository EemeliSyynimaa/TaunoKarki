#ifndef COLLIDER_H
#define COLLIDER_H

#include "component.h"
#include "Box2D\Box2D.h"

class Collider : public Component
{
public:
	Collider(GameObject* owner, uint16 categoryBits, uint16 maskBits) : Component(owner) { fixtureDef.filter.maskBits = maskBits; fixtureDef.filter.categoryBits = categoryBits; }
	~Collider() {};

	void update() {};
	b2FixtureDef& getFixtureDef() { return fixtureDef; }

protected:
	b2FixtureDef fixtureDef;
};

#endif
