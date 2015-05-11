#include "gameobject.h"
#include "health.h"
#include "damage.h"
#include "transform.h"
#include "playercontroller.h"
#include "aicontroller.h"
#include "collectible.h"
#include "locator.h"

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

void GameObject::update()
{
	for (auto component : components)
		component->update();
}

void GameObject::draw()
{
	for (auto component : drawableComponents)
		component->update();
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
			case ENEMY_BULLET:
			case ENEMY:
			{
				getComponent<Health>()->change(-gameObject->getComponent<Damage>()->getDamage());

				if (isAlive())
					getComponent<PlayerController>()->playerAudioChannel = Locator::getAudio()->playSound(Locator::getAssetManager()->playerHitSound, getComponent<PlayerController>()->playerAudioChannel);
			} break;
			case ITEM: getComponent<PlayerController>()->handleItem(gameObject->getComponent<Collectible>()->getType()); Locator::getAudio()->playSound(Locator::getAssetManager()->powerupSound); break;
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
			case PLAYER_BULLET:
			{
				getComponent<Health>()->change(-gameObject->getComponent<Damage>()->getDamage());

				if (!isAlive() && !getComponent<AIController>()->droppedItem)
				{
					getComponent<AIController>()->AIAudioChannel = Locator::getAudio()->playSound(Locator::getAssetManager()->enemyDeadSound, getComponent<AIController>()->AIAudioChannel);
					gameObjectManager.addNewObject([this]()
					{
						this->gameObjectManager.createRandomItem(this->getComponent<Transform>()->getPosition());
					});

					getComponent<AIController>()->droppedItem = true;
				}

				getComponent<AIController>()->gotShot(gameObject->getComponent<Transform>()->getPosition());
			} 
			break;
			case WALL: getComponent<AIController>()->getNewTarget(); break;
			default: break;
			}
		}
		break;
	}
	case PLAYER_BULLET: kill(); break;
	case ENEMY_BULLET: kill(); break;
	case ITEM: kill(); break;
	default: break;
	}
}