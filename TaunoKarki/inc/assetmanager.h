#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include "texture.h"
#include "mesh.h"
#include "shaderprogram.h"

struct AssetManager
{
	AssetManager();
	~AssetManager();

	ShaderProgram* shaderProgram;
	Texture* wallTexture;
	Texture* floorTexture;
	Texture* sphereTexture;
	Texture* playerTexture;
	Texture* enemyTexture;

	Mesh* wallMesh;
	Mesh* sphereMesh;
	Mesh* floorMesh;
};

#endif