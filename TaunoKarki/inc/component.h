#ifndef COMPONENT_H
#define COMPONENT_H

class GameObject;

class Component
{
public:
	bool dead;
	bool enabled;

	Component();
	virtual ~Component();

	virtual void update() = 0;
	void setOwner(GameObject* gameObject) { owner = gameObject; }
	GameObject* getOwner() { return owner; }
protected:
	GameObject* owner;
};

#endif