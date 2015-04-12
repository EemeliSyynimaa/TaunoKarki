#include "pistol.h"

Pistol::Pistol(GameObjectManager& gameObjectManager) : Weapon(gameObjectManager), fired(false)
{
	damage = 25.0f;
	speed = 0.3f;
	clipSize = 12.0f;
	currentAmmo = clipSize;
	reloadTime = 1500.0f;
	bulletSpread = 0.01f;
	type = COLLECTIBLES::PISTOL;
}

Pistol::~Pistol()
{
}

void Pistol::update()
{
	if (triggerPulled && currentAmmo > 0.0f && !reloading && !fired)
	{
		std::random_device randomDevice;
		std::default_random_engine randomGenerator(randomDevice());
		glm::vec2 dirVec;

		size_t ownero = ENEMY_BULLET;;
		if (owner->getType() == PLAYER) ownero = PLAYER_BULLET;

		float angle = glm::atan(owner->getComponent<Transform>()->getDirVec().y, owner->getComponent<Transform>()->getDirVec().x);
		
		angle += randomFloat(-bulletSpread, bulletSpread)(randomGenerator);
		dirVec.x = glm::cos(angle);
		dirVec.y = glm::sin(angle);

		owner->gameObjectManager.createBullet(owner->getComponent<Transform>()->getPosition(), dirVec, ownero, damage, speed);
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