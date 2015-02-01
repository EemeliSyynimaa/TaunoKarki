#include "gameobject.h"

GameObject::GameObject()
{
}

GameObject::~GameObject()
{
}

void GameObject::addComponent(std::string tag, Component* component)
{
	component->setOwner(this);
	components.insert(std::pair<std::string, Component*>(tag, component));
}

Component* GameObject::getComponent(std::string tag)
{
	return components[tag];
}

void GameObject::removeComponent(std::string tag)
{
	components.erase(tag);
}