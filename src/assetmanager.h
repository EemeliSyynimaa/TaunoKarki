#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <vector>
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

namespace GLOBALS
{
    const f32 PLAYER_HEALTH = 500.0f;
    const f32 PLAYER_SPEED = 0.2f;
    const f32 PLAYER_HEALTH_PER_PACK = 100.0f;
    
    const f32 ENEMY_HEALTH = 50.0f;
    const f32 ENEMY_SPEED = 0.125f;
    const f32 ENEMY_ACTIVATION_DISTANCE = 20.0f;
    const f32 ENEMY_HEALTH_PER_LEVEL = 10.0f;
    const f32 ENEMY_HIT_DAMAGE = 50.0f;
    const f32 ENEMY_HIT_DAMAGE_PER_LEVEL = 5.0f;
    const f32 ENEMY_ANGLE_OF_VISION = 60.0f;

    const f32 MACHINEGUN_DAMAGE = 25.0;
    const f32 MACHINEGUN_RELOAD_TIME = 2.50;
    const f32 MACHINEGUN_CLIP_SIZE = 25.0f;
    const f32 MACHINEGUN_BULLET_SPEED = 1.0f;
    const f32 MACHINEGUN_FIRE_RATE = 0.125f;
    const f32 MACHINEGUN_BULLET_SPREAD = 0.05f;

    const f32 PISTOL_DAMAGE = 30.0f;
    const f32 PISTOL_BULLET_SPEED = 1.0f;
    const f32 PISTOL_CLIP_SIZE = 8.0f;
    const f32 PISTOL_RELOAD_TIME = 1.5f;
    const f32 PISTOL_BULLET_SPREAD = 0.01f;

    const f32 SHOTGUN_DAMAGE = 12.5f;
    const f32 SHOTGUN_BULLET_SPEED = 1.0f;
    const f32 SHOTGUN_CLIP_SIZE = 7.0f;
    const f32 SHOTGUN_RELOAD_TIME = 3.0f;
    const f32 SHOTGUN_FIRE_RATE = 0.7f;
    const f32 SHOTGUN_BULLET_SPREAD = 0.125f;
    const s32 SHOTGUN_NUMBER_OF_SHELLS = 12;

    const f32 PROJECTILE_SIZE = 0.1f;
};

struct AssetManager
{
    AssetManager();
    ~AssetManager();

    ShaderProgram* shaderProgram;

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