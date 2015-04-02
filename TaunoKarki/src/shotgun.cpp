#include "shotgun.h"

Shotgun::Shotgun(GameObjectManager& gameObjectManager) : Weapon(gameObjectManager), lastShot(0), fired(false), fireRate(500.0f), numberOfShells(12)
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
			float angle = glm::atan(owner->getComponent<Transform>()->getDirVec().y, owner->getComponent<Transform>()->getDirVec().x);

			angle += randomFloat(-0.125f, 0.125f)(randomGenerator);
			
			glm::vec2 dirVec;
			dirVec.x = glm::cos(angle);
			dirVec.y = glm::sin(angle);

			owner->gameObjectManager.createBullet(owner->getComponent<Transform>()->getPosition(), dirVec, ownero, 10.0f);
		}

		lastShot = SDL_GetTicks();
	}
	else if (!triggerPulled && fired) fired = false;
}