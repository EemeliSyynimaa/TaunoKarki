#include "gameobject.h"

GameObject::GameObject(GameObjectManager& gameObjectManager) : gameObjectManager(gameObjectManager)
{
}

GameObject::~GameObject()
{
	for (auto component : components)
		delete component;

	components.clear();
}

void GameObject::addComponent(Component* component)
{
	components.push_back(component);
}

void GameObject::update(float deltaTime)
{
	for (auto component : components)
		component->update(deltaTime);
}

void GameObject::draw()
{
	// TODO WHAT
}