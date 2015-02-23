#include "gamescene.h"
#include "menuscene.h"
#include "game.h"
#include <iostream>
#include "glm/gtc/matrix_transform.hpp"
#include "tilemap.h"

#include "meshrenderer.h"
#include "transform.h"
#include "rigidbody.h"
#include "circlecollider.h"
#define DEGTORAD 0.0174532925199432957f

GameScene::GameScene(Game& game) : Scene(game), turnLeft(false), turnRight(false), moveForward(false), moveBackward(false), world(b2Vec2(0.0f, 0.0f))
{
	projectionMatrix = glm::perspective(glm::radians(60.0f),
		static_cast <float>(game.getScreenWidth()) / game.getScreenHeight(),
		0.1f, 100.0f);

	glClearColor(0.5f, 0.0f, 0.0f, 0.0f);

	texture = new Texture(GL_TEXTURE_2D, "assets/textures/cube.png");
	mapTexture = new Texture(GL_TEXTURE_2D, "assets/textures/wall.png");
	sphereTexture = new Texture(GL_TEXTURE_2D, "assets/textures/sphere.png");
	shaderProgram = new ShaderProgram("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl");
	mesh = new Mesh("assets/meshes/cube.mesh");
	sphereMesh = new Mesh("assets/meshes/sphere.mesh");

	plr = new GameObject();
	plr->addComponent(new Transform(plr, 4.0f, -4.0f, 0));
	plr->addComponent(new CircleCollider(plr, 1.0f));
	plr->addComponent(new MeshRenderer(plr));
	plr->getComponent<MeshRenderer>()->setMesh(mesh);
	plr->getComponent<MeshRenderer>()->setProgram(shaderProgram);
	plr->getComponent<MeshRenderer>()->setProjectionMatrix(&projectionMatrix);
	plr->getComponent<MeshRenderer>()->setViewMatrix(&viewMatrix);
	plr->getComponent<MeshRenderer>()->setTexture(texture);

	plr->addComponent(new Rigidbody(plr, world));
	plr->getComponent<Rigidbody>()->getBody()->SetFixedRotation(true);

	gameObjects.push_back(plr);

	tilemap = new Tilemap("assets/maps/mappi.txt", mesh, mapTexture, shaderProgram, &viewMatrix, &projectionMatrix, world);
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
	world.Step(1 / 60.0f, 8, 3);
	tilemap->draw();

	int x, y;
	b2Body* plrBody = plr->getComponent<Rigidbody>()->getBody();

	SDL_GetMouseState(&x, &y);
	float halfX = float(game.getScreenWidth()) / 2.0f;
	float halfY = float(game.getScreenHeight()) / 2.0f;

	glm::vec3 mouseInDaWorld((halfX - x) / -halfX, (halfY - y) / -halfY, 0.0f);

	plr->getComponent<Transform>()->lookAt(plr->getComponent<Transform>()->getPosition()-mouseInDaWorld);

	float moveSpeed = 15.0f;
	float turnSpeed = 0.05f;

	b2Vec2 desiredVel(0.0f, 0.0f);
	b2Vec2 vel = plrBody->GetLinearVelocity();
	if (turnLeft)
		desiredVel.x = -moveSpeed;
	else if (turnRight)
		desiredVel.x = moveSpeed;
	else 
		desiredVel.x = 0.0f;

	if (moveForward)
		desiredVel.y = moveSpeed;
	else if (moveBackward)
		desiredVel.y = -moveSpeed;
	else
		desiredVel.y = 0.0f;

	b2Vec2 velChange(0.0f, 0.0f);
	velChange.x = desiredVel.x - vel.x;
	velChange.y = desiredVel.y - vel.y;
	
	b2Vec2 impulse(0.0f, 0.0f);
	impulse.x = plrBody->GetMass() * velChange.x;
	impulse.y = plrBody->GetMass() * velChange.y;

	plrBody->ApplyLinearImpulse(impulse, plrBody->GetWorldCenter(), true);

	viewMatrix = glm::lookAt(
		glm::vec3(plr->getComponent<Transform>()->getPosition().x, plr->getComponent<Transform>()->getPosition().y, 15),
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
		if (event.key.keysym.sym == SDLK_SPACE)
		{
			// Ammutaan! :D
			GameObject* obj = new GameObject();
			obj->addComponent(new Transform(obj, 
				plr->getComponent<Transform>()->getPosition().x + plr->getComponent<Transform>()->getDirVec().x,
				plr->getComponent<Transform>()->getPosition().y - plr->getComponent<Transform>()->getDirVec().y,
				0));
			obj->addComponent(new CircleCollider(obj, 0.1f));
			obj->addComponent(new MeshRenderer(obj));
			obj->addComponent(new Rigidbody(obj, world));

			MeshRenderer* temp = obj->getComponent<MeshRenderer>();
			temp->setMesh(sphereMesh);
			temp->setTexture(sphereTexture);
			temp->setProjectionMatrix(&projectionMatrix);
			temp->setViewMatrix(&viewMatrix);
			temp->setProgram(shaderProgram);

			b2Body* body = obj->getComponent<Rigidbody>()->getBody();
			b2Vec2 forceDir(plr->getComponent<Transform>()->getDirVec().x, -plr->getComponent<Transform>()->getDirVec().y);

			forceDir *= 2.0f;
			body->ApplyLinearImpulse(forceDir, body->GetWorldCenter(), true);

			gameObjects.push_back(obj);
		}
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