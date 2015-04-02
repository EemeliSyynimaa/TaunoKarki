#include "machinegun.h"

MachineGun::MachineGun(GameObjectManager& gameObjectManager) : Weapon(gameObjectManager), fireRate(100.0f), lastShot(0)
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

		float angle = glm::atan(owner->getComponent<Transform>()->getDirVec().y, owner->getComponent<Transform>()->getDirVec().x);
		glm::vec2 dirVec;

		angle += randomFloat(-0.05, 0.05)(randomGenerator);
		dirVec.x = glm::cos(angle);
		dirVec.y = glm::sin(angle);

		owner->gameObjectManager.createBullet(owner->getComponent<Transform>()->getPosition(), dirVec, ownero, 10.0f);
	}
}