#include "gamescene.h"
#include "menuscene.h"
#include "game.h"
#include <iostream>
#include "glm/gtc/matrix_transform.hpp"
#include "meshrenderer.h"
#include "transform.h"

GameScene::GameScene(Game& game) : Scene(game), turnLeft(false), turnRight(false), moveForward(false), moveBackward(false)
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

	plr = new GameObject();
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
	/* follows player :-D
	viewMatrix = glm::lookAt(
		glm::vec3(plr->getComponent<Transform>()->getPosition().x, plr->getComponent<Transform>()->getPosition().y, 10),
		glm::vec3(plr->getComponent<Transform>()->getPosition().x, plr->getComponent<Transform>()->getPosition().y, 0),
		glm::vec3(0, 1, 0));
	*/

	float moveSpeed = 0.1f;
	float turnSpeed = 0.05f;

	if (turnLeft)
	{
		gameObjects.back()->getComponent<Transform>()->rotate(turnSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
		gameObjects.back()->getComponent<Transform>()->translate(glm::vec3(-moveSpeed, 0.0f, 0.0f));
	}

	if (turnRight)
	{ 
		gameObjects.back()->getComponent<Transform>()->rotate(turnSpeed, glm::vec3(0.0f, 0.0f, -1.0f));
		gameObjects.back()->getComponent<Transform>()->translate(glm::vec3(moveSpeed, 0.0f, 0.0f));
	}

	if (moveForward)
		gameObjects.back()->getComponent<Transform>()->translate(glm::vec3(0.0f, moveSpeed, 0.0f));

	if (moveBackward)
		gameObjects.back()->getComponent<Transform>()->translate(glm::vec3(0.0f, -moveSpeed, 0.0f));

	for (auto gameObject : gameObjects)
		gameObject->update();
}

void GameScene::draw()
{
}

void GameScene::handleEvent(SDL_Event& event)
{
	if (event.type == SDL_KEYDOWN)
	{ 
		if (event.key.keysym.sym == SDLK_UP)
			moveForward = true;
		if (event.key.keysym.sym == SDLK_DOWN)
			moveBackward = true;
		if (event.key.keysym.sym == SDLK_RIGHT)
			turnRight = true;
		if (event.key.keysym.sym == SDLK_LEFT)
			turnLeft = true;
	}
	else if (event.type == SDL_KEYUP)
	{
		if (event.key.keysym.sym == SDLK_UP)
			moveForward = false;
		if (event.key.keysym.sym == SDLK_DOWN)
			moveBackward = false;
		if (event.key.keysym.sym == SDLK_RIGHT)
			turnRight = false;
		if (event.key.keysym.sym == SDLK_LEFT)
			turnLeft = false;
	}

}