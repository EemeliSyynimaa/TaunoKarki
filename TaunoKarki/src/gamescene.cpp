#include "gamescene.h"
#include "menuscene.h"
#include "game.h"
#include <iostream>
#include "gameobject.h"
#include "player.h"

GameScene::GameScene(Game& game) : Scene(game)
{
	// Testing
	GameObject testi;

	testi.addComponent(new Player());
}

GameScene::~GameScene()
{
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