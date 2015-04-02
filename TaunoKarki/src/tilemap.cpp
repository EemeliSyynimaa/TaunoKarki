#include <random>
#include <algorithm>
#include "tilemap.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/glm.hpp"

#define WALL 0
#define randomInt std::uniform_int_distribution<int>

Tilemap::Tilemap(glm::vec3 position, AssetManager& assetManager, Camera& camera, b2World& world) : assetManager(assetManager), world(world), camera(camera), VBO(0), IBO(0), textureIndex(0), MVPIndex(0), program(*assetManager.shaderProgram), texture(*assetManager.tilesetTexture), matrix(1.0f), position(position)
{
	matrix = glm::translate(position);
}

Tilemap::~Tilemap()
{
	for (b2Body* body : wallBodies)
		world.DestroyBody(body);

	wallBodies.clear();

	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IBO);
}


Tilemap::Data::Data(unsigned int width, unsigned int height) : width(width), height(height), currentRegion(0), windingPercent(50)
{
	data = new unsigned short*[height];

	for (size_t y = 0; y < height; y++)
	{
		data[y] = new unsigned short[width];

		for (size_t x = 0; x < width; x++)
		{
			// All tiles are walls  before we start carving.
			data[y][x] = WALL;
		}
	}
}

Tilemap::Data::~Data()
{
	for (size_t i = 0; i < height; i++)
	{
		delete[] data[i];
	}

	delete[] data;
}

bool Tilemap::Data::roomConflictsWithOthers(size_t roomX, size_t roomY, size_t roomW, size_t roomH)
{
	// +1 and -1 are for the borders of the room.
	for (size_t y = roomY - 1; y < roomY + roomH + 1; y++)
	{
		for (size_t x = roomX - 1; x < roomX + roomW + 1; x++)
		{
			// Achievement unlocked: Use goto in C++
			if (data[y][x] != WALL) goto conflicted;
		}
	}

	return false;

conflicted:
	return true;
}

void Tilemap::generate(size_t width, size_t height)
{
	// We want our maps to be odd sized.
	assert(width % 2 == 1 || height % 2 == 1);

	Data data(width, height);

	startingPositions = data.addRooms();

	for (size_t y = 1; y < height; y += 2)
	{
		for (size_t x = 1; x < width; x += 2)
		{
			if (data.data[y][x] != WALL) continue;
			data.growMaze(glm::uvec2(x, y));
		}
	}

	data.connectRegions();
	data.removeDeadEnds();
	data.openClosedAreas();
	createMeshes(data);
}

std::vector<glm::vec3> Tilemap::Data::addRooms()
{
	std::random_device randomDevice;
	std::default_random_engine randomGenerator(randomDevice());
	std::vector<glm::vec3> startingPositions;

	size_t howManyTries = (width * height);

	for (size_t i = 0; i < howManyTries; i++)
	{
		// We want our room size to be odd.
		size_t roomW = randomInt(3, 9)(randomGenerator);
		size_t roomH = randomInt(3, 9)(randomGenerator);

		roomW += 1 - roomW % 2;
		roomH += 1 - roomH % 2;

		size_t roomX = randomInt(1, width - roomW - 1)(randomGenerator);
		size_t roomY = randomInt(1, height - roomH - 1)(randomGenerator);

		// We want our room position to be odd too.
		roomX += 1 - roomX % 2;
		roomY += 1 - roomY % 2;

		// We check if the room conflicts with an existing room.
		if (roomConflictsWithOthers(roomX, roomY, roomW, roomH)) continue;

		carveRoom(roomX, roomY, roomW, roomH);

		// Every room has a random starting point
		startingPositions.push_back(glm::vec3(float(randomInt(roomX + 1, roomX + roomW - 1)(randomGenerator)) * 2.0f, float(randomInt(roomY + 1, roomY + roomH - 1)(randomGenerator)) * 2.0f, 0.0f));
	}

	return startingPositions;
}

void Tilemap::Data::carveRoom(size_t roomX, size_t roomY, size_t roomW, size_t roomH)
{
	startRegion();

	for (size_t y = roomY; y < (roomY + roomH); y++)
	{
		for (size_t x = roomX; x < (roomX + roomW); x++)
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

			if (std::find(unmadeCells.begin(), unmadeCells.end(), lastDir) != unmadeCells.end() && randomInt(0, 100)(randomGenerator) > windingPercent)
				dir = lastDir;
			else
				dir = unmadeCells.at(randomInt(0, unmadeCells.size() - 1)(randomGenerator));

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

	std::vector<size_t> openRegions;

	for (size_t i = 1; i <= currentRegion; i++)
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

		for (size_t y = 1; y < height - 1; y++)
		{
			for (size_t x = 1; x < width - 1; x++)
			{
				for (auto region : connector.regions)
				{
					if (data[y][x] == region) data[y][x] = regionID;
				}
			}
		}

		data[connector.y][connector.x] = regionID;

		for (size_t i = 0; i < connector.regions.size(); i++)
		{
			for (size_t j = openRegions.size(); j > 0; j--)
			{
				if (openRegions[j - 1] == connector.regions[i]) openRegions.erase(openRegions.begin() + j - 1);
			}
		}

		// Lets open a random connector just for funz!
		std::random_device randomDevice;
		std::default_random_engine randomGenerator(randomDevice());

		if (randomInt(0, 100)(randomGenerator) > 75)
		{
			Connector& temp = connectors[randomInt(0, connectors.size() - 1)(randomGenerator)];

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

		for (size_t y = 1; y < height - 1; y++)
		{
			for (size_t x = 1; x < width - 1; x++)
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

void Tilemap::Data::openClosedAreas()
{
	startRegion();
	for (size_t y = 1; y < height - 1; y++)
	{
		for (size_t x = 1; x < width - 1; x++)
		{
			if ((checkTile(x, y + 1, WALL) || checkTile(x, y + 1, currentRegion)) &&
				(checkTile(x, y - 1, WALL) || checkTile(x, y - 1, currentRegion)) &&
				(checkTile(x + 1, y, WALL) || checkTile(x + 1, y, currentRegion)) &&
				(checkTile(x - 1, y, WALL) || checkTile(x - 1, y, currentRegion)) &&
				(checkTile(x + 1, y + 1, WALL) || checkTile(x + 1, y + 1, currentRegion)) &&
				(checkTile(x + 1, y - 1, WALL) || checkTile(x + 1, y - 1, currentRegion)) &&
				(checkTile(x - 1, y + 1, WALL) || checkTile(x - 1, y + 1, currentRegion)) &&
				(checkTile(x - 1, y - 1, WALL) || checkTile(x - 1, y - 1, currentRegion))) carve(x, y);
		}
	}

	// Borders!
	for (size_t x = 0; x < width; x++)
	{
		if ((checkTile(x, 1, currentRegion))) carve(x, 0);
		if ((checkTile(x, height - 2, currentRegion))) carve(x, height - 1);
	}

	for (size_t y = 0; y < height; y++)
	{
		if ((checkTile(1, y, currentRegion))) carve(0, y);
		if ((checkTile(width - 2, y, currentRegion))) carve(width - 1, y);
	}
}

void Tilemap::createMeshes(Data& data)
{
	GLuint counter = 0;
	unsigned short currentRegion = 0;
	std::random_device randomDevice;
	std::default_random_engine randomGenerator(randomDevice());
	float floorTileOffset = 0.0f + 0.125f * randomInt(0, 7) (randomGenerator);

	for (size_t y = 0; y < data.height; y++)
	{
		for (size_t x = 0; x < data.width; x++)
		{
			if (data.data[y][x] == WALL)
			{
				for (const Vertex& vertex : assetManager.wallMesh->getVertices())
				{
					Vertex newVertex;

					newVertex.position.x = x * 2.0f + vertex.position.x;
					newVertex.position.y = y * 2.0f + vertex.position.y;
					newVertex.position.z = vertex.position.z;

					newVertex.uv = vertex.uv;
					newVertex.normal = vertex.normal;

					vertices.push_back(newVertex);
					
				}

				for (GLuint index : assetManager.wallMesh->getIndices())
					indices.push_back(index + counter);

				counter += assetManager.wallMesh->getVertices().size();

				b2BodyDef bodyDef;
				bodyDef.position = b2Vec2(x * 2 + position.x, y * 2 + position.y);
				bodyDef.type = b2_staticBody;
				
				b2Body* body = world.CreateBody(&bodyDef);
				
				b2PolygonShape shape;
				shape.SetAsBox(1.0f, 1.0f);

				b2FixtureDef fixtureDef;
				fixtureDef.density = 1.0f;
				fixtureDef.friction = 0.0f;
				fixtureDef.shape = &shape;
				fixtureDef.filter.categoryBits = COL_WALL;
				fixtureDef.filter.maskBits = (COL_PLAYER | COL_ENEMY | COL_PLAYER_BULLET | COL_ENEMY_BULLET);

				body->CreateFixture(&fixtureDef);

				wallBodies.push_back(body);
			}
			else if (data.data[y][x] != data.currentRegion)
			{
				for (const Vertex& vertex : assetManager.floorMesh->getVertices())
				{
					Vertex newVertex;

					newVertex.position.x = x * 2.0f + vertex.position.x;
					newVertex.position.y = y * 2.0f + vertex.position.y;
					newVertex.position.z = vertex.position.z - 2.0f;

					newVertex.uv.x = vertex.uv.x + floorTileOffset;
					newVertex.uv.y = vertex.uv.y;
					newVertex.normal = vertex.normal;

					vertices.push_back(newVertex);

				}

				for (GLuint index : assetManager.floorMesh->getIndices())
					indices.push_back(index + counter);

				counter += assetManager.floorMesh->getVertices().size();
			}
		}
	}

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	program.bind();

	MVPIndex = program.getUniformLocation("MVP");
	textureIndex = program.getUniformLocation("texture");

	program.unbind();
}

void Tilemap::draw()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, uv)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));

	program.bind();

	glUniform1i(textureIndex, 0);
	glUniformMatrix4fv(MVPIndex, 1, GL_FALSE, glm::value_ptr(camera.getProjectionMatrix() * camera.getViewMatrix() * matrix));

	texture.bind(GL_TEXTURE0);

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

	program.unbind();

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

glm::vec3 Tilemap::getStartingPosition()
{
	glm::vec3 position = startingPositions.back();
	startingPositions.pop_back();

	return position;
}