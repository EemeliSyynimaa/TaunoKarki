#include "assetmanager.h"

AssetManager::AssetManager()
{
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
	delete pistolMesh;
	delete machinegunMesh;
	delete shotgunMesh;
	delete healthpackMesh;
	delete ammoBarMesh;
	delete healthBarMesh;

	Mix_FreeChunk(pistolBangSound);
	Mix_FreeChunk(machinegunBangSound);
	Mix_FreeChunk(shotgunBangSound);
	Mix_FreeChunk(playerHitSound);
	Mix_FreeChunk(enemyDeadSound);
	Mix_FreeChunk(playerDeadSound);
	Mix_FreeChunk(powerupSound);
	Mix_FreeChunk(ambienceSound);

	for (auto sprite : meshDump)
		delete sprite;

	meshDump.clear();
}

void AssetManager::loadAssets()
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
	pistolMesh = new Mesh("assets/meshes/pistol.mesh");
	machinegunMesh = new Mesh("assets/meshes/machinegun.mesh");
	shotgunMesh = new Mesh("assets/meshes/shotgun.mesh");
	healthpackMesh = new Mesh("assets/meshes/healthpack.mesh");
	ammoBarMesh = new Mesh("assets/meshes/ammobar.mesh");
	healthBarMesh = new Mesh("assets/meshes/healthbar.mesh");

	pistolBangSound = Mix_LoadWAV("assets/sounds/pistol_bang.wav");
	machinegunBangSound = Mix_LoadWAV("assets/sounds/machinegun_bang.wav");
	shotgunBangSound = Mix_LoadWAV("assets/sounds/shotgun_bang.wav");
	playerHitSound = Mix_LoadWAV("assets/sounds/player_hit.wav");
	enemyDeadSound = Mix_LoadWAV("assets/sounds/enemy_dead.wav");
	playerDeadSound = Mix_LoadWAV("assets/sounds/player_dead.wav");
	powerupSound = Mix_LoadWAV("assets/sounds/powerup.wav");
	ambienceSound = Mix_LoadWAV("assets/sounds/ambience.wav");
}