#include "component.h"
#include <iostream>
#include "gameobject.h"

Component::Component(GameObject* owner) : owner(owner)
{
	assert(owner);
	std::cout << "COMPONENT IS HERE" << std::endl;
}

Component::~Component()
{
	std::cout << "COMPONENT IS DIE" << std::endl;
}