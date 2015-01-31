#ifndef SCENE_H
#define SCENE_H

#include "scenemanager.h"

class Scene
{
public:
	Scene(SceneManager &sceneManager);
	virtual ~Scene();

	virtual void update() = 0;
	virtual void draw() = 0;

protected:
	SceneManager &sceneManager;
};

#endif