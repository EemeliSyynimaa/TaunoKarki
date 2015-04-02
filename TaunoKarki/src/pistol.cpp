#include "pistol.h"

Pistol::Pistol(GameObjectManager& gameObjectManager) : Weapon(gameObjectManager), fired(false)
{
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

		owner->gameObjectManager.createBullet(owner->getComponent<Transform>()->getPosition(), owner->getComponent<Transform>()->getDirVec(), ownero, 25.0f);
		fired = true;
	}
	else if (!triggerPulled && fired) fired = false;
}