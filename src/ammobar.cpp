#include "ammobar.h"
#include "playercontroller.h"

AmmoBar::AmmoBar(GameObject* owner) : GuiBar(owner)
{
}

AmmoBar::~AmmoBar()
{
}

void AmmoBar::update()
{
	glm::vec3 position;
	position.x = owner->gameObjectManager.getCamera().getPosition().x + offsetPosition.x;
	position.y = owner->gameObjectManager.getCamera().getPosition().y + offsetPosition.y;
	position.z = offsetPosition.z;
	transform->setPosition(position);

	GameObject* player = owner->gameObjectManager.getFirstObjectOfType(GAMEOBJECT_TYPES::PLAYER);

	if (player)
		transform->setScale(glm::vec3(player->getComponent<PlayerController>()->getWeapon()->getCurrentAmmo() / player->getComponent<PlayerController>()->getWeapon()->getClipSize(), 1.0f, 1.0f));
}