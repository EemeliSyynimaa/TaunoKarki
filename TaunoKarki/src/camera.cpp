#include "camera.h"
#include "glm/gtc/matrix_transform.hpp"

Camera::Camera() : viewMatrix(1.0f), projectionMatrix(1.0f), position(0.0f), width(0.0f), height(0.0f)
{
}

Camera::~Camera()
{
}

void Camera::createNewPerspectiveMatrix(float fov, float width, float height, float near, float far)
{
	projectionMatrix = glm::perspective(glm::radians(fov), width/height, near, far);
	this->width = width;
	this->height = height;
}

void Camera::follow(glm::vec2 pos)
{
	position.x = pos.x;
	position.y = pos.y;

	viewMatrix = glm::lookAt(
		glm::vec3(position.x, position.y - 5.0f, position.z),
		glm::vec3(position.x, position.y, 0),
		glm::vec3(0, 1, 0));
}