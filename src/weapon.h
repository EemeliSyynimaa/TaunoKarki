#ifndef WEAPON_H
#define WEAPON_H

#include "gameobjectmanager.h"
#include "gameobject.h"
#include "transform.h"
#include <random>
#define randomFloat std::uniform_real_distribution<float>

class Weapon
{
public:
    Weapon(GameObjectManager& gameObjectManager);
    virtual ~Weapon() {}
    void pullTheTrigger() { triggerPulled = true; }
    void releaseTheTrigger() { triggerPulled = false; }
    void reload(bool instantReload = false) 
    { 
        if (instantReload)
        {
            currentAmmo = clipSize;
        }
        else
        {
            reloading = reload_time;
        }
    }
    void setOwner(GameObject* owner) { this->owner = owner; }
    float getClipSize() { return clipSize; }
    float getCurrentAmmo() { return currentAmmo; }
    bool isTriggerPulled() { return triggerPulled; }
    bool isReloading() { return reloading > 0; }
    bool canShoot() { return !reloading && currentAmmo > 0; }
    void levelUp(int level = 1, bool instantReload = false)
    {
        for (int i = 0; i < level; i++)
        {
            damage *= 1.05f;
            clipSize *= 1.05f;
            fireRate *= 0.95f;
            speed *= 1.025f;
            reload_time *= 0.95;
            bulletSpread *= 0.95f;
        }

        reload(instantReload);
    }
    COLLECTIBLES getType() { return type; }
    virtual Weapon* getCopy() = 0;
    virtual void update(f32 delta_time) = 0;
protected:
    GameObjectManager& gameObjectManager;
    GameObject* owner;
    bool triggerPulled;
    float damage;
    float speed;
    float clipSize;
    float currentAmmo;
    float fireRate;
    float bulletSpread;
    COLLECTIBLES type;
    f32 reload_time;
    f32 reloading;
};

#endif