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

void GameObject::update(game_input* input)
{
    for (auto component : components)
        component->update(input);
}

void GameObject::draw()
{
    for (auto component : drawableComponents)
        component->update(0);
}

void GameObject::handleCollisionWith(GameObject* gameObject)
{
    switch (type)
    {
    case PLAYER:
    {
        if (gameObject)
        {
            PlayerController* controller = getComponent<PlayerController>();
            
            switch (gameObject->getType())
            {
            case ENEMY_BULLET:
            case ENEMY:
            {
                getComponent<Health>()->change(-gameObject->getComponent<Damage>()->getDamage());
            } break;
            case ITEM: 
            {
                controller->handleItem(gameObject->getComponent<Collectible>()->getType());
            } break;
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
                AIController* controller = getComponent<AIController>();

                getComponent<Health>()->change(-gameObject->getComponent<Damage>()->getDamage());

                if (!isAlive() && !controller->droppedItem)
                {
                    gameObjectManager.addNewObject([this]()
                    {
                        this->gameObjectManager.createRandomItem(this->getComponent<Transform>()->getPosition());
                    });

                    getComponent<AIController>()->droppedItem = true;
                }

                getComponent<AIController>()->gotShot(gameObject->getComponent<Transform>()->getPosition());
            } 
            break;
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