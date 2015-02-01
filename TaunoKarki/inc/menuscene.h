#ifndef MENUSCENE_H
#define MENUSCENE_H

#include "scene.h"

class MenuScene : public Scene
{
public:
	MenuScene(Game& game);
	~MenuScene();

	void update();
	void draw();
private:
};

#endif