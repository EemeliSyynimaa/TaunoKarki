#include "pistol.h"
#include "locator.h"

Pistol::Pistol(GameObjectManager& gameObjectManager) :
    Weapon(gameObjectManager),
    fired(false)
{
    damage = GLOBALS::PISTOL_DAMAGE;
    speed = GLOBALS::PISTOL_BULLET_SPEED;
    clipSize = GLOBALS::PISTOL_CLIP_SIZE;
    currentAmmo = clipSize;
    reload_time = GLOBALS::PISTOL_RELOAD_TIME;
    bulletSpread = GLOBALS::PISTOL_BULLET_SPREAD;
    type = COLLECTIBLES::PISTOL;
}

Pistol::~Pistol()
{
}

void Pistol::update(f32 delta_time)
{
    if (reloading > 0)
    {
        reloading -= delta_time;

        if (reloading <= 0)
        {
            currentAmmo = clipSize;
        }
        else
        {
            currentAmmo = clipSize * (reload_time - reloading) / reload_time;
        }
    }
    else if (triggerPulled && currentAmmo > 0.0f && !fired)
    {
        std::random_device randomDevice;
        std::default_random_engine randomGenerator(randomDevice());
        glm::vec2 dirVec;

        size_t ownero = ENEMY_BULLET;
        if (owner->getType() == PLAYER) ownero = PLAYER_BULLET;

        float angle = glm::atan(
            owner->getComponent<Transform>()->getDirVec().y, 
            owner->getComponent<Transform>()->getDirVec().x);
        
        angle += randomFloat(-bulletSpread, bulletSpread)(randomGenerator);
        dirVec.x = glm::cos(angle);
        dirVec.y = glm::sin(angle);

        owner->gameObjectManager.createBullet(
            owner->getComponent<Transform>()->getPosition(),
            dirVec, ownero, damage, speed);
        
        fired = true;

        if (--currentAmmo <= 0.0f) reload();
    }
    else if (!triggerPulled && fired)
    {
        fired = false;
    }
}