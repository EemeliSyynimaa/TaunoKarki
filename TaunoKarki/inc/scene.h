#ifndef SCENE_H
#define SCENE_H

#include "SDL/SDL_events.h"
class SceneManager;
class Game;

class Scene
{
public:
	Scene(Game& game);
	virtual ~Scene();

	virtual void update(float deltaTime) = 0;
	virtual void draw() = 0;
	virtual void handleEvent(SDL_Event& event) = 0;
protected:
	Game& game;
};

#endif
