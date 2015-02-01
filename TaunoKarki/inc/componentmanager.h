#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include <vector>

template <class T>
class ComponentManager
{
public:
	ComponentManager();
	~ComponentManager();
private:
	std::vector<T> components;
};

#endif