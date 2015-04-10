#include "pistol.h"

Pistol::Pistol(GameObjectManager& gameObjectManager) : Weapon(gameObjectManager), fired(false)
{
	damage = 25.0f;
	speed = 0.3f;
	clipSize = 12.0f;
	currentAmmo = clipSize;
	reloadTime = 750.0f;
}

Pistol::~Pistol()
{

}

void Pistol::update()
{
	if (triggerPulled && currentAmmo > 0.0f && !reloading && !fired)
	{
		size_t ownero = ENEMY_BULLET;;
		if (owner->getType() == PLAYER) ownero = PLAYER_BULLET;

		owner->gameObjectManager.createBullet(owner->getComponent<Transform>()->getPosition(), owner->getComponent<Transform>()->getDirVec(), ownero, damage, speed);
		fired = true;
		if (--currentAmmo <= 0.0f) reload();
	}
	else if (!triggerPulled && fired) fired = false;
	else if (reloading)
	{
		Uint32 deltaTime = SDL_GetTicks() - startedReloading;
		if (deltaTime > reloadTime)
		{
			reloading = false;
			currentAmmo = clipSize;
		}
		else currentAmmo = clipSize * (deltaTime / reloadTime);
	}
}