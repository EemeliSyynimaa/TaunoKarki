#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include "texture.h"
#include "mesh.h"
#include "shaderprogram.h"

enum COLLISION_TYPES {
	COL_WALL = 0x0001,
	COL_PLAYER = 0x0002,
	COL_ENEMY = 0x0004,
	COL_ENEMY_BULLET = 0x0008,
	COL_PLAYER_BULLET = 0x0010,
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

	Mesh* wallMesh;
	Mesh* cubeMesh;
	Mesh* sphereMesh;
	Mesh* floorMesh;
};

#endif