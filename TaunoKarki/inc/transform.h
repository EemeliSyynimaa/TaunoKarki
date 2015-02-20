#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "glm\glm.hpp"
#include "component.h"

class Transform : public Component
{
public:
	Transform(GameObject* owner);
	~Transform();

	void translate(glm::vec3& vec);
	void rotate(float r, glm::vec3& axis);

	void update();
	const glm::vec3 getPosition() const { return position; }
	const glm::mat4 getMatrix() const { return transform * rotation * scale; }
private:
	glm::vec3 position;
	glm::mat4 transform;
	glm::mat4 scale;
	glm::mat4 rotation;
};

#endif