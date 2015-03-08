#ifndef HEALTH_H
#define HEALTH_H

#include "component.h"

class Health : public Component
{
public:
	Health(GameObject* owner, int max);
	~Health();

	void update(float deltaTime);
	void change(int amount);
private:
	int current;
	int max;
};

#endif