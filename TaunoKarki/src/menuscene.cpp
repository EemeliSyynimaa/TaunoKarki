#include "menuscene.h"
#include "game.h"
#include "gamescene.h"
#include <iostream>

MenuScene::MenuScene(Game& game) : Scene(game)
{
}

MenuScene::~MenuScene()
{
}

void MenuScene::update(float deltaTime)
{
	std::cout << "MENUSCENE WORKS TOO" << std::endl;
	game.getSceneManager().change(new GameScene(game));
}

void MenuScene::draw()
{
}

void MenuScene::handleEvent(SDL_Event& event)
{
}