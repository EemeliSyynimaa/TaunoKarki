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
	AIController(GameObject* owner, Tilemap* tilemap, b2World* world);
	~AIController();

	void update();
	void giveWeapon(Weapon* weapon) { this->weapon = weapon; this->weapon->setOwner(this->owner); this->weapon->reload(true); }
	bool droppedItem;
	Weapon* getWeapon() { return weapon; }
	void getNewTarget();

	void initWander();
	void initAttack();
	void initPursue();
	void initEscape();
private:

	class RayCastCallback : public b2RayCastCallback
	{
	public:
		RayCastCallback() : playerIsVisible(true) {}
		bool playerIsVisible;
		float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction);
	};

	void wander();
	void attack();
	void pursue();
	void escape();

	void shoot();
	void moveTo(glm::vec3 position);

	bool isPlayerInSight(GameObject* player);

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
	b2World* world;

	glm::vec3 target;

	Uint32 lastShot;
	float moveSpeed;
};

#endif