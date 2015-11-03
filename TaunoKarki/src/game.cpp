#include <cassert>
#include <iostream>
#include <algorithm>
#include "game.h"
#include "menuscene.h"
#include "locator.h"


Game::Game() : screenWidth(1280), screenHeight(720), running(true), step(1.0f / 60.0f)
{
	int SDLResult = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	assert(SDLResult == 0);

	int mixResult = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024);
	assert(mixResult == 0);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	window = SDL_CreateWindow("Tauno Kaerki", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screenWidth, screenHeight, SDL_WINDOW_OPENGL);

	assert(window != nullptr);

	context = SDL_GL_CreateContext(window);

	assert(context != nullptr);

	glewExperimental = GL_TRUE;
	const GLenum glewResult = glewInit();
	assert(glewResult == GLEW_OK);
	glGetError();

	int versionMajor = 0;
	int versionMinor = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
	glGetIntegerv(GL_MINOR_VERSION, &versionMinor);
	std::cout << "OpenGL context version: " << versionMajor << "." << versionMinor << std::endl;

	int parameter;
	glGetIntegerv(GL_SAMPLES, &parameter);
	std::cout << "Multisampling samples: " << parameter << '\n';
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	Mix_Init(MIX_INIT_MOD | MIX_INIT_MP3);
	Mix_Volume(-1, MIX_MAX_VOLUME / 2);
	Mix_AllocateChannels(128);

	SDL_SetRelativeMouseMode(SDL_TRUE);

	Locator::init();
	Locator::provideAudio(&gameAudio);
	Locator::provideAssetManager(&assetManager);

	assetManager.loadAssets();
	sceneManager.change(new MenuScene(*this));
}

Game::~Game()
{
	Mix_CloseAudio();
	Mix_Quit();

	glDeleteVertexArrays(1, &VAO);

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Game::run()
{
	float deltaTime = 0.0f;
	float newTime = 0.0f;
	float currentTime = SDL_GetTicks() / 1000.0f;
	SDL_Event event;

	while (running)
	{
		newTime = SDL_GetTicks() / 1000.0f;
		deltaTime = std::min(newTime - currentTime, 0.25f);
		currentTime = newTime;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		while (SDL_PollEvent(&event) == 1)
		{
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
			sceneManager.handleEvent(event);
		}

		update(deltaTime);
		draw();
	}
}

void Game::update(float deltaTime)
{
	sceneManager.update(deltaTime);
}

void Game::draw()
{
	sceneManager.draw();
	SDL_GL_SwapWindow(window);
}