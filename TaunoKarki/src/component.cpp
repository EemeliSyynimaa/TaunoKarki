#include "component.h"
#include <iostream>

Component::Component() : alive(false), enabled(false)
{
	std::cout << "COMPONENT IS HERE" << std::endl;
}

Component::~Component()
{
	std::cout << "COMPONENT IS DIE" << std::endl;
}