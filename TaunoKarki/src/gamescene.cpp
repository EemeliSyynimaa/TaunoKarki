#include "gamescene.h"
#include "menuscene.h"
#include "game.h"
#include <iostream>
#include "glm/gtc/matrix_transform.hpp"
#include "meshrenderer.h"
#include "transform.h"

GameScene::GameScene(Game& game) : Scene(game)
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
	mesh = new Mesh();
	mesh->load("assets/meshes/cube.mesh");

	GameObject *plr = new GameObject();
	plr->addComponent(new Transform(plr));
	plr->addComponent(new MeshRenderer(plr));
	plr->getComponent<MeshRenderer>()->setMesh(mesh);
	plr->getComponent<MeshRenderer>()->setProgram(shaderProgram);
	plr->getComponent<MeshRenderer>()->setProjectionMatrix(&projectionMatrix);
	plr->getComponent<MeshRenderer>()->setViewMatrix(&viewMatrix);
	plr->getComponent<MeshRenderer>()->setTexture(texture);

	

	gameObjects.push_back(plr);
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
	delete mesh;
}

void GameScene::update()
{
	for (auto gameObject : gameObjects)
		gameObject->update();
}

void GameScene::draw()
{
}