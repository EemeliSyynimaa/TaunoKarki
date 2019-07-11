#include "shotgun.h"
#include "locator.h"

Shotgun::Shotgun(GameObjectManager& gameObjectManager) :
    Weapon(gameObjectManager),
    lastShot(0),
    fired(false),
    numberOfShells(GLOBALS::SHOTGUN_NUMBER_OF_SHELLS)
{
    damage = GLOBALS::SHOTGUN_DAMAGE;
    speed = GLOBALS::SHOTGUN_BULLET_SPEED;
    clipSize = GLOBALS::SHOTGUN_CLIP_SIZE;
    currentAmmo = clipSize;
    reloadTime = GLOBALS::SHOTGUN_RELOAD_TIME;
    fireRate = GLOBALS::SHOTGUN_FIRE_RATE;
    bulletSpread = GLOBALS::SHOTGUN_BULLET_SPREAD;
    type = COLLECTIBLES::SHOTGUN;
}

Shotgun::~Shotgun()
{
}

void Shotgun::update()
{
    if (triggerPulled && !fired && currentAmmo > 0.0f && !reloading &&
        (tk_current_time_get() - lastShot) > fireRate)
    {
        tk_sound_play(Locator::getAssetManager()->shotgunBangSound);

        std::random_device randomDevice;
        std::default_random_engine randomGenerator(randomDevice());

        size_t ownero = ENEMY_BULLET;;
        if (owner->getType() == PLAYER) ownero = PLAYER_BULLET;

        fired = true;

        for (size_t i = 0; i < numberOfShells; i++)
        {
            float angle = glm::atan(
                owner->getComponent<Transform>()->getDirVec().y,
                owner->getComponent<Transform>()->getDirVec().x);
            float finalSpeed = speed;
            angle += randomFloat(-bulletSpread, bulletSpread)(randomGenerator);
            finalSpeed *= randomFloat(0.9f, 1.1f)(randomGenerator);

            glm::vec2 dirVec;
            dirVec.x = glm::cos(angle);
            dirVec.y = glm::sin(angle);

            owner->gameObjectManager.createBullet(
                owner->getComponent<Transform>()->getPosition(),
                dirVec, ownero, damage, finalSpeed);
        }

        lastShot = tk_current_time_get();
        if (--currentAmmo <= 0.0f) reload();
    }
    else if (!triggerPulled && fired) fired = false;
    else if (reloading)
    {
        Uint32 deltaTime = tk_current_time_get() - startedReloading;
        if (deltaTime > reloadTime)
        {
            reloading = false;
            currentAmmo = clipSize;
        }
        else currentAmmo = clipSize * (deltaTime / reloadTime);
    }
}