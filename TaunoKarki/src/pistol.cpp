#include "pistol.h"

Pistol::Pistol(GameObjectManager& gameObjectManager) : Weapon(gameObjectManager), fired(false)
{
	damage = 25.0f;
	speed = 0.3f;
	clipSize = 12.0f;
	currentAmmo = clipSize;
}

Pistol::~Pistol()
{

}

void Pistol::update()
{
	if (triggerPulled && !fired)
	{
		size_t ownero = ENEMY_BULLET;;
		if (owner->getType() == PLAYER) ownero = PLAYER_BULLET;

		owner->gameObjectManager.createBullet(owner->getComponent<Transform>()->getPosition(), owner->getComponent<Transform>()->getDirVec(), ownero, damage, speed);
		fired = true;
		currentAmmo--;
	}
	else if (!triggerPulled && fired) fired = false;
}