#ifndef GAME_H
#define GAME_H

#include <SDL\SDL.h>
#include <GL\glew.h>

class Game
{
public:
	Game();
	~Game();

	void run();
private:
	SDL_Window *window;
	SDL_GLContext context;

	int screenWidth;
	int screenHeight;

	void update();
	void draw();
};

#endif