#include "gamescene.h"
#include "menuscene.h"
#include "game.h"
#include <iostream>
#include "gameobject.h"
#include "player.h"
#include "sprite.h"
#include "texture.h"
#include <cstdarg>

GameScene::GameScene(Game& game) : Scene(game)
{
	texture = new Texture(GL_TEXTURE_2D, "assets/textures/cube.pn");
	addGameObject(2, new Player(), new Sprite(texture));
}

GameScene::~GameScene()
{
	for (auto& gameObject : gameObjects)
	{
		delete gameObject;
	}

	gameObjects.clear();

	delete texture;
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