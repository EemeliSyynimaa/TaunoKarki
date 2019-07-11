#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <vector>
#include "SDL/SDL_mixer.h"
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

namespace GLOBALS
{
    const float PLAYER_HEALTH = 500.0f;
    const float PLAYER_SPEED = 10.0f;
    const float PLAYER_HEALTH_PER_PACK = 100.0f;
    
    const float ENEMY_HEALTH = 50.0f;
    const float ENEMY_SPEED = 7.0f;
    const float ENEMY_ACTIVATION_DISTANCE = 20.0f;
    const float ENEMY_HEALTH_PER_LEVEL = 10.0f;
    const float ENEMY_HIT_DAMAGE = 50.0f;
    const float ENEMY_HIT_DAMAGE_PER_LEVEL = 5.0f;
    const float ENEMY_ANGLE_OF_VISION = 60.0f;

    const float MACHINEGUN_DAMAGE = 25.0;
    const float MACHINEGUN_RELOAD_TIME = 2500.0f;
    const float MACHINEGUN_CLIP_SIZE = 25.0f;
    const float MACHINEGUN_BULLET_SPEED = 1.5f;
    const float MACHINEGUN_FIRE_RATE = 125.0f;
    const float MACHINEGUN_BULLET_SPREAD = 0.05f;

    const float PISTOL_DAMAGE = 30.0f;
    const float PISTOL_BULLET_SPEED = 1.5f;
    const float PISTOL_CLIP_SIZE = 8.0f;
    const float PISTOL_RELOAD_TIME = 1500.0f;
    const float PISTOL_BULLET_SPREAD = 0.01f;

    const float SHOTGUN_DAMAGE = 12.5f;
    const float SHOTGUN_BULLET_SPEED = 1.5f;
    const float SHOTGUN_CLIP_SIZE = 7.0f;
    const float SHOTGUN_RELOAD_TIME = 4000.0f;
    const float SHOTGUN_FIRE_RATE = 1000.0f;
    const float SHOTGUN_BULLET_SPREAD = 0.125f;
    const int SHOTGUN_NUMBER_OF_SHELLS = 12;

    const float PROJECTILE_SIZE = 0.1f;
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

    tk_sound_sample* pistolBangSound;
    tk_sound_sample* machinegunBangSound;
    tk_sound_sample* shotgunBangSound;
    tk_sound_sample* playerHitSound;
    tk_sound_sample* enemyDeadSound;
    tk_sound_sample* playerDeadSound;
    tk_sound_sample* powerupSound;
    tk_sound_sample* ambienceSound;

    Mesh* addMesh(Mesh mesh);

    std::vector<Mesh*> meshDump;

    void loadAssets();
    void addMesh(Mesh* mesh) { meshDump.push_back(mesh); }
};

#endif