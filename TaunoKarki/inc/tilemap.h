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
	glm::vec3 getPlayerStartingPosition() { return playerStartingPosition; }
private:
	// Data struct has the temporary data needed for generating the level.
	struct Data
	{
		unsigned short** data;
		unsigned int width;
		unsigned int height;
		unsigned short currentRegion;

		glm::vec3 playerStartingPosition;

		void startRegion() { currentRegion++; };
		void addRooms();
		bool roomConflictsWithOthers(unsigned int roomX, unsigned int roomY, unsigned int roomW, unsigned int roomH);
		void carveRoom(unsigned int roomX, unsigned int roomY, unsigned int roomW, unsigned int roomH);
		void growMaze(glm::uvec2 pos);
		void connectRegions();
		void removeDeadEnds();
		void carve(unsigned short x, unsigned short y) { data[y][x] = currentRegion; }
		void carve(glm::uvec2 pos) { carve(pos.x, pos.y); }
		bool canCarve(glm::uvec2 pos, glm::uvec2 dir);

		Data(unsigned int width, unsigned int height);
		~Data();
	};

	void createWallObjects(Data& data);

	std::vector<MeshRenderer*> tileRenderers;
	GameObjectManager gameObjectManager;

	glm::vec3 playerStartingPosition;
};

#endif