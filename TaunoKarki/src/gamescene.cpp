#include "gamescene.h"
#include "menuscene.h"
#include "game.h"
#include <iostream>
#include "glm/gtc/matrix_transform.hpp"
#include "meshrenderer.h"
#include "transform.h"
#include "tilemap.h"

GameScene::GameScene(Game& game) : Scene(game), turnLeft(false), turnRight(false), moveForward(false), moveBackward(false)
{
	projectionMatrix = glm::perspective(glm::radians(60.0f),
		static_cast <float>(game.getScreenWidth()) / game.getScreenHeight(),
		0.1f, 100.0f);
	viewMatrix = glm::lookAt(
		glm::vec3(0, 0, 25),
		glm::vec3(0, 0, 0),
		glm::vec3(0, 1, 0));

	glClearColor(0.5f, 0.0f, 0.0f, 0.0f);

	texture = new Texture(GL_TEXTURE_2D, "assets/textures/cube.png");
	mapTexture = new Texture(GL_TEXTURE_2D, "assets/textures/wall.png");
	shaderProgram = new ShaderProgram("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl");
	mesh = new Mesh("assets/meshes/cube.mesh");

	plr = new GameObject();
	plr->addComponent(new Transform(plr, 4.0f, -4.0f, 0));

	plr->addComponent(new MeshRenderer(plr));
	plr->getComponent<MeshRenderer>()->setMesh(mesh);
	plr->getComponent<MeshRenderer>()->setProgram(shaderProgram);
	plr->getComponent<MeshRenderer>()->setProjectionMatrix(&projectionMatrix);
	plr->getComponent<MeshRenderer>()->setViewMatrix(&viewMatrix);
	plr->getComponent<MeshRenderer>()->setTexture(texture);

	gameObjects.push_back(plr);

	tilemap = new Tilemap("assets/maps/mappi.txt", mesh, mapTexture, shaderProgram, &viewMatrix, &projectionMatrix);
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
	tilemap->draw();

	int x, y, z;
	
	SDL_GetMouseState(&x, &y);
	glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
	glm::vec3 mouse(x, y, z);
	glm::vec4 viewport(0.0f, 0.0f, game.getScreenWidth(), game.getScreenHeight());
	glm::vec3 mouseInDaWorld = glm::unProject(mouse, viewMatrix, projectionMatrix, viewport);
	
	//plr->getComponent<Transform>()->setPosition(glm::vec3(mouseInDaWorld.x, mouseInDaWorld.y, 0));
	//std::cout << mouseInDaWorld.x << ", " << mouseInDaWorld.y << ", " << mouseInDaWorld.z << ", " << std::endl;

	//plr->getComponent<Transform>()->lookAt(mouseInDaWorld);
	float moveSpeed = 0.1f;
	float turnSpeed = 0.05f;

	if (turnLeft)
	{
		plr->getComponent<Transform>()->translate(glm::vec3(-moveSpeed, 0.0f, 0.0f));
	}

	if (turnRight)
	{ 
		plr->getComponent<Transform>()->translate(glm::vec3(moveSpeed, 0.0f, 0.0f));
	}

	if (moveForward)
		plr->getComponent<Transform>()->translate(glm::vec3(0.0f, moveSpeed, 0.0f));

	if (moveBackward)
		plr->getComponent<Transform>()->translate(glm::vec3(0.0f, -moveSpeed, 0.0f));

	viewMatrix = glm::lookAt(
		glm::vec3(plr->getComponent<Transform>()->getPosition().x, plr->getComponent<Transform>()->getPosition().y, 25),
		glm::vec3(plr->getComponent<Transform>()->getPosition().x, plr->getComponent<Transform>()->getPosition().y, 0),
		glm::vec3(0, 1, 0));

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
		if (event.key.keysym.sym == SDLK_w)
			moveForward = true;
		if (event.key.keysym.sym == SDLK_s)
			moveBackward = true;
		if (event.key.keysym.sym == SDLK_d)
			turnRight = true;
		if (event.key.keysym.sym == SDLK_a)
			turnLeft = true;
	}
	else if (event.type == SDL_KEYUP)
	{
		if (event.key.keysym.sym == SDLK_w)
			moveForward = false;
		if (event.key.keysym.sym == SDLK_s)
			moveBackward = false;
		if (event.key.keysym.sym == SDLK_d)
			turnRight = false;
		if (event.key.keysym.sym == SDLK_a)
			turnLeft = false;
	}

}