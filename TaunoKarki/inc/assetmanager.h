#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include "texture.h"
#include "mesh.h"
#include "shaderprogram.h"

struct AssetManager
{
public:
	AssetManager();
	~AssetManager();

	ShaderProgram* shaderProgram;
	Texture* wallTexture;
	Texture* sphereTexture;
	Texture* playerTexture;

	Mesh* wallMesh;
	Mesh* sphereMesh;
};

#endif