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
	virtual void reset() = 0;

	void setOwner(GameObject* gameObject) { owner = gameObject; }
	GameObject* getOwner() { return owner; }

	bool isAlive() { return alive; }
	void kill() { alive = false; }
	void revive() { alive = true; }

	bool isEnabled() { return enabled; }
	void enable() { enabled = true; }
	void disable() { enabled = false; }
protected:
	GameObject* owner;
	bool alive;
	bool enabled;
};

#endif