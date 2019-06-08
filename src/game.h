#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include "SDL\SDL.h"
#include "GL\glew.h"
#include "SDL\SDL_mixer.h"
#include "scenemanager.h"
#include "assetmanager.h"
#include "audio.h"

typedef struct game_state_t
{
	SDL_Window* window;
	SDL_GLContext context;
	GLuint VAO;
	int32_t screen_width;
	int32_t screen_height;
	int32_t running;
	float step;
	SceneManager scenes;
	AssetManager assets;
	GameAudio audio;

} game_state_t;

#endif