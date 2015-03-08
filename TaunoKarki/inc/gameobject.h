#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <vector>
#include <algorithm>
#include "component.h"
#include "gameobjectmanager.h"

const enum GAMEOBJECT_TYPES
{
	PLAYER,
	ENEMY,
	PLAYER_BULLET,
	ENEMY_BULLET,
	WALL
};

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

	GameObjectManager& gameObjectManager;
	bool isAlive() { return alive; }
	void kill() { alive = false; }
	unsigned int getType() { return type; }
	void setType(unsigned int type) { this->type = type; }
private:
	std::vector<Component*> components;
	bool alive;
	unsigned int type;
};

#endif