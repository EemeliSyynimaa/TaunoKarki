#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <vector>
#include "texture.h"
#include "mesh.h"
#include "shaderprogram.h"

enum COLLISION_TYPES {
	COL_WALL = 0x0001,
	COL_PLAYER = 0x0002,
	COL_ENEMY = 0x0004,
	COL_ENEMY_BULLET = 0x0008,
	COL_PLAYER_BULLET = 0x0010,
	COL_ITEM = 0x0020
};

enum COLLECTIBLES
{
	PISTOL = 0,
	MACHINEGUN,
	SHOTGUN,
	HEALTHPACK,
	NONE
};

struct AssetManager
{
	AssetManager();
	~AssetManager();

	ShaderProgram* shaderProgram;

	Texture* tilesetTexture;
	Texture* sphereTexture;
	Texture* playerTexture;
	Texture* enemyTexture;
	Texture* itemsTexture;

	Mesh* wallMesh;
	Mesh* cubeMesh;
	Mesh* sphereMesh;
	Mesh* floorMesh;
	Mesh* pistolMesh;
	Mesh* machinegunMesh;
	Mesh* shotgunMesh;
	Mesh* healthpackMesh;
	Mesh* ammoBarMesh;
	Mesh* healthBarMesh;

	Mesh* addMesh(Mesh mesh);

	std::vector<Mesh*> meshDump;

	void loadAssets();
	void addMesh(Mesh* mesh) { meshDump.push_back(mesh); }
};

#endif