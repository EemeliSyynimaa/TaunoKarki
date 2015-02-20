#ifndef MENUSCENE_H
#define MENUSCENE_H

#include "SDL/SDL_events.h"
#include "scene.h"

class MenuScene : public Scene
{
public:
	MenuScene(Game& game);
	~MenuScene();

	void handleEvent(SDL_Event& event);
	void update();
	void draw();
private:
};

#endif