#ifndef GAMESCENE_H
#define GAMESCENE_H

#include "scene.h"
#include "gameobject.h"

class GameScene : public Scene
{
public:
	GameScene(Game& game);
	~GameScene();

	void update();
	void draw();
private:

	std::vector<GameObject*> gameObjects;
};

#endif