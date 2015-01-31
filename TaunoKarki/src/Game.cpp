#include <Game.h>
#include <iostream>

Game::Game()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
	}
}

Game::~Game()
{
	SDL_Quit();
}

void Game::run()
{
}