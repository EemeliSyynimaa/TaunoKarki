#include "healthbar.h"
#include "gameobject.h"
#include "health.h"

HealthBar::HealthBar(GameObject* owner) : GuiBar(owner)
{
}

HealthBar::~HealthBar()
{
}

void HealthBar::update(tk_state_player_input_t* input)
{
    (void)input;
    glm::vec3 position;
    position.x = owner->gameObjectManager.getCamera().getPosition().x + offsetPosition.x;
    position.y = owner->gameObjectManager.getCamera().getPosition().y + offsetPosition.y;
    position.z = offsetPosition.z;
    transform->setPosition(position);

    GameObject* player = owner->gameObjectManager.getFirstObjectOfType(GAMEOBJECT_TYPES::PLAYER);

    if (player && player->getComponent<Health>()->getCurrent() >= 0.0f)
        transform->setScale(glm::vec3(player->getComponent<Health>()->getCurrent() / player->getComponent<Health>()->getMax(), 1.0f, 1.0f));
    else
        transform->setScale(glm::vec3(0.001f, 1.0f, 1.0f));
}