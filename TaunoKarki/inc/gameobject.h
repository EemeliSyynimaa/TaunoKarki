#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <vector>
#include <algorithm>
#include "component.h"
#include "gameobjectmanager.h"

class GameObject
{
public:
	GameObject(GameObjectManager& gameObjectManager);
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

	void update(float deltaTime);
	void draw();
private:
	std::vector<Component*> components;
	GameObjectManager& gameObjectManager;
};

#endif