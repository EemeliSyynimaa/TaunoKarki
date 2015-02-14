#include "gamescene.h"
#include "menuscene.h"
#include "game.h"
#include <iostream>
#include "gameobject.h"
#include "player.h"
#include "sprite.h"
#include "texture.h"
#include "componentmanager.h"
#include "shaderprogram.h"

GameScene::GameScene(Game& game) : Scene(game), spriteComponents(1), playerComponents(1)
{
	texture = new Texture(GL_TEXTURE_2D, "assets/textures/cube.pn");

	GameObject *plr = addGameObject();
	
	spriteComponents.addComponent(plr);
	playerComponents.addComponent(plr);

	ShaderProgram shaderProgram;

	shaderProgram.loadShaders("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl");
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

GameObject* GameScene::addGameObject()
{
	GameObject* gameObject = new GameObject();
	gameObjects.push_back(gameObject);

	return gameObject;
}

void GameScene::update()
{
	playerComponents.update();
}

void GameScene::draw()
{
	spriteComponents.update();
}