#ifndef AICONTROLLER_H
#define AICONTROLLER_H

#include <vector>
#include "component.h"
#include "transform.h"

class AIController : public Component
{
public:
	AIController(GameObject* owner);
	~AIController();

	void update(float deltaTime);
private:

	void wander();
	void attack();
	void pursue();
	void escape();

	enum states
	{
		WANDER,
		ATTACK,
		PURSUE,
		ESCAPE
	};

	states state;

	// The current target player
	GameObject* targetPlayer;

	Transform* transform;
};

#endif