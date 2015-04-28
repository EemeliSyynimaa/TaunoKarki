#include "audio.h"

GameAudio::GameAudio()
{
}

GameAudio::~GameAudio()
{
}

void GameAudio::playSound(Mix_Chunk* sound, int channel, int loops)
{
	Mix_PlayChannel(channel, sound, loops);
}

void GameAudio::stopSound(Mix_Chunk* sound)
{
}

void GameAudio::stopAllSounds()
{
}