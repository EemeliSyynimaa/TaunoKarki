#include "gameobject.h"
#include "health.h"

GameObject::GameObject(GameObjectManager& gameObjectManager) : gameObjectManager(gameObjectManager), alive(true)
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
			case ENEMY_BULLET: getComponent<Health>()->change(-25); break;
			case ENEMY: getComponent<Health>()->change(-25); break;
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
			case PLAYER_BULLET: getComponent<Health>()->change(-10); break;
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