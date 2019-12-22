#include "assetmanager.h"

AssetManager::AssetManager()
{
}

AssetManager::~AssetManager()
{
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

    for (auto sprite : meshDump)
        delete sprite;

    meshDump.clear();
}

void AssetManager::loadAssets()
{
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
}