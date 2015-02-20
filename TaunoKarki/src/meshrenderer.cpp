#include "meshrenderer.h"
#include "glm/gtc/type_ptr.hpp"
#include "gameobject.h"

MeshRenderer::MeshRenderer(GameObject* owner) : Component(owner)
{
	mesh = nullptr;
	texture = nullptr;
	program = nullptr;
	projectionMatrix = nullptr;
	viewMatrix = nullptr;

	transform = owner->getComponent<Transform>();
	assert(transform);

	textureIndex = 0;
	MVPIndex = 0;
	IBO = 0;
	VBO = 0;
}

MeshRenderer::~MeshRenderer()
{
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IBO);
}

void MeshRenderer::update()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, uv)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));

	program->bind();

	glUniform1i(textureIndex, 0);
	glUniformMatrix4fv(MVPIndex, 1, GL_FALSE, glm::value_ptr(*projectionMatrix * *viewMatrix * transform->getMatrix()));

	texture->bind(GL_TEXTURE0);

	glDrawElements(GL_TRIANGLES, mesh->getIndices().size(), GL_UNSIGNED_SHORT, nullptr);

	program->unbind();

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

void MeshRenderer::setMesh(Mesh* mesh)
{
	this->mesh = mesh;

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, mesh->getVertices().size() * sizeof(Vertex), mesh->getVertices().data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->getIndices().size() * sizeof(GLushort), mesh->getIndices().data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void MeshRenderer::setTexture(Texture* texture)
{
	this->texture = texture;
}

void MeshRenderer::setProgram(ShaderProgram* prgrm)
{
	program = prgrm;
	program->bind();

	MVPIndex = glGetUniformLocation(program->getID(), "MVP");
	textureIndex = glGetUniformLocation(program->getID(), "texture");

	program->unbind();
}

void MeshRenderer::setViewMatrix(glm::mat4* matrix)
{
	viewMatrix = matrix;
}

void MeshRenderer::setProjectionMatrix(glm::mat4* matrix)
{
	projectionMatrix = matrix;
}