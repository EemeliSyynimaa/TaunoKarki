#ifndef COMPONENT_H
#define COMPONENT_H

#include "Box2D\Common\b2Math.h"
class GameObject;

class Component
{
public:
	Component(GameObject* owner);
	virtual ~Component();

	virtual void update(float deltaTime) = 0;

	GameObject* getOwner() { return owner; }
protected:
	GameObject* owner;
};

#endif