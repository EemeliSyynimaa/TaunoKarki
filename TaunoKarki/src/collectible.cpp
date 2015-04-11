#include "collectible.h"
#include "gameobject.h"


Collectible::Collectible(GameObject* owner) : Component(owner), type(COLLECTIBLES::NONE), transform(owner->getComponent<Transform>())
{
	assert(transform);
}

Collectible::~Collectible()
{
}

void Collectible::update()
{
	transform->rotate(0.07f, glm::vec3(0.0f, 0.0f, -1.0f));
}