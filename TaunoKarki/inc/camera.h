#ifndef CAMERA_H
#define CAMERA_H

#include "glm\glm.hpp"

class Camera
{
public:
	Camera();
	~Camera();

	void createNewPerspectiveMatrix(float fov, float width, float height, float near, float far);
	
	void setViewMatrix(glm::mat4 matrix) { viewMatrix = matrix; }
	void setPosition(glm::vec3 pos) { position = pos; }

	glm::mat4& getViewMatrix() { return viewMatrix; }
	glm::mat4& getProjectionMatrix() { return projectionMatrix; }

	void follow(glm::vec2 position);

	float getWidth() { return width; }
	float getHeight() { return height; }
private:
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	glm::vec3 position;

	float width;
	float height;
};

#endif
