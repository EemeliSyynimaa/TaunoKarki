#include "component.h"
#include <iostream>

Component::Component()
{
}

Component::~Component()
{
	std::cout << "COMPONENT IS DIE" << std::endl;
}