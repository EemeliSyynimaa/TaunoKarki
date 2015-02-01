#ifndef SCENE_H
#define SCENE_H

class SceneManager;
class Game;

class Scene
{
public:
	Scene(Game& game);
	virtual ~Scene();

	virtual void update() = 0;
	virtual void draw() = 0;
protected:
	Game& game;
};

#endif
