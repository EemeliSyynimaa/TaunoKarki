#ifndef HEALTH_H
#define HEALTH_H

#include "component.h"

class Health : public Component
{
public:
	Health(GameObject* owner, float max);
	~Health();

	void update(float deltaTime);
	void change(float amount);
private:
	float current;
	float max;
};

#endif