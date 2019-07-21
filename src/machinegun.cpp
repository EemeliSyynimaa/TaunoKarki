#include "machinegun.h"
#include "locator.h"

MachineGun::MachineGun(GameObjectManager& gameObjectManager) :
    Weapon(gameObjectManager),
    lastShot(0)
{
    damage = GLOBALS::MACHINEGUN_DAMAGE;
    speed = GLOBALS::MACHINEGUN_BULLET_SPEED;
    clipSize = GLOBALS::MACHINEGUN_CLIP_SIZE;
    currentAmmo = clipSize;
    reload_time = GLOBALS::MACHINEGUN_RELOAD_TIME;
    fireRate = GLOBALS::MACHINEGUN_FIRE_RATE;
    bulletSpread = GLOBALS::MACHINEGUN_BULLET_SPREAD;
    type = COLLECTIBLES::MACHINEGUN;
}

MachineGun::~MachineGun()
{
}

void MachineGun::update(f32 delta_time)
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
    else if (lastShot > 0)
    {
        lastShot -= delta_time;
    }
    else if (triggerPulled && currentAmmo > 0.0f)
    {
        std::random_device randomDevice;
        std::default_random_engine randomGenerator(randomDevice());

        lastShot = fireRate;

        size_t ownero = ENEMY_BULLET;
        if (owner->getType() == PLAYER) ownero = PLAYER_BULLET;

        float angle = glm::atan(
            owner->getComponent<Transform>()->getDirVec().y,
            owner->getComponent<Transform>()->getDirVec().x);
        float finalSpeed = speed;
        glm::vec2 dirVec;
        
        angle += randomFloat(-bulletSpread, bulletSpread)(randomGenerator);
        finalSpeed *= randomFloat(0.85f, 1.15f)(randomGenerator);
        dirVec.x = glm::cos(angle);
        dirVec.y = glm::sin(angle);

        owner->gameObjectManager.createBullet(
            owner->getComponent<Transform>()->getPosition(),
            dirVec, ownero, damage, finalSpeed);
        
        if (--currentAmmo <= 0.0f) reload();
    }
}