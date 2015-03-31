#include "gameobject.h"
#include "health.h"
#include "damage.h"

GameObject::GameObject(GameObjectManager& gameObjectManager) : gameObjectManager(gameObjectManager), alive(true)
{
}

GameObject::~GameObject()
{
	for (auto component : components)
		delete component;

	for (auto component : drawableComponents)
		delete component;

	components.clear();
	drawableComponents.clear();
}

void GameObject::addComponent(Component* component)
{
	components.push_back(component);
}

void GameObject::addDrawableComponent(Component* component)
{
	drawableComponents.push_back(component);
}

void GameObject::update(float deltaTime)
{
	for (auto component : components)
		component->update(deltaTime);
}

void GameObject::draw()
{
	for (auto component : drawableComponents)
		component->update(0.0f);
}

void GameObject::handleCollisionWith(GameObject* gameObject)
{
	switch (type)
	{
	case PLAYER:
	{
		if (gameObject)
		{
			switch (gameObject->getType())
			{
			case ENEMY_BULLET: getComponent<Health>()->change(-gameObject->getComponent<Damage>()->getDamage()); break;
			case ENEMY: getComponent<Health>()->change(-gameObject->getComponent<Damage>()->getDamage()); break;
			default: break;
		}
	}
		break;
	}
	case ENEMY:
	{
		if (gameObject)
		{
			switch (gameObject->getType())
			{
			case PLAYER_BULLET: getComponent<Health>()->change(-gameObject->getComponent<Damage>()->getDamage()); break;
			default: break;
			}
		}
		break;
	}
	case PLAYER_BULLET:
	{
		if (gameObject)
		{
			switch (gameObject->getType())
			{
			case PLAYER: break;
			default: break;
			}
		} 
		kill();
		break;
	}
	case ENEMY_BULLET:
	{
		if (gameObject)
		{
			switch (gameObject->getType())
			{
			case ENEMY: break;
			default: break;
			}
		}
		kill();
		break;
	}
	default: break;
	}
}