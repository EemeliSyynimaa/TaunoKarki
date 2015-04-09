#ifndef GUIBAR_H
#define GUIBAR_H

#include "component.h"
#include "transform.h"

class HealthBar : public Component
{
public:
	HealthBar(GameObject* owner);
	~HealthBar();

	void update();
private:
	Transform* transform;
};

#endif