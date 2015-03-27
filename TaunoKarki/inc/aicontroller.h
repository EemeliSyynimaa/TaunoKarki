#ifndef AICONTROLLER_H
#define AICONTROLLER_H

#include "component.h"

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
};

#endif