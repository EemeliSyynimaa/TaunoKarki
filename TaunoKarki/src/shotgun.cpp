#include "shotgun.h"

Shotgun::Shotgun(GameObjectManager& gameObjectManager) : Weapon(gameObjectManager), lastShot(SDL_GetTicks()), fired(false), fireRate(750.0f), numberOfShells(12)
{
}

Shotgun::~Shotgun()
{
}

void Shotgun::update()
{
	if (triggerPulled && !fired && (SDL_GetTicks() - lastShot) > fireRate)
	{
		std::random_device randomDevice;
		std::default_random_engine randomGenerator(randomDevice());

		size_t ownero = ENEMY_BULLET;;
		if (owner->getType() == PLAYER) ownero = PLAYER_BULLET;

		fired = true;

		for (size_t i = 0; i < numberOfShells; i++)
		{
			glm::vec2 dirVec = owner->getComponent<Transform>()->getDirVec();

			dirVec.x *= randomFloat(0.750f, 1.250f)(randomGenerator);
			dirVec.y *= randomFloat(0.750f, 1.250f)(randomGenerator);

			owner->gameObjectManager.createBullet(owner->getComponent<Transform>()->getPosition(), dirVec, ownero, 25.0f);
		}
	}
	else if (!triggerPulled && fired) fired = false;
}