#include "camera.h"
#include "glm/gtc/matrix_transform.hpp"

Camera::Camera() : viewMatrix(1.0f), viewMatrixWithoutOffset(1.0f), perspectiveMatrix(1.0f), orthographicMatrix(1.0f), position(0.0f), width(0.0f), height(0.0f), offset(0.0f)
{
}

Camera::~Camera()
{
}

void Camera::createNewPerspectiveMatrix(float fov, float width, float height, float near, float far)
{
	perspectiveMatrix = glm::perspective(glm::radians(fov), width/height, near, far);
	this->width = width;
	this->height = height;
}

void Camera::createNewOrthographicMatrix(float width, float height)
{
	orthographicMatrix = glm::ortho(0.0f, width, height, 0.0f, 0.0f, 100.0f);
}

void Camera::follow(glm::vec2 pos, glm::vec2 dirvec)
{
	position.x = pos.x;
	position.y = pos.y;

	viewMatrix = glm::lookAt(
		glm::vec3(position.x - dirvec.x, position.y - dirvec.y, 1.5f),
		glm::vec3(position.x, position.y, 1.5f),
		glm::vec3(0, 0, 1));

	viewMatrixWithoutOffset = glm::lookAt(
		glm::vec3(position.x, position.y, position.z),
		glm::vec3(position.x, position.y, 0),
		glm::vec3(0, 1, 0));
}