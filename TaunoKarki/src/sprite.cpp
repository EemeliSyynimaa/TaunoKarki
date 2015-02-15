#include "sprite.h"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>
#include "glm/gtc/matrix_transform.hpp">

Sprite::Sprite()
{
	reset();
}

Sprite::~Sprite()
{

}

void Sprite::setProgram(ShaderProgram* prgrm)
{
	program = prgrm;
	program->bind();

	MVPIndex = glGetUniformLocation(program->getID(), "MVP");
	textureIndex = glGetUniformLocation(program->getID(), "texture");

	program->unbind();
}

void Sprite::setTexture(Texture* txtr)
{
	texture = txtr;
}

void Sprite::setViewMatrix(glm::mat4* matrix)
{
	viewMatrix = matrix;
}

void Sprite::setProjectionMatrix(glm::mat4* matrix)
{
	projectionMatrix = matrix;
}

void Sprite::reset()
{
	texture = nullptr;
	program = nullptr;

	modelMatrix = new glm::mat4(1.0f);
	projectionMatrix = nullptr;
	viewMatrix = nullptr;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	GLfloat vertexData[] = {
		-1.0, -1.0, 1.0,
		1.0, -1.0, 1.0,
		1.0, 1.0, 1.0,
		-1.0, 1.0, 1.0,
		// top
		-1.0, 1.0, 1.0,
		1.0, 1.0, 1.0,
		1.0, 1.0, -1.0,
		-1.0, 1.0, -1.0,
		// back
		1.0, -1.0, -1.0,
		-1.0, -1.0, -1.0,
		-1.0, 1.0, -1.0,
		1.0, 1.0, -1.0,
		// bottom
		-1.0, -1.0, -1.0,
		1.0, -1.0, -1.0,
		1.0, -1.0, 1.0,
		-1.0, -1.0, 1.0,
		// left
		-1.0, -1.0, -1.0,
		-1.0, -1.0, 1.0,
		-1.0, 1.0, 1.0,
		-1.0, 1.0, -1.0,
		// right
		1.0, -1.0, 1.0,
		1.0, -1.0, -1.0,
		1.0, 1.0, -1.0,
		1.0, 1.0, 1.0,
	};

	vertexSize = sizeof(vertexData);

	GLfloat texcoords[2*4*6] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
	};

	for (int i = 1; i < 6; i++)
		memcpy(&texcoords[i * 4 * 2], &texcoords[0], 2 * 4 * sizeof(GLfloat));

	GLfloat indices[] = {
		0, 1, 2,
		2, 3, 0,
		// top
		4, 5, 6,
		6, 7, 4,
		// back
		8, 9, 10,
		10, 11, 8,
		// bottom
		12, 13, 14,
		14, 15, 12,
		// left
		16, 17, 18,
		18, 19, 16,
		// right
		20, 21, 22,
		22, 23, 20,
	};

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexSize + sizeof(texcoords), NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, vertexSize, vertexData);
	glBufferSubData(GL_ARRAY_BUFFER, vertexSize, sizeof(texcoords), texcoords);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Sprite::update()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)vertexSize);

	program->bind();

	glUniform1i(textureIndex, 0);
	glUniformMatrix4fv(MVPIndex, 1, GL_FALSE, glm::value_ptr(*projectionMatrix * *viewMatrix * *modelMatrix));

	texture->bind(GL_TEXTURE0);

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}