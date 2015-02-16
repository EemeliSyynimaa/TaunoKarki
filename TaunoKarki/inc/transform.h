#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "glm\glm.hpp"
#include "component.h"

class Transform : public Component
{
public:
	Transform();
	~Transform();

	void update();
	void reset();

	const glm::vec3& getPosition() { return position; }
	const glm::vec3& getRotation() { return rotation; }
	const glm::vec3& getScale() { return scale; }

	const glm::mat4& getOrientation();
	const glm::mat4& getTranslation();
	const glm::mat4& getScaling();

	void setPosition(const glm::vec3& pos);
	void setRotation(const glm::vec3& rot);
private:
	glm::mat4 translation;
	glm::mat4 orientation;
	glm::mat4 scaling;

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	bool needUpdate;
};

#endif