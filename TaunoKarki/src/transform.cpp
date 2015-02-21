#include "transform.h"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

Transform::Transform(GameObject* owner, float x, float y, float z) : Component(owner), position(x, y, z), scale(1.0f), rotation(1.0f), transform(1.0f)
{
	transform = glm::translate(position);
}

Transform::~Transform()
{
}

void Transform::update()
{
}

void Transform::lookAt(glm::vec3& vec)
{
	glm::vec3 deltaPos = position - vec;
	glm::vec3 axis(0.0f, 0.0f, 1.0f);
	float angle;
	angle = glm::atan(vec.x, vec.y);

	rotation = glm::rotate(angle, axis);
}

void Transform::setPosition(glm::vec3& vec)
{
	transform = glm::translate(vec);
	position = vec;
}

void Transform::setRotation(float r, glm::vec3& axis)
{
	rotation = glm::rotate(r, axis);
}

void Transform::translate(glm::vec3& vec)
{
	transform = glm::translate(transform, vec);
	position += vec;
}

void Transform::rotate(float r, glm::vec3& axis)
{
	rotation = glm::rotate(rotation, r, axis);
}