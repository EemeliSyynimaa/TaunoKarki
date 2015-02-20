#include "gameobject.h"

GameObject::GameObject()
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

void GameObject::update()
{
	for (auto component : components)
		component->update();
}

void GameObject::draw()
{
	// TODO WHAT
}