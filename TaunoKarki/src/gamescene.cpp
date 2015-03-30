#include "gamescene.h"
#include "menuscene.h"
#include "game.h"
#include "gameobject.h"

#include <iostream>

GameScene::GameScene(Game& game, int level) : Scene(game), world(b2Vec2(0.0f, 0.0f)), gameObjectManager(assetManager, world, camera), collisionHandler(), tilemap(glm::vec3(0.0f), assetManager, camera, world), level(level)
{
	std::cout << "GAMESCENE ALIVE - entering level " << level << std::endl;

	world.SetContactListener(&collisionHandler);

	camera.createNewPerspectiveMatrix(60.0f, (float)game.getScreenWidth(), (float)game.getScreenHeight(), 0.1f, 100.0f);
	camera.setPosition(glm::vec3(0.0f, 0.0f, 20.0f));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	tilemap.generate(7 + level * 4, 7 + level * 4);

	gameObjectManager.createPlayer(tilemap.getStartingPosition());

	while (tilemap.getNumberOfStartingPositions() > 0)
	{
		gameObjectManager.createEnemy(tilemap.getStartingPosition());
	}
}

GameScene::~GameScene()
{
	std::cout << "GAMESCENE DIE - leaving level " << level << std::endl;
}

void GameScene::update(float deltaTime)
{
	world.Step(1 / 60.0f, 8, 3);

	gameObjectManager.update(deltaTime);

	if (gameObjectManager.getNumberOfObjectsOfType(GAMEOBJECT_TYPES::PLAYER) == 0)
	{
		std::cout << "PLAYER LOST - died on level " << level << std::endl;
		game.getSceneManager().change(new MenuScene(game));
	}
	else if (gameObjectManager.getNumberOfObjectsOfType(GAMEOBJECT_TYPES::ENEMY) == 0)
	{
		std::cout << "PLAYER WON - cleared level " << level << std::endl;
		game.getSceneManager().change(new GameScene(game, level + 1));
	}
}

void GameScene::draw()
{
	tilemap.draw();
}

void GameScene::handleEvent(SDL_Event& event)
{
}