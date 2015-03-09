#include <random>
#include <functional>
#include "tilemap.h"
#include "gameobject.h"

#include <iostream>


// TODO http://journal.stuffwithstuff.com/2014/12/21/rooms-and-mazes/

Tilemap::Data::Data(unsigned int width, unsigned int height) : width(width), height(height)
{
	data = new unsigned short*[height];

	for (unsigned int y = 0; y < height; y++)
	{
		data[y] = new unsigned short[width];

		for (unsigned int x = 0; x < width; x++)
		{
			// All tiles are walls before we start carving.
			data[y][x] = 1;
		}
	}
}

Tilemap::Data::~Data()
{
	for (unsigned int i = 0; i < height; i++)
	{
		delete[] data[i];
	}

	delete[] data;
}

int Tilemap::Room::distanceTo(Room& otherRoom)
{
	return 0;
}

Tilemap::Tilemap(AssetManager& assetManager, Camera& camera, b2World& world) : gameObjectManager(assetManager, world, camera)
{
}

Tilemap::~Tilemap()
{
	tileRenderers.clear();
}

void Tilemap::generate(unsigned int width, unsigned int height)
{
	Data data(width, height);

	addRooms(data);	
	
	createWallObjects(data);

	// Update the manager. This needs to be done only once, because all the objects are static.
	gameObjectManager.update(0.0f);
}

void Tilemap::addRooms(Data& data)
{
	std::default_random_engine randomGenerator;
	std::uniform_int_distribution<int> distrY(1, data.height-6);
	std::uniform_int_distribution<int> distrX(1, data.width-6);

	unsigned int numberOfTries = 3;

	for (unsigned int i = 0; i < numberOfTries; i++)
	{
		Room room;
		room.x = distrX(randomGenerator);
		room.y = distrY(randomGenerator);
		room.width = 5;
		room.height = 5;

		std::cout << room.x << ", " << room.y << std::endl;

		data.rooms.push_back(room);

		for (unsigned int y = room.y; y < (room.y + room.height); y++)
		{
			for (unsigned int x = room.x; x < (room.x + room.width); x++)
			{
				data.data[y][x] = 0;
			}
		}
	}
}

void Tilemap::connectRegions()
{
}

void Tilemap::removeDeadEnds()
{
}

void Tilemap::createWallObjects(Data& data)
{
	for (unsigned int y = 0; y < data.height; y++)
	{
		for (unsigned int x = 0; x < data.width; x++)
		{
			if (data.data[y][x] == 1)
			{
				GameObject* gameObject = gameObjectManager.createWall(glm::vec3(float(x * 2), float(int(y) * -2), 0.0f));
				tileRenderers.push_back(gameObject->getComponent<MeshRenderer>());
			}
		}
	}
}

void Tilemap::draw()
{
	for (auto tile : tileRenderers)
		tile->update(0.0f);
}