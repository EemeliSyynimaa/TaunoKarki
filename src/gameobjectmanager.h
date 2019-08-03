#ifndef GAMEOBJECTMANAGER_H
#define GAMEOBJECTMANAGER_H

#include <list>
#include <functional>
#include "glm\glm.hpp"
#include "assetmanager.h"
#include "camera.h"

class Weapon;
class GameObject;
class Tilemap;

class GameObjectManager
{
public:
    GameObjectManager(AssetManager& assetManager, Camera& camera);
    ~GameObjectManager();

    void update(game_input* input);
    void draw();
    void createObjects();
    void deleteObjects();
    void addNewObject(std::function< void(void)> gameObject);
    GameObject* createPlayer(glm::vec3 pos, Weapon* weapon);
    GameObject* createEnemy(glm::vec3 pos, int level = 1, Tilemap* tilemap = nullptr);
    GameObject* createBullet(glm::vec3 pos, glm::vec2 direction, unsigned int owner, float damage, float speed);
    GameObject* createPlayerHealthBar(glm::vec3 pos, glm::vec3 size);
    GameObject* createPlayerAmmoBar(glm::vec3 pos, glm::vec3 size);
    GameObject* createRandomItem(glm::vec3 position);
    
    Camera& getCamera() { return camera; }
    AssetManager& getAssetManager() { return assetManager; }

    GameObject* getFirstObjectOfType(size_t type) const;
    size_t getNumberOfObjectsOfType(size_t type) const;

    void interpolate(float alpha);
private:
    GameObject* createObject();

    AssetManager& assetManager;
    Camera& camera;
    std::list<GameObject*> gameObjects;
    std::list<GameObject*> deadObjects;

    std::vector<std::function<void(void)>> newObjects;
};

#endif