#ifndef AICONTROLLER_H
#define AICONTROLLER_H

#include <vector>
#include <SDL\SDL_timer.h>
#include "component.h"
#include "transform.h"
#include "weapon.h"

class AIController : public Component
{
public:
	AIController(GameObject* owner);
	~AIController();

	void update();
	void giveWeapon(Weapon* weapon) { this->weapon = weapon; weapon->setOwner(this->owner); }
private:

	void wander();
	void attack();
	void pursue();
	void escape();
	void shoot();

	enum states
	{
		WANDER,
		ATTACK,
		PURSUE,
		ESCAPE
	};

	states state;
	Transform* transform;
	Weapon* weapon;

	Uint32 lastShot;
};

#endif