#include "gamescene.h"
#include "menuscene.h"
#include "game.h"
#include <iostream>
#include "gameobject.h"
#include "player.h"
#include <cstdarg>

GameScene::GameScene(Game& game) : Scene(game)
{
	addGameObject(1, new Player());
}

GameScene::~GameScene()
{
	for (auto& gameObject : gameObjects)
	{
		delete gameObject;
	}

	gameObjects.clear();
}

void GameScene::addGameObject(int numberOfComponents, ...)
{
	GameObject* gameObject = new GameObject();

	va_list vl;
	va_start(vl, numberOfComponents);
	for (unsigned int i = 0; i < numberOfComponents; i++)
	{
		gameObject->addComponent(va_arg(vl, Component*));
	}

	va_end(vl);

	gameObjects.push_back(gameObject);
}

void GameScene::update()
{
	for (auto& gameObject : gameObjects)
	{
		gameObject->update();
	}
}

void GameScene::draw()
{
}