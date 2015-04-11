#include "transform.h"
#include "glm/gtx/transform.hpp"

Transform::Transform(GameObject* owner, float x, float y, float z) : Component(owner), position(x, y, z), scale(1.0f), rotation(1.0f), transform(1.0f), dirVec(0.0f)
{
	transform = glm::translate(position);
}

Transform::Transform(GameObject* owner, glm::vec3 position) : Component(owner), position(position), scale(1.0f), rotation(1.0f), transform(1.0f)
{
	transform = glm::translate(position);
}

Transform::~Transform()
{
}

void Transform::update()
{
}

void Transform::lookAt(const glm::vec3& target)
{
	glm::vec3 deltaPos = target - position;
	glm::vec3 axis(0.0f, 0.0f, 1.0f);
	float angle = glm::atan(deltaPos.y, deltaPos.x);
	float length = sqrt(powf(deltaPos.x, 2) + powf(deltaPos.y, 2));

	dirVec = glm::vec2(deltaPos.x / length, deltaPos.y / length);

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

void Transform::setScale(glm::vec3& vec)
{
	scale = glm::scale(vec);
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