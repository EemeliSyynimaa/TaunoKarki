#include "weapon.h"

Weapon::Weapon(GameObjectManager& gameObjectManager) :
    gameObjectManager(gameObjectManager),
    owner(nullptr),
    triggerPulled(false),
    reloading(false),
    damage(0.0f),
    speed(0.0f),
    clipSize(0.0f),
    currentAmmo(0.0f),
    reloadTime(0.0f),
    fireRate(0.0f),
    bulletSpread(0.0f),
    startedReloading(0),
    type(COLLECTIBLES::NONE)
{
    
}

