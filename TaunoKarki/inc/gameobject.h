#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <map>
#include "component.h"


class GameObject
{
public:
	GameObject();
	~GameObject();

	void addComponent(std::string tag, Component* component);
	Component* getComponent(std::string tag);
	void removeComponent(std::string tag);
private:
	std::map<std::string, Component*> components;
};

#endif