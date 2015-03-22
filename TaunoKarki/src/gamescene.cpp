#include "gamescene.h"
#include "menuscene.h"
#include "game.h"

GameScene::GameScene(Game& game) : Scene(game), world(b2Vec2(0.0f, 0.0f)), gameObjectManager(assetManager, world, camera), collisionHandler(), tilemap(glm::vec3(0.0f), assetManager, camera, world)
{
	world.SetContactListener(&collisionHandler);

	camera.createNewPerspectiveMatrix(60.0f, (float)game.getScreenWidth(), (float)game.getScreenHeight(), 0.1f, 100.0f);
	camera.setPosition(glm::vec3(0.0f, 0.0f, 20.0f));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	tilemap.generate(101, 101);

	gameObjectManager.createPlayer(tilemap.getStartingPosition());

	while (tilemap.getNumberOfStartingPositions() > 0)
	{
		gameObjectManager.createEnemy(tilemap.getStartingPosition());
	}
}

GameScene::~GameScene()
{
}

void GameScene::update(float deltaTime)
{
	world.Step(1 / 60.0f, 8, 3);

	gameObjectManager.update(deltaTime);
}

void GameScene::draw()
{
	tilemap.draw();
}

void GameScene::handleEvent(SDL_Event& event)
{
}