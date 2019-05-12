#ifndef GAME_H
#define GAME_H

#include "SDL\SDL.h"
#include "GL\glew.h"
#include "SDL\SDL_mixer.h"
#include "scenemanager.h"
#include "assetmanager.h"
#include "audio.h"

class Game
{
public:
	Game();
	~Game();

	SceneManager& getSceneManager() { return sceneManager; }
	AssetManager& getAssetManager() { return assetManager; }
	int getScreenWidth() { return screenWidth; }
	int getScreenHeight() { return screenHeight; }
	float getStep() { return step; }
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
	AssetManager assetManager;
	GameAudio gameAudio;

	void update(float deltaTime);
	void draw();

	float step;
};

#endif