#include "assetmanager.h"

AssetManager::AssetManager()
{
	playerTexture = new Texture(GL_TEXTURE_2D, "assets/textures/cube.png");
	sphereTexture = new Texture(GL_TEXTURE_2D, "assets/textures/sphere.png");
	tilesetTexture = new Texture(GL_TEXTURE_2D, "assets/textures/tileset.png");
	enemyTexture = new Texture(GL_TEXTURE_2D, "assets/textures/enemy.png");
	itemsTexture = new Texture(GL_TEXTURE_2D, "assets/textures/items.png");

	shaderProgram = new ShaderProgram("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl");
	
	wallMesh = new Mesh("assets/meshes/wall.mesh");
	cubeMesh = new Mesh("assets/meshes/cube.mesh");
	sphereMesh = new Mesh("assets/meshes/sphere.mesh");
	floorMesh = new Mesh("assets/meshes/floor.mesh");
	itemMesh = new Mesh("assets/meshes/item.mesh");
}

AssetManager::~AssetManager()
{
	delete playerTexture;
	delete sphereTexture;
	delete tilesetTexture;
	delete enemyTexture;
	delete itemsTexture;

	delete shaderProgram;
	
	delete wallMesh;
	delete cubeMesh;
	delete sphereMesh;
	delete floorMesh;
	delete itemMesh;

	for (auto sprite : meshDump)
		delete sprite;

	meshDump.clear();
}