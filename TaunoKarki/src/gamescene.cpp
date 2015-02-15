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
#include "glm/gtc/matrix_transform.hpp">

GameScene::GameScene(Game& game) : Scene(game), spriteComponents(1), playerComponents(1)
{
	projectionMatrix = glm::perspective(glm::radians(60.0f),
		static_cast <float>(game.getScreenWidth()) / game.getScreenHeight(),
		0.1f, 100.0f);

	viewMatrix = glm::lookAt(
		glm::vec3(0, 0, 10),
		glm::vec3(0, 0, 0),
		glm::vec3(0, 1, 0));

	glClearColor(0.5f, 0.0f, 0.0f, 0.0f);

	texture = new Texture(GL_TEXTURE_2D, "assets/textures/cube.png");
	shaderProgram = new ShaderProgram();
	shaderProgram->loadShaders("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl");

	GameObject *plr = addGameObject();
	
	spriteComponents.addComponent(plr);
	playerComponents.addComponent(plr);

	Sprite* temp = plr->getComponent<Sprite>();
	temp->setProgram(shaderProgram);
	temp->setTexture(texture);
	temp->setViewMatrix(&viewMatrix);
	temp->setProjectionMatrix(&projectionMatrix);
}

GameScene::~GameScene()
{
	for (auto& gameObject : gameObjects)
	{
		delete gameObject;
	}

	gameObjects.clear();

	delete texture;
	delete shaderProgram;
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