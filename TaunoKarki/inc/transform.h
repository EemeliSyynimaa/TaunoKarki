#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "glm\glm.hpp"
#include "component.h"

class Transform : public Component
{
public:
	Transform(GameObject* owner);
	~Transform();


	void update();
	const glm::mat4 getMatrix() const { return position * rotation * scale; }
private:
	glm::mat4 position;
	glm::mat4 scale;
	glm::mat4 rotation;
};

#endif