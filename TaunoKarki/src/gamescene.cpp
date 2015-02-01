#include "gamescene.h"
#include "menuscene.h"
#include "game.h"
#include <iostream>

GameScene::GameScene(Game& game) : Scene(game)
{
}

GameScene::~GameScene()
{
}

void GameScene::update()
{
	std::cout << "GAMESCENE WORKS" << std::endl;

	game.getSceneManager().change(new MenuScene(game));
}

void GameScene::draw()
{
}