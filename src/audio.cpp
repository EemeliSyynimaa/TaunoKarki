#include "audio.h"

GameAudio::GameAudio()
{
}

GameAudio::~GameAudio()
{
}

int GameAudio::playSound(Mix_Chunk* sound, int channel, int loops)
{
	return Mix_PlayChannel(channel, sound, loops);
}

void GameAudio::stopSound(Mix_Chunk* sound)
{
}

void GameAudio::stopAllSounds()
{
}