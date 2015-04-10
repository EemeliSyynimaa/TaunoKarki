#include "shotgun.h"

Shotgun::Shotgun(GameObjectManager& gameObjectManager) : Weapon(gameObjectManager), lastShot(0), fired(false), fireRate(500.0f), numberOfShells(12)
{
	damage = 8.5f;
	speed = 0.3f;
	clipSize = 7.0f;
	currentAmmo = clipSize;
	reloadTime = 2500.0f;
}

Shotgun::~Shotgun()
{
}

void Shotgun::update()
{
	if (triggerPulled && !fired && currentAmmo > 0.0f && !reloading && (SDL_GetTicks() - lastShot) > fireRate)
	{
		std::random_device randomDevice;
		std::default_random_engine randomGenerator(randomDevice());

		size_t ownero = ENEMY_BULLET;;
		if (owner->getType() == PLAYER) ownero = PLAYER_BULLET;

		fired = true;

		for (size_t i = 0; i < numberOfShells; i++)
		{
			float angle = glm::atan(owner->getComponent<Transform>()->getDirVec().y, owner->getComponent<Transform>()->getDirVec().x);
			float finalSpeed = speed;
			angle += randomFloat(-0.125f, 0.125f)(randomGenerator);
			finalSpeed *= randomFloat(0.9f, 1.1f)(randomGenerator);

			glm::vec2 dirVec;
			dirVec.x = glm::cos(angle);
			dirVec.y = glm::sin(angle);

			owner->gameObjectManager.createBullet(owner->getComponent<Transform>()->getPosition(), dirVec, ownero, damage, finalSpeed);
		}

		lastShot = SDL_GetTicks();
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