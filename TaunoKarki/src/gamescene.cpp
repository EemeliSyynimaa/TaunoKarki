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

GameScene::GameScene(Game& game) : Scene(game), turnLeft(false), turnRight(false), moveForward(false), moveBackward(false), world(b2Vec2(0.0f, 0.0f))
{
	camera.createNewPerspectiveMatrix(60.0f, (float)game.getScreenWidth(), (float)game.getScreenHeight(), 0.1f, 100.0f);
	camera.setPosition(glm::vec3(0.0f, 0.0f, 20.0f));

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
	plr->getComponent<MeshRenderer>()->setCamera(camera);
	plr->getComponent<MeshRenderer>()->setTexture(texture);

	plr->addComponent(new RigidBody(plr, world));
	plr->getComponent<RigidBody>()->getBody()->SetFixedRotation(true);

	gameObjects.push_back(plr);

	tilemap = new Tilemap("assets/maps/mappi.txt", mesh, mapTexture, shaderProgram, camera, world);
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

void GameScene::update(float deltaTime)
{
	world.Step(1 / 60.0f, 8, 3);
	tilemap->draw();

	int x, y;
	b2Body* plrBody = plr->getComponent<RigidBody>()->getBody();

	SDL_GetMouseState(&x, &y);
	float halfX = (float(game.getScreenWidth()) / 2.0f - x) * -1;
	float halfY = (float(game.getScreenHeight()) / 2.0f - y) * -1;

	glm::vec3 mouseInDaWorld(halfX, halfY, 0.0f);

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

	camera.follow(glm::vec2(plr->getComponent<Transform>()->getPosition().x, plr->getComponent<Transform>()->getPosition().y));

	for (auto gameObject : gameObjects)
		gameObject->update(deltaTime);
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
	else if (event.type == SDL_MOUSEBUTTONDOWN)
	{

		if (event.button.button == SDL_BUTTON_LEFT)
		{
			// Ammutaan! :D
			GameObject* obj = new GameObject();
			obj->addComponent(new Transform(obj,
				plr->getComponent<Transform>()->getPosition().x + plr->getComponent<Transform>()->getDirVec().x,
				plr->getComponent<Transform>()->getPosition().y - plr->getComponent<Transform>()->getDirVec().y,
				0));
			obj->addComponent(new CircleCollider(obj, 0.1f));
			obj->addComponent(new MeshRenderer(obj));
			obj->addComponent(new RigidBody(obj, world));
			obj->getComponent<Transform>()->setScale(glm::vec3(0.1f));

			MeshRenderer* temp = obj->getComponent<MeshRenderer>();
			temp->setMesh(sphereMesh);
			temp->setTexture(sphereTexture);
			temp->setCamera(camera);
			temp->setProgram(shaderProgram);

			b2Body* body = obj->getComponent<RigidBody>()->getBody();
			body->SetBullet(true);
			b2Vec2 forceDir(plr->getComponent<Transform>()->getDirVec().x, -plr->getComponent<Transform>()->getDirVec().y);

			forceDir *= 2.0f;
			body->ApplyLinearImpulse(forceDir, body->GetWorldCenter(), true);

			gameObjects.push_back(obj);
		}
	}

}