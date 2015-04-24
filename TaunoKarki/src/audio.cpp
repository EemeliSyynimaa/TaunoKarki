#include "audio.h"

GameAudio::GameAudio()
{
}

GameAudio::~GameAudio()
{
}

void GameAudio::playSound(Mix_Chunk* sound, int loops)
{
	Mix_PlayChannel(-1, sound, loops);
}

void GameAudio::stopSound(Mix_Chunk* sound)
{
}

void GameAudio::stopAllSounds()
{
}