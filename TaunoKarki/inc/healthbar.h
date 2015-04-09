#ifndef GUIBAR_H
#define GUIBAR_H

#include "component.h"
#include "transform.h"

class HealthBar : public Component
{
public:
	HealthBar(GameObject* owner);
	~HealthBar();

	void setOffsetPosition(glm::vec3 pos) { offsetPosition = pos; }
	void update();
private:
	Transform* transform;
	glm::vec3 offsetPosition;
};

#endif