#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <vector>
#include "component.h"


class GameObject
{
public:
	GameObject();
	~GameObject();

	void addComponent(Component* component);
	void update();

	b2Vec3 position;
	b2Vec3 rotation;
	b2Vec3 scale;
private:
	std::vector<Component*> components;
};

#endif