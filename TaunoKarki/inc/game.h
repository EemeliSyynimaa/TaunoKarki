#ifndef GAME_H
#define GAME_H

#include <SDL\SDL.h>
#include <GL\glew.h>
#include "scenemanager.h"

class Game
{
public:
	Game();
	~Game();

	SceneManager& getSceneManager() { return sceneManager; }
	int getScreenWidth() { return screenWidth; }
	int getScreenHeight() { return screenHeight; }
	void run();
	void stop() { running = false; }
	GLuint VAO = 0;
private:
	SDL_Window* window;
	SDL_GLContext context;

	int screenWidth;
	int screenHeight;

	bool running;

	SceneManager sceneManager;

	void update(float deltaTime);
	void draw();
};

#endif