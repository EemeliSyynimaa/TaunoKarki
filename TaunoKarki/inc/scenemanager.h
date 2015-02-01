#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <vector>
#include "scene.h"

class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	void update();
	void draw();

	void push(Scene* scene);
	void pop();
	void change(Scene* scene);
private:

	std::vector<Scene*> scenes;
};

#endif