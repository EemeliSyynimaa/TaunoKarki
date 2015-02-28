#ifndef TILEMAP_H
#define TILEMAP_H

#include <string>
#include "meshrenderer.h"
#include "Box2D\Box2D.h"

class Tilemap
{
public:
	Tilemap(const std::string& path, Mesh* mesh, Texture* texture, ShaderProgram* program, Camera& camera, b2World& world);
	~Tilemap();

	void draw();
private:
	unsigned short** data;
	unsigned int width;
	unsigned int height;

	std::vector<GameObject*> tiles;
};

#endif