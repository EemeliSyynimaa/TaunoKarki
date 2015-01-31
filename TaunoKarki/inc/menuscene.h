#ifndef MENUSCENE_H
#define MENUSCENE_H

#include "scene.h"

class MenuScene : public Scene
{
public:
	MenuScene(SceneManager &sceneManager);
	~MenuScene();

	void update();
	void draw();
private:
};

#endif