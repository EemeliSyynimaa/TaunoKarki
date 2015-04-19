#ifndef AICONTROLLER_H
#define AICONTROLLER_H

#include <vector>
#include <SDL\SDL_timer.h>
#include "component.h"
#include "transform.h"
#include "weapon.h"
#include "rigidbody.h"

class AIController : public Component
{
public:
	AIController(GameObject* owner, Tilemap* tilemap);
	~AIController();

	void update();
	void giveWeapon(Weapon* weapon) { this->weapon = weapon; this->weapon->setOwner(this->owner); this->weapon->reload(true); }
	bool droppedItem;
	Weapon* getWeapon() { return weapon; }
	void getNewTarget();
private:

	void wander();
	void attack();
	void pursue();
	void escape();

	void initWander();
	void initAttack();
	void initPursue();
	void initEscape();

	void shoot();
	void moveTo(glm::vec3 position);

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
	b2Body* body;
	Tilemap* tilemap;

	glm::vec3 target;

	Uint32 lastShot;
	float moveSpeed;
};

#endif