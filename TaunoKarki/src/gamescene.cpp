#include "gamescene.h"
#include "menuscene.h"
#include "game.h"

GameScene::GameScene(Game& game) : Scene(game), world(b2Vec2(0.0f, 0.0f)), gameObjectManager(assetManager, world, camera), collisionHandler()
{
	world.SetContactListener(&collisionHandler);

	camera.createNewPerspectiveMatrix(60.0f, (float)game.getScreenWidth(), (float)game.getScreenHeight(), 0.1f, 100.0f);
	camera.setPosition(glm::vec3(0.0f, 0.0f, 20.0f));

	glClearColor(0.5f, 0.0f, 0.0f, 0.0f);

	gameObjectManager.createPlayer(glm::vec3(4.0f, -4.0f, 0.0f));
	gameObjectManager.createEnemy(glm::vec3(20.0f, -28.0f, 0.0f));

	tilemap = new Tilemap("assets/maps/mappi.txt", assetManager, camera, world);
}

GameScene::~GameScene()
{
	delete tilemap;
}

void GameScene::update(float deltaTime)
{
	world.Step(1 / 60.0f, 8, 3);

	gameObjectManager.update(deltaTime);
}

void GameScene::draw()
{
	tilemap->draw();
}

void GameScene::handleEvent(SDL_Event& event)
{
}