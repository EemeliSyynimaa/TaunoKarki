#include "transform.h"

Transform::Transform()
{
	reset();
}

Transform::~Transform()
{
}

void Transform::update()
{
}

void Transform::reset()
{
	needUpdate = true;
	scale = glm::vec3(1.0f, 1.0f, 1.0f);
}

const glm::mat4& Transform::getOrientation()
{
	return orientation;
}

const glm::mat4& Transform::getTranslation()
{
	return translation;
}

const glm::mat4& Transform::getScaling()
{
	return scaling;
}