#include "gamescene.h"

GameScene::GameScene(SceneManager &sceneManager) : Scene(sceneManager)
{
}

GameScene::~GameScene()
{
}

void GameScene::update()
{
	sceneManager.change();
}

void GameScene::draw()
{
}