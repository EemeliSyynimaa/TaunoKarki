#include "transform.h"
#include "glm/gtx/transform.hpp"

Transform::Transform(GameObject* owner) : Component(owner), position(0.0f), scale(1.0f), rotation(1.0f), transform(1.0f)
{
}

Transform::~Transform()
{
}

void Transform::update()
{
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