#ifndef TILEMAP_H
#define TILEMAP_H

#include <string>
#include "meshrenderer.h"

class Tilemap
{
public:
	Tilemap(const std::string& path, Mesh* mesh, Texture* texture, ShaderProgram* program, glm::mat4* viewMatrix, glm::mat4* projectionMatrix);
	~Tilemap();

	void draw();
private:
	unsigned short** data;
	unsigned int width;
	unsigned int height;

	std::vector<GameObject*> tiles;
};

#endif