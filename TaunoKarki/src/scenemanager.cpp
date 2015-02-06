#include <algorithm>
#include "scenemanager.h"
#include <iostream>

SceneManager::SceneManager()
{
}

SceneManager::~SceneManager()
{
	for (auto& scene : scenes)
	{
		delete scene;
	}

	scenes.clear();
}

void SceneManager::update()
{
	scenes.back()->update();
}

void SceneManager::draw()
{
	scenes.back()->draw();
}

void SceneManager::push(Scene* scene)
{
	scenes.push_back(scene);
}

void SceneManager::pop()
{
	delete scenes.back();
	scenes.pop_back();
}

void SceneManager::change(Scene* scene)
{
	if (!scenes.empty())
	{
		delete scenes.back();
		scenes.pop_back();
	}
		
	scenes.push_back(scene);
}
