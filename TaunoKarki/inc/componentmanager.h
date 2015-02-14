#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include <vector>
#include <algorithm>
#include "gameobject.h"

template <class T>
class ComponentManager
{
public:
	ComponentManager(int numberOfComponents)
	{
		for (int i = 0; i < numberOfComponents; i++)
		{
			components.push_back(new T());
		}
	}
	~ComponentManager()
	{
		for (auto& component : components)
		{
			delete component;
		}

		components.clear();
	}

	void update()
	{
		for (auto& component : components)
		{
			if (component->isAlive())
			{
				component->update();
			}
		}
	}

	void addComponent(GameObject* gameobject)
	{
		for (auto& component : components)
		{
			if (!component->isAlive())
			{
				component->setOwner(gameobject);
				component->reset();
				component->revive();
				component->enable();
				gameobject->addComponent(component);
			}
		}
	}
private:

	std::vector<T*> components;
};

#endif