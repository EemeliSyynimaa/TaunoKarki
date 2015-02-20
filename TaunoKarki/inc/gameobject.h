#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <vector>
#include <algorithm>
#include "component.h"


class GameObject
{
public:
	GameObject();
	~GameObject();

	void addComponent(Component* component);

	template<class T> 
	T* const getComponent() const {
		T* foundComponent = nullptr;

		std::find_if(components.begin(), components.end(), [=, &foundComponent](Component* component) {
			foundComponent = dynamic_cast<T*>(component);

			return foundComponent != nullptr;
		});

		return foundComponent;
	}

	void update();
	void draw();
private:
	std::vector<Component*> components;
};

#endif