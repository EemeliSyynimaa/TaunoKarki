#include "machinegun.h"

MachineGun::MachineGun(GameObjectManager& gameObjectManager) : Weapon(gameObjectManager), fireRate(100.0f), lastShot(0)
{
	damage = 10.0f;
	speed = 0.3f;
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
		float finalSpeed = speed;
		glm::vec2 dirVec;
		
		angle += randomFloat(-0.05f, 0.05f)(randomGenerator);
		finalSpeed *= randomFloat(0.85f, 1.15f)(randomGenerator);
		dirVec.x = glm::cos(angle);
		dirVec.y = glm::sin(angle);

		owner->gameObjectManager.createBullet(owner->getComponent<Transform>()->getPosition(), dirVec, ownero, damage, finalSpeed);
	}
}