#ifndef SPRITE_H
#define SPRITE_H

#include <iostream>
#include "component.h"
#include "texture.h"
#include "shaderprogram.h"
#include "transform.h"
#include "glm/glm.hpp"

class Sprite : public Component
{
public:
	Sprite();
	~Sprite();
	void reset();
	void update();
	void setTexture(Texture* texture);
	void setProgram(ShaderProgram* program);
	void setViewMatrix(glm::mat4* matrix);
	void setProjectionMatrix(glm::mat4* matrix);
private:
	Texture* texture;
	ShaderProgram* program;
	Transform* transform;

	glm::mat4* projectionMatrix;
	glm::mat4* viewMatrix;

	GLuint vertexSize;
	GLuint VBO;
	GLuint IBO;
	GLint textureIndex;
	GLint MVPIndex;
};

#endif