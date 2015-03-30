#include "menuscene.h"
#include "game.h"
#include "gamescene.h"
#include <iostream>

MenuScene::MenuScene(Game& game) : Scene(game)
{
	std::cout << "MENUSCENE ALIVE" << std::endl;
}

MenuScene::~MenuScene()
{
	std::cout << "MENUSCENE DIE" << std::endl;
}

void MenuScene::update(float deltaTime)
{
}

void MenuScene::draw()
{
}

void MenuScene::handleEvent(SDL_Event& event)
{
	if (event.key.keysym.sym == SDLK_SPACE)
		game.getSceneManager().change(new GameScene(game, 1));
}