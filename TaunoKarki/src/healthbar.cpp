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
	position.x = owner->gameObjectManager.getCamera().getPosition().x + offsetPosition.x;
	position.y = owner->gameObjectManager.getCamera().getPosition().y + offsetPosition.y;
	position.z = offsetPosition.z;
	transform->setPosition(position);

	transform->setScale(glm::vec3(owner->gameObjectManager.getFirstObjectOfType(GAMEOBJECT_TYPES::PLAYER)->getComponent<Health>()->getCurrent() / owner->gameObjectManager.getFirstObjectOfType(GAMEOBJECT_TYPES::PLAYER)->getComponent<Health>()->getMax(), 1.0f, 1.0f));
}