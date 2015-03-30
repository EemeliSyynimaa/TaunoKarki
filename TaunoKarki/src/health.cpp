#include "health.h"
#include "gameobject.h"

Health::Health(GameObject* owner, float max) : Component(owner), max(max), current(max)
{
}

Health::~Health()
{
}

void Health::update(float deltaTime)
{
}

void Health::change(float amount)
{
	current += amount;
	if (current > max) current = max;
	else if (current <= 0) owner->kill();
}