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
private:
	struct Room
	{
		unsigned int x;
		unsigned int y;
		unsigned int width;
		unsigned int height;

		int distanceTo(Room& otherRoom);
	};

	struct Data
	{
		unsigned short** data;
		unsigned int width;
		unsigned int height;

		std::vector<Room> rooms;

		Data(unsigned int width, unsigned int height);
		~Data();
	};

	void addRooms(Data& data);
	void connectRegions();
	void removeDeadEnds();
	void createWallObjects(Data& data);

	std::vector<MeshRenderer*> tileRenderers;
	GameObjectManager gameObjectManager;
};

#endif