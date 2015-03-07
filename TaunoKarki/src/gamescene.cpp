#include "gamescene.h"
#include "menuscene.h"
#include "game.h"
#include "glm/gtc/matrix_transform.hpp"

#include "meshrenderer.h"
#include "transform.h"
#include "rigidbody.h"
#include "circlecollider.h"
#include "playercontroller.h"

GameScene::GameScene(Game& game) : Scene(game), world(b2Vec2(0.0f, 0.0f)), gameObjectManager(assetManager)
{
	camera.createNewPerspectiveMatrix(60.0f, (float)game.getScreenWidth(), (float)game.getScreenHeight(), 0.1f, 100.0f);
	camera.setPosition(glm::vec3(0.0f, 0.0f, 20.0f));

	glClearColor(0.5f, 0.0f, 0.0f, 0.0f);

	plr = gameObjectManager.createPlayer(glm::vec3(4.0f, -4.0f, 0.0f));	
	plr->addComponent(new Transform(plr, 4.0f, -4.0f, 0));
	plr->addComponent(new CircleCollider(plr, 1.0f));
	plr->addComponent(new MeshRenderer(plr));
	plr->getComponent<MeshRenderer>()->setMesh(assetManager.wallMesh);
	plr->getComponent<MeshRenderer>()->setProgram(assetManager.shaderProgram);
	plr->getComponent<MeshRenderer>()->setCamera(camera);
	plr->getComponent<MeshRenderer>()->setTexture(assetManager.playerTexture);

	plr->addComponent(new RigidBody(plr, world));
	plr->getComponent<RigidBody>()->getBody()->SetFixedRotation(true);
	plr->addComponent(new PlayerController(plr));

	tilemap = new Tilemap("assets/maps/mappi.txt", assetManager, camera, world);
}

GameScene::~GameScene()
{
	delete tilemap;
}

void GameScene::update(float deltaTime)
{
	world.Step(1 / 60.0f, 8, 3);

	int x, y;
	b2Body* plrBody = plr->getComponent<RigidBody>()->getBody();

	SDL_GetMouseState(&x, &y);
	float halfX = (float(game.getScreenWidth()) / 2.0f - x) * -1;
	float halfY = (float(game.getScreenHeight()) / 2.0f - y) * -1;

	glm::vec3 mouseInDaWorld(halfX, halfY, 0.0f);

	plr->getComponent<Transform>()->lookAt(plr->getComponent<Transform>()->getPosition()-mouseInDaWorld);

	camera.follow(glm::vec2(plr->getComponent<Transform>()->getPosition().x, plr->getComponent<Transform>()->getPosition().y));

	gameObjectManager.update(deltaTime);
}

void GameScene::draw()
{
	tilemap->draw();
}

void GameScene::handleEvent(SDL_Event& event)
{
	if (event.type == SDL_MOUSEBUTTONDOWN)
	{

		if (event.button.button == SDL_BUTTON_LEFT)
		{
			// Ammutaan! :D
			GameObject* obj = gameObjectManager.createBullet(plr->getComponent<Transform>()->getPosition());
			obj->addComponent(new Transform(obj,
				plr->getComponent<Transform>()->getPosition().x + plr->getComponent<Transform>()->getDirVec().x,
				plr->getComponent<Transform>()->getPosition().y - plr->getComponent<Transform>()->getDirVec().y,
				0));
			obj->addComponent(new CircleCollider(obj, 0.1f));
			obj->addComponent(new MeshRenderer(obj));
			obj->addComponent(new RigidBody(obj, world));
			obj->getComponent<Transform>()->setScale(glm::vec3(0.1f));

			MeshRenderer* temp = obj->getComponent<MeshRenderer>();
			temp->setMesh(assetManager.sphereMesh);
			temp->setTexture(assetManager.sphereTexture);
			temp->setCamera(camera);
			temp->setProgram(assetManager.shaderProgram);

			b2Body* body = obj->getComponent<RigidBody>()->getBody();
			body->SetBullet(true);
			b2Vec2 forceDir(plr->getComponent<Transform>()->getDirVec().x, -plr->getComponent<Transform>()->getDirVec().y);

			forceDir *= 2.0f;
			body->ApplyLinearImpulse(forceDir, body->GetWorldCenter(), true);
		}
	}

}