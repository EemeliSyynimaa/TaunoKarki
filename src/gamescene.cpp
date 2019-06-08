#include "gamescene.h"
#include "menuscene.h"
#include "game.h"
#include "gameobject.h"

#include "playercontroller.h"
#include "locator.h"

#include <random>
#include <iostream>

#define randomFloat std::uniform_real_distribution<float>

GameScene::GameScene(game_state_t* state, int level, Weapon* weapon) : 
	Scene(state),
	world(b2Vec2(0.0f, 0.0f)),
	gameObjectManager(state->assets, camera, &world),
	collisionHandler(),
	level(level),
	gameEnding(false),
	playerDyingChannel(0)
{
	std::random_device randomDevice;
	std::default_random_engine randomGenerator(randomDevice());

	Locator::getAudio()->playSound(Locator::getAssetManager()->ambienceSound, 0, -1);

	std::cout << "GAMESCENE ALIVE - entering level " << level << std::endl;

	world.SetContactListener(&collisionHandler);

	camera.createNewPerspectiveMatrix(60.0f, (float)state->screen_width, (float)state->screen_height, 0.1f, 100.0f);
	camera.createNewOrthographicMatrix((float)state->screen_width, (float)state->screen_height);
	camera.setPosition(glm::vec3(0.0f, 0.0f, 20.0f));
	camera.setOffset(0.0f, -7.5f, 0.0f);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	while (true)
	{
		tilemap = new Tilemap(glm::vec3(0.0f), state->assets, camera, world);
		tilemap->generate(7 + level * 4, 7 + level * 4);

		if (tilemap->getNumberOfStartingPositions() > 1) break;
		else delete tilemap;
	}

	gameObjectManager.createPlayer(tilemap->getStartingPosition(), weapon);
	
	while (tilemap->getNumberOfStartingPositions() > 0)
	{
		gameObjectManager.createEnemy(tilemap->getStartingPosition(), level, tilemap);
	}

	gameObjectManager.createPlayerAmmoBar(glm::vec3(10.0f, -7.5f, 5.0f), glm::vec3(9.0f, 0.5f, 0.5f));
	gameObjectManager.createPlayerHealthBar(glm::vec3(-10.0f, -7.5f, 5.0f), glm::vec3(9.0f, 0.5f, 0.5f));
}

GameScene::~GameScene()
{
	std::cout << "GAMESCENE DIE - leaving level " << level << std::endl;
}

void GameScene::update(float deltaTime)
{
	accumulator += deltaTime;

	while (accumulator >= state->step)
	{
		world.Step(state->step, 8, 3);
		accumulator -= state->step;
		gameObjectManager.update();
	}

	gameObjectManager.interpolate(accumulator / state->step);

	if (!gameEnding && gameObjectManager.getNumberOfObjectsOfType(GAMEOBJECT_TYPES::PLAYER) == 0)
	{
		std::cout << "PLAYER LOST - died on level " << level << std::endl;
		playerDyingChannel = Locator::getAudio()->playSound(Locator::getAssetManager()->playerDeadSound);
		gameEnding = true;
	}
	else if (gameEnding)
	{
		if (!Mix_Playing(playerDyingChannel))
		{
			Mix_HaltChannel(-1);
			state->scenes.change(new MenuScene(state));
		}
	}
	else if (gameObjectManager.getNumberOfObjectsOfType(GAMEOBJECT_TYPES::ENEMY) == 0)
	{
		Weapon* weapon = gameObjectManager.getFirstObjectOfType(GAMEOBJECT_TYPES::PLAYER)->getComponent<PlayerController>()->getWeapon()->getCopy();
		
		std::cout << "PLAYER WON - cleared level " << level << std::endl;
		state->scenes.change(new GameScene(state, level + 1, weapon));
	}
}

void GameScene::draw()
{
	tilemap->draw();
	gameObjectManager.draw();
}

void GameScene::handleEvent(SDL_Event& event)
{
	if (event.type == SDL_KEYDOWN)
	{
		if (event.key.keysym.sym == SDLK_ESCAPE)
		{
			Mix_HaltChannel(-1);
			state->scenes.change(new MenuScene(state));
		}
	}
}