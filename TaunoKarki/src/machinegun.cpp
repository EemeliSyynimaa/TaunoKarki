#include "machinegun.h"

MachineGun::MachineGun(GameObjectManager& gameObjectManager) : Weapon(gameObjectManager), fireRate(100.0f), lastShot(SDL_GetTicks())
{
}

MachineGun::~MachineGun()
{
}

void MachineGun::update()
{
	if (triggerPulled && (SDL_GetTicks() - lastShot) > fireRate)
	{
		std::random_device randomDevice;
		std::default_random_engine randomGenerator(randomDevice());

		lastShot = SDL_GetTicks();
		size_t ownero = ENEMY_BULLET;;
		if (owner->getType() == PLAYER) ownero = PLAYER_BULLET;

		glm::vec2 dirVec = owner->getComponent<Transform>()->getDirVec();

		dirVec.x *= randomFloat(0.875f, 1.125f)(randomGenerator);
		dirVec.y *= randomFloat(0.875f, 1.125f)(randomGenerator);

		owner->gameObjectManager.createBullet(owner->getComponent<Transform>()->getPosition(), dirVec, ownero, 10.0f);
	}
}