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
	//Component* getComponent(id); JOKU TÄLLÄNEN OIS KIVA 
	//void removeComponent(std::string tag);
	void update();
private:
	std::vector<Component*> components;
};

#endif