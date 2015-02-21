#include "tilemap.h"
#include <fstream>
#include <cassert>
#include <iostream>
#include "gameobject.h"
#include "transform.h"

Tilemap::Tilemap(const std::string& path, Mesh* mesh, Texture* texture, ShaderProgram* program, glm::mat4* viewMatrix, glm::mat4* projectionMatrix)
{
	std::ifstream file(path);

	assert(file.is_open());

	file >> width >> std::ws;
	file >> height >> std::ws;

	data = new unsigned short* [height];

	for (unsigned int y = 0; y < height; y++)
	{
		data[y] = new unsigned short[width];

		for (unsigned int x = 0; x < width; x++)
		{
			file >> data[y][x] >> std::ws;
			std::cout << data[y][x] << " ";

			if (data[y][x] == 1)
			{
				GameObject *gameobject = new GameObject();
				gameobject->addComponent(new Transform(gameobject, float(x * 2), float(int(y) * -2)));
				gameobject->addComponent(new MeshRenderer(gameobject));

				gameobject->getComponent<MeshRenderer>()->setMesh(mesh);
				gameobject->getComponent<MeshRenderer>()->setProgram(program);
				gameobject->getComponent<MeshRenderer>()->setProjectionMatrix(projectionMatrix);
				gameobject->getComponent<MeshRenderer>()->setViewMatrix(viewMatrix);
				gameobject->getComponent<MeshRenderer>()->setTexture(texture);

				tiles.push_back(gameobject);
			}
		}
		std::cout << std::endl;
	}

	file.close();
}

Tilemap::~Tilemap()
{
	for (unsigned int i = 0; i < height; i++)
	{
		delete[] data[i];
	}

	delete[] data;

	for (auto tile : tiles)
		delete tile;

	tiles.clear();
}

void Tilemap::draw()
{
	for (auto tile : tiles)
		tile->update();
}