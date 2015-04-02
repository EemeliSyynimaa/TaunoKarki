#ifndef WEAPON_H
#define WEAPON_H

#include "gameobjectmanager.h"
#include "gameobject.h"
#include "transform.h"
#include "SDL\SDL_timer.h"
#include <random>
#define randomFloat std::uniform_real_distribution<float>

class Weapon
{
public:
	Weapon(GameObjectManager& gameObjectManager) : gameObjectManager(gameObjectManager), owner(nullptr), triggerPulled(false) {}
	virtual ~Weapon() {}
	void pullTheTrigger() { triggerPulled = true; }
	void releaseTheTrigger() { triggerPulled = false; }
	void setOwner(GameObject* owner) { this->owner = owner; }
	bool isTriggerPulled() { return triggerPulled; }
	virtual void update() = 0;
protected:
	GameObjectManager& gameObjectManager;
	GameObject* owner;
	bool triggerPulled;
};

#endif