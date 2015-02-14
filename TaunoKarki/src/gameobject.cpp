#include "gameobject.h"

GameObject::GameObject()
{
}

GameObject::~GameObject()
{
	components.clear();
}

void GameObject::addComponent(Component* component)
{
	components.push_back(component);
}