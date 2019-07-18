#include "weapon.h"

Weapon::Weapon(GameObjectManager& gameObjectManager) :
    gameObjectManager(gameObjectManager),
    owner(nullptr),
    triggerPulled(false),
    damage(0.0f),
    speed(0.0f),
    clipSize(0.0f),
    currentAmmo(0.0f),
    fireRate(0.0f),
    bulletSpread(0.0f),
    type(COLLECTIBLES::NONE),
    reload_time(0),
    reloading(0)
{
    
}

