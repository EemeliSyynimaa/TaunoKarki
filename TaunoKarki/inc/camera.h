#ifndef CAMERA_H
#define CAMERA_H

#include "glm\glm.hpp"

class Camera
{
public:
	Camera();
	~Camera();

	void createNewPerspectiveMatrix(float fov, float width, float height, float near, float far);
	void createNewOrthographicMatrix(float width, float height);
	void setViewMatrix(glm::mat4 matrix) { viewMatrix = matrix; }
	void setPosition(glm::vec3 pos) { position = pos; }
	void setOffset(glm::vec3 vec) { offset = vec; }
	void setOffset(float x, float y, float z) { offset.x = x; offset.y = y; offset.z = z; }

	glm::mat4& getViewMatrix() { return viewMatrix; }
	glm::mat4& getPerspectiveMatrix() { return perspectiveMatrix; }
	glm::mat4& getOrthographicMatrix() { return orthographicMatrix; }

	void follow(glm::vec2 position);

	float getWidth() { return width; }
	float getHeight() { return height; }
	glm::vec3 getPosition() { return position; }
private:
	glm::mat4 viewMatrix;
	glm::mat4 orthographicMatrix;
	glm::mat4 perspectiveMatrix;

	glm::vec3 position;
	glm::vec3 offset;

	float width;
	float height;
};

#endif
