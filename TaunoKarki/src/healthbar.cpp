#include "healthbar.h"
#include "gameobject.h"
#include "health.h"

HealthBar::HealthBar(GameObject* owner) : Component(owner), transform(owner->getComponent<Transform>())
{
	assert(transform);
}

HealthBar::~HealthBar()
{
}

void HealthBar::update()
{
	glm::vec3 position;
	position.x = owner->gameObjectManager.getCamera().getPosition().x - 5.0f;
	position.y = owner->gameObjectManager.getCamera().getPosition().y - 7.0f;
	position.z = 3;
	transform->setPosition(position);

	transform->setScale(glm::vec3(owner->gameObjectManager.getFirstObjectOfType(GAMEOBJECT_TYPES::PLAYER)->getComponent<Health>()->getCurrent() / owner->gameObjectManager.getFirstObjectOfType(GAMEOBJECT_TYPES::PLAYER)->getComponent<Health>()->getMax(), 1.0f, 1.0f));
}