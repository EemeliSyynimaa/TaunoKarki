#ifndef TILEMAP_H
#define TILEMAP_H

#include "meshrenderer.h"
#include "gameobjectmanager.h"



class Tilemap
{
public:
	Tilemap(AssetManager& assetManager, Camera& camera, b2World& world);
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

	void createWallObjects(Data& data);

	std::vector<MeshRenderer*> tileRenderers;
	GameObjectManager gameObjectManager;

	std::vector<glm::vec3> startingPositions;
};

#endif