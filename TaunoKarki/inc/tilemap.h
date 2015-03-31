#ifndef TILEMAP_H
#define TILEMAP_H

#include <vector>
#include "assetmanager.h"
#include "camera.h"
#include "Box2D\Box2D.h"

class Tilemap
{
public:
	Tilemap(glm::vec3 position, AssetManager& assetManager, Camera& camera, b2World& world);
	~Tilemap();

	void draw();
	void generate(unsigned int width, unsigned int height);
	glm::vec3 getStartingPosition();
	size_t getNumberOfStartingPositions() { return startingPositions.size(); }
private:
	// Data struct has the temporary data needed for generating the level.
	struct Data
	{
		unsigned short** data;
		unsigned int width;
		unsigned int height;
		unsigned short currentRegion;
		int windingPercent;

		void startRegion() { currentRegion++; };
		std::vector<glm::vec3> addRooms();
		bool roomConflictsWithOthers(unsigned int roomX, unsigned int roomY, unsigned int roomW, unsigned int roomH);
		void carveRoom(unsigned int roomX, unsigned int roomY, unsigned int roomW, unsigned int roomH);
		void growMaze(glm::uvec2 pos);
		void connectRegions();
		void removeDeadEnds();
		void openClosedAreas();
		void carve(unsigned short x, unsigned short y) { data[y][x] = currentRegion; }
		void carve(glm::uvec2 pos) { carve(pos.x, pos.y); }
		bool canCarve(glm::uvec2 pos, glm::uvec2 dir);
		bool checkTile(size_t x, size_t y, size_t tile) { return data[y][x] == tile; };

		Data(unsigned int width, unsigned int height);
		~Data();
	};

	void createMeshes(Data& data);

	std::vector<glm::vec3> startingPositions;

	Camera& camera;
	AssetManager& assetManager;
	b2World& world;
	glm::vec3 position;

	std::vector<b2Body*> wallBodies;

	GLuint VBO;
	GLuint IBO;
	GLint textureIndex;
	GLint MVPIndex;

	std::vector<GLuint> indices;
	std::vector<Vertex> vertices;

	ShaderProgram& program;
	Texture& texture;

	glm::mat4 matrix;
};

#endif