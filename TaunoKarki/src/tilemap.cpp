#include "tilemap.h"
#include <fstream>
#include <cassert>
#include <iostream>
#include "gameobject.h"
#include "transform.h"
#include "staticbody.h"
#include "boxcollider.h"
#include "camera.h"

Tilemap::Tilemap(const std::string& path, AssetManager& assetManager, Camera& camera, b2World& world) : gameObjectManager(assetManager, world, camera)
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
				GameObject* gameobject = gameObjectManager.createWall(glm::vec3(float(x * 2), float(int(y) * -2), 0.0f));
				tileRenderers.push_back(gameobject->getComponent<MeshRenderer>());
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
}

void Tilemap::draw()
{
	for (auto tile : tileRenderers)
		tile->update(0.0f);
}