#include "assetmanager.h"

AssetManager::AssetManager()
{
	playerTexture = new Texture(GL_TEXTURE_2D, "assets/textures/cube.png");
	sphereTexture = new Texture(GL_TEXTURE_2D, "assets/textures/sphere.png");
	tilesetTexture = new Texture(GL_TEXTURE_2D, "assets/textures/tileset.png");
	enemyTexture = new Texture(GL_TEXTURE_2D, "assets/textures/enemy.png");

	shaderProgram = new ShaderProgram("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl");
	
	wallMesh = new Mesh("assets/meshes/wall.mesh");
	cubeMesh = new Mesh("assets/meshes/cube.mesh");
	sphereMesh = new Mesh("assets/meshes/sphere.mesh");
	floorMesh = new Mesh("assets/meshes/floor.mesh");
}

AssetManager::~AssetManager()
{
	delete playerTexture;
	delete sphereTexture;
	delete tilesetTexture;
	delete enemyTexture;

	delete shaderProgram;
	
	delete wallMesh;
	delete cubeMesh;
	delete sphereMesh;
	delete floorMesh;

	spriteMeshes.clear();
}

Mesh* AssetManager::addSprite(Mesh sprite)
{
	spriteMeshes.push_back(sprite); 
	return &spriteMeshes.back();
}