#ifndef WEAPON_H
#define WEAPON_H

#include "gameobjectmanager.h"
#include "gameobject.h"
#include "transform.h"
#include "SDL\SDL_timer.h"
#include "assetmanager.h"
#include <random>
#define randomFloat std::uniform_real_distribution<float>

class Weapon
{
public:
	Weapon(GameObjectManager& gameObjectManager) : gameObjectManager(gameObjectManager), owner(nullptr), triggerPulled(false), reloading(false), damage(0.0f), speed(0.0f), clipSize(0.0f), currentAmmo(0.0f), reloadTime(0.0f), fireRate(0.0f), bulletSpread(0.0f), startedReloading(0), type(COLLECTIBLES::NONE) {}
	virtual ~Weapon() {}
	void pullTheTrigger() { triggerPulled = true; }
	void releaseTheTrigger() { triggerPulled = false; }
	void reload() { reloading = true; startedReloading = SDL_GetTicks(); }
	void setOwner(GameObject* owner) { this->owner = owner; }
	float getClipSize() { return clipSize; }
	float getCurrentAmmo() { return currentAmmo; }
	bool isTriggerPulled() { return triggerPulled; }
	bool isReloading() { return reloading; }
	bool canShoot() { return !reloading && currentAmmo > 0; }
	void levelUp()
	{
		damage *= 1.05f;
		clipSize = clipSize * 1.05f;
		fireRate *= 0.95f;
		speed *= 1.025f;
		reloadTime *= 0.95f;
		bulletSpread *= 0.95f;

		reload();
	}
	COLLECTIBLES getType() { return type; }
	virtual Weapon* getCopy() = 0;
	virtual void update() = 0;
protected:
	GameObjectManager& gameObjectManager;
	GameObject* owner;
	bool triggerPulled;
	bool reloading;
	float damage;
	float speed;
	float clipSize;
	float currentAmmo;
	float reloadTime;
	float fireRate;
	float bulletSpread;
	COLLECTIBLES type;
	Uint32 startedReloading;
};

#endif