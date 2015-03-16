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

	unsigned int numberOfTries = 10;

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

			if (std::find(unmadeCells.begin(), unmadeCells.end(), lastDir) != unmadeCells.end() && windingPercent(randomGenerator) > 25)
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
	// ELI
	// DONED: Etitään connectorit (eli muurinpalat, jotka on kahden regionin välissä)
	// Katotaan joku huone (vaikka listan eka)
	// Avataan yks sitä myötäilevistä connectoreista
	// Se toinen alue jota connector myötäilee, sulautetaan se samaan alueeseen aloitushuoneen kanssa
	// Discardataan muut connectorit jotka on tämän alueen välillä
	// Haetaan sit seuraava connectori ja yhdistetään se jne jne.

	// :D:D C++ on paras
	// TODO siisti tätä 
	struct VecComparison
	{
		bool operator()(const glm::uvec2& vec1, const glm::uvec2& vec2)
		{
			return true;
		}
	};

	std::map<int, int> merged;
	std::vector<unsigned int> openRegions;

	for (unsigned int i = 1; i <= currentRegion; i++)
	{
		merged[i] = i;
		openRegions.push_back(i);
	}

	// Eli ny yhdistellään kaikki regionit
	// Toistetaan looppia kunnes regioneita on yksi
	while (openRegions.size() > 1)
	{

		std::map<glm::uvec2, std::vector<unsigned short>, VecComparison> connectorRegions;
		std::vector<glm::uvec2> connectors;

		// Ei kuitenkaan reunoja, koska ulos ei saa pelaajaa päästää
		for (unsigned short y = 1; y < height - 1; y++)
		{
			for (unsigned short x = 1; x < width - 1; x++)
			{
				std::vector<unsigned short> regions;
				if (data[y][x] != WALL) continue;

				// Meillä on tiili, katsotaan onko sen ympärillä useampaa eri regionia
				// Jos regioni löytyy jo vektorista, ni ei lisätä sitä sitte ni!
				if (data[y][x + 1] != WALL && std::find(regions.begin(), regions.end(), data[y][x + 1]) == regions.end()) regions.push_back(data[y][x + 1]);
				if (data[y][x - 1] != WALL && std::find(regions.begin(), regions.end(), data[y][x - 1]) == regions.end()) regions.push_back(data[y][x - 1]);
				if (data[y + 1][x] != WALL && std::find(regions.begin(), regions.end(), data[y + 1][x]) == regions.end()) regions.push_back(data[y + 1][x]);
				if (data[y - 1][x] != WALL && std::find(regions.begin(), regions.end(), data[y - 1][x]) == regions.end()) regions.push_back(data[y - 1][x]);

				if (regions.size() < 2) continue;

				connectorRegions[glm::uvec2(x, y)] = regions;
				connectors.push_back(glm::uvec2(x, y));
			}
		}

		// Otetaan eka connectori
		glm::uvec2 connector = connectors.front();
		std::vector<unsigned short> regions = connectorRegions[connector];
		unsigned short regionID = regions.front();

		// Muutetaan kaikki regionit samaan
		for (auto region : regions) merged[region] = regionID;

		// Otetaan eka regioni listalta pois!
		regions.erase(regions.begin());

		// Muutetaan regionit 
		for (unsigned int i = 1; i <= currentRegion; i++)
		{
			for (unsigned int j = 0; j < regions.size(); j++)
			{
				if (merged[i] == regions[j]) merged[i] = regionID;
			}
		}

		for (unsigned int i = 0; i < regions.size(); i++)
		{
			for (unsigned int j = openRegions.size(); j > 0; j--)
			{
				if (openRegions[j-1] == regions[i]) openRegions.erase(openRegions.begin() + j-1);
			}
		}
	}

	/*
	BULLSHIT WALKS

	while (openRegions.size() > 1)
	{
		glm::uvec2 connector = connectors.front();

		carve(connector);

		std::vector<unsigned short> regions = connectorRegions[connector];
		int regionID = regions.front();

		// Muutetaan kaikki regionit yhdeksi tai jotain
		for (auto region : regions) merged[region] = regionID;

		// Poistetaan eka
		regions.erase(regions.begin());

		// Muutetaan regionit sit lopulta
		for (unsigned int i = 1; i <= currentRegion; i++)
		{
			if (std::find(regions.begin(), regions.end(), merged[i]) != regions.end())
			{
				merged[i] = regionID;
			}
		}

		for (auto region : regions)
		{
			for (unsigned int i = 0; i < openRegions.size(); i++)
			{
				if (openRegions[i] == region) openRegions.erase(openRegions.begin() + i);
			}
		}
	
		for (unsigned int i = connectors.size(); i >= 0; i--)
		{
		}
	}
	*/
}

void Tilemap::Data::removeDeadEnds()
{
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