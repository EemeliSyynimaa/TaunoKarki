#ifndef GAMESCENE_H
#define GAMESCENE_H

#include "scene.h"
#include "gameobject.h"
#include "component.h"
#include "texture.h"

class GameScene : public Scene
{
public:
	GameScene(Game& game);
	~GameScene();

	void update();
	void draw();
	void addGameObject(int numberOfComponents, ...);
private:
	Texture* texture;
	std::vector<GameObject*> gameObjects;
};

#endif