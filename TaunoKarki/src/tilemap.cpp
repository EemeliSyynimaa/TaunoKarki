#include <random>
#include <algorithm>
#include <map>
#include "tilemap.h"
#include "gameobject.h"

#include <iostream>
#include "glm\glm.hpp"

#define WALL 0


// TODO http://journal.stuffwithstuff.com/2014/12/21/rooms-and-mazes/

Tilemap::Data::Data(unsigned int width, unsigned int height) : width(width), height(height), currentRegion(0)
{
	data = new unsigned short*[height];

	for (unsigned int y = 0; y < height; y++)
	{
		data[y] = new unsigned short[width];

		for (unsigned int x = 0; x < width; x++)
		{
			// All tiles are walls  before we start carving.
			data[y][x] = WALL;
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

bool Tilemap::Data::roomConflictsWithOthers(unsigned int roomX, unsigned int roomY, unsigned int roomW, unsigned int roomH)
{
	// +1 and -1 are for the borders of the room.
	for (unsigned int y = roomY - 1; y < roomY + roomH + 1; y++)
	{
		for (unsigned int x = roomX - 1; x < roomX + roomW + 1; x++)
		{
			// Achievement unlocked: Use goto in C++
			if (data[y][x] != WALL) goto conflicted;
		}
	}

	return false;

conflicted:
	return true;
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
	// We want our maps to be odd sized.
	assert(width % 2 == 1 || height % 2 == 1);

	Data data(width, height);

	data.addRooms();

	for (unsigned int y = 1; y < height; y += 2)
	{
		for (unsigned int x = 1; x < width; x += 2)
		{
			if (data.data[y][x] != WALL) continue;
			data.growMaze(glm::uvec2(x, y));
		}
	}

	playerStartingPosition = data.playerStartingPosition;
	
	data.connectRegions();
	data.removeDeadEnds();
	createWallObjects(data);

	// Update the manager. This needs to be done only once, because all the objects are static.
	gameObjectManager.update(0.0f);
}

void Tilemap::Data::addRooms()
{
	std::random_device randomDevice;
	std::default_random_engine randomGenerator(randomDevice());

	unsigned int numberOfTries = 100;

	for (unsigned int i = 0; i < numberOfTries; i++)
	{
		std::uniform_int_distribution<int> distrPosW(3, 9);
		std::uniform_int_distribution<int> distrPosH(3, 9);

		// We want our room size to be odd.
		unsigned int roomW = distrPosW(randomGenerator);
		unsigned int roomH = distrPosH(randomGenerator);

		roomW += 1 - roomW % 2;
		roomH += 1 - roomH % 2;

		std::uniform_int_distribution<int> distrPosX(1, width - roomW - 1);
		std::uniform_int_distribution<int> distrPosY(1, height - roomH - 1);

unsigned int roomX = distrPosX(randomGenerator);
unsigned int roomY = distrPosY(randomGenerator);

// We want our room position to be odd too.
roomX += 1 - roomX % 2;
roomY += 1 - roomY % 2;

// We check if the room conflicts with an existing room.
if (roomConflictsWithOthers(roomX, roomY, roomW, roomH)) continue;

carveRoom(roomX, roomY, roomW, roomH);

// Player starts from the first room generated.
if (currentRegion == 1) playerStartingPosition = glm::vec3(float((roomX + roomW / 2.f)* 2.f), float((roomY + roomH / 2.f) * -2.f), 0.0f);
	}
}

void Tilemap::Data::carveRoom(unsigned int roomX, unsigned int roomY, unsigned int roomW, unsigned int roomH)
{
	startRegion();

	for (unsigned int y = roomY; y < (roomY + roomH); y++)
	{
		for (unsigned int x = roomX; x < (roomX + roomW); x++)
		{
			carve(x, y);
		}
	}
}

void Tilemap::Data::growMaze(glm::uvec2 pos)
{
	std::vector<glm::uvec2> cells;
	glm::uvec2 lastDir;

	std::random_device randomDevice;
	std::default_random_engine randomGenerator(randomDevice());

	startRegion();

	carve(pos);
	cells.push_back(pos);

	while (!cells.empty())
	{
		glm::uvec2& cell = cells.back();
		std::vector<glm::uvec2> unmadeCells;

		if (canCarve(cell, glm::uvec2(1, 0))) unmadeCells.push_back(glm::uvec2(1, 0));
		if (canCarve(cell, glm::uvec2(-1, 0))) unmadeCells.push_back(glm::uvec2(-1, 0));
		if (canCarve(cell, glm::uvec2(0, 1))) unmadeCells.push_back(glm::uvec2(0, 1));
		if (canCarve(cell, glm::uvec2(0, -1))) unmadeCells.push_back(glm::uvec2(0, -1));

		if (!unmadeCells.empty())
		{
			glm::uvec2 dir;
			std::uniform_int_distribution<int> windingPercent(0, 100);
			std::uniform_int_distribution<int> randomDirection(0, unmadeCells.size() - 1);

			if (std::find(unmadeCells.begin(), unmadeCells.end(), lastDir) != unmadeCells.end() && windingPercent(randomGenerator) > 50)
				dir = lastDir;
			else
				dir = unmadeCells.at(randomDirection(randomGenerator));

			carve(cell + dir);
			carve(cell + glm::uvec2(dir.x * 2, dir.y * 2));

			cells.push_back(cell + glm::uvec2(dir.x * 2, dir.y * 2));

			lastDir = dir;
		}
		else
		{
			cells.pop_back();
			lastDir = glm::uvec2(0, 0);
		}
	}
}

bool Tilemap::Data::canCarve(glm::uvec2 pos, glm::uvec2 dir)
{
	if (pos.x + dir.x * 3 >= width || pos.y + dir.y * 3 >= height) return false;

	return data[pos.y + dir.y * 2][pos.x + dir.x * 2] == WALL;
}

void Tilemap::Data::connectRegions()
{
	struct Connector
	{
		unsigned short x;
		unsigned short y;

		std::vector<unsigned short> regions;
	};

	std::vector<unsigned int> openRegions;

	for (unsigned int i = 1; i <= currentRegion; i++)
	{
		openRegions.push_back(i);
	}

	while (openRegions.size() > 1)
	{
		std::vector<Connector> connectors;

		for (unsigned short y = 1; y < height - 1; y++)
		{
			for (unsigned short x = 1; x < width - 1; x++)
			{
				Connector connector;
				if (data[y][x] != WALL) continue;

				if (data[y][x + 1] != WALL && std::find(connector.regions.begin(), connector.regions.end(), data[y][x + 1]) == connector.regions.end()) connector.regions.push_back(data[y][x + 1]);
				if (data[y][x - 1] != WALL && std::find(connector.regions.begin(), connector.regions.end(), data[y][x - 1]) == connector.regions.end()) connector.regions.push_back(data[y][x - 1]);
				if (data[y + 1][x] != WALL && std::find(connector.regions.begin(), connector.regions.end(), data[y + 1][x]) == connector.regions.end()) connector.regions.push_back(data[y + 1][x]);
				if (data[y - 1][x] != WALL && std::find(connector.regions.begin(), connector.regions.end(), data[y - 1][x]) == connector.regions.end()) connector.regions.push_back(data[y - 1][x]);

				if (connector.regions.size() < 2) continue;

				connector.x = x;
				connector.y = y;

				connectors.push_back(connector);
			}
		}

		Connector& connector = connectors.front();
		unsigned short regionID = connector.regions.front();
		connector.regions.erase(connector.regions.begin());

		for (unsigned int y = 1; y < height - 1; y++)
		{
			for (unsigned int x = 1; x < width - 1; x++)
			{
				for (auto region : connector.regions)
				{
					if (data[y][x] == region) data[y][x] = regionID;
				}
			}
		}

		data[connector.y][connector.x] = regionID;

		for (unsigned int i = 0; i < connector.regions.size(); i++)
		{
			for (unsigned int j = openRegions.size(); j > 0; j--)
			{
				if (openRegions[j - 1] == connector.regions[i]) openRegions.erase(openRegions.begin() + j - 1);
			}
		}

		// Lets open a random connector just for funz!
		std::random_device randomDevice;
		std::default_random_engine randomGenerator(randomDevice());

		std::uniform_int_distribution<int> distrOpenConnectors(0, 100);
		std::uniform_int_distribution<int> distrRandomConnector(0, connectors.size());

		if (distrOpenConnectors(randomGenerator) > 25)
		{
			Connector& temp = connectors[distrRandomConnector(randomGenerator)];

			data[temp.y][temp.x] = regionID;
		}
	}
}

void Tilemap::Data::removeDeadEnds()
{
	bool done = false;

	while (!done)
	{
		done = true;

		for (unsigned int y = 1; y < height - 1; y++)
		{
			for (unsigned int x = 1; x < width - 1; x++)
			{
				if (data[y][x] == WALL) continue;

				short exists = 0;

				if (data[y][x + 1] != WALL) exists++;
				if (data[y][x - 1] != WALL) exists++;
				if (data[y + 1][x] != WALL) exists++;
				if (data[y - 1][x] != WALL) exists++;

				if (exists != 1) continue;

				done = false;
				data[y][x] = WALL;
			}
		}
	}
}

void Tilemap::createWallObjects(Data& data)
{
	for (unsigned int y = 0; y < data.height; y++)
	{
		for (unsigned int x = 0; x < data.width; x++)
		{
			if (data.data[y][x] == WALL)
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