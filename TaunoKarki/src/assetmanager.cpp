#include "assetmanager.h"

AssetManager::AssetManager()
{
	playerTexture = new Texture(GL_TEXTURE_2D, "assets/textures/cube.png");
	wallTexture = new Texture(GL_TEXTURE_2D, "assets/textures/wall.png");
	sphereTexture = new Texture(GL_TEXTURE_2D, "assets/textures/sphere.png");
	floorTexture = new Texture(GL_TEXTURE_2D, "assets/textures/floor.png");
	enemyTexture = new Texture(GL_TEXTURE_2D, "assets/textures/enemy.png");

	shaderProgram = new ShaderProgram("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl");
	
	wallMesh = new Mesh("assets/meshes/cube.mesh");
	sphereMesh = new Mesh("assets/meshes/sphere.mesh");
	floorMesh = new Mesh("assets/meshes/floor.mesh");
}

AssetManager::~AssetManager()
{
	delete playerTexture;
	delete wallTexture;
	delete sphereTexture;
	delete floorTexture;
	delete enemyTexture;

	delete shaderProgram;
	
	delete wallMesh;
	delete sphereMesh;
	delete floorMesh;
}