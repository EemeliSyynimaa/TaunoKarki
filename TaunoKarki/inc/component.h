#ifndef COMPONENT_H
#define COMPONENT_H

#include "Box2D\Common\b2Math.h"
class GameObject;

class Component
{
public:
	Component();
	virtual ~Component();

	virtual void update() = 0;
	void setOwner(GameObject* gameObject) { owner = gameObject; }
	GameObject* getOwner() { return owner; }
protected:
	GameObject* owner;
};

#endif