#include "menuscene.h"
#include "game.h"
#include "gamescene.h"
#include <iostream>

MenuScene::MenuScene(Game& game) : Scene(game), gameObjectManager(game.getAssetManager(), camera)
{
	std::cout << "MENUSCENE ALIVE" << std::endl;

	camera.createNewPerspectiveMatrix(60.0f, (float)game.getScreenWidth(), (float)game.getScreenHeight(), 0.1f, 100.0f);
	camera.createNewOrthographicMatrix((float)game.getScreenWidth(), (float)game.getScreenHeight());
	camera.setPosition(glm::vec3(0.0f, 0.0f, 20.0f));
	camera.setOffset(0.0f, 0.0f, 0.0f);

	gameObjectManager.createMenuBlock(glm::vec3(0.0f, 0.0f, 0.0f));

	glClearColor(0.5f, 0.0f, 0.5f, 0.0f);
}

MenuScene::~MenuScene()
{
	std::cout << "MENUSCENE DIE" << std::endl;
}

void MenuScene::update(float deltaTime)
{
	accumulator += deltaTime;

	while (accumulator >= step)
	{
		accumulator -= step;
		gameObjectManager.update();
	}

	gameObjectManager.interpolate(accumulator / step);
}

void MenuScene::draw()
{
	gameObjectManager.draw();
}

void MenuScene::handleEvent(SDL_Event& event)
{
	if (event.type == SDL_KEYDOWN)
	{
		if (event.key.keysym.sym == SDLK_SPACE)
			game.getSceneManager().change(new GameScene(game, 1, nullptr));
		else if (event.key.keysym.sym == SDLK_ESCAPE)
			game.stop();
	}
}