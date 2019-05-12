#ifndef AUDIO_H
#define AUDIO_H

#include "SDL\SDL_mixer.h"

class Audio
{
public:
	virtual ~Audio() {}
	virtual int playSound(Mix_Chunk* sound, int channel = -1, int loops = 0) = 0;
	virtual void stopSound(Mix_Chunk* sound) = 0;
	virtual void stopAllSounds() = 0;
};

class NullAudio : public Audio
{
public:
	int playSound(Mix_Chunk* sound, int channel = -1, int loops = 0) { return 0; }
	void stopSound(Mix_Chunk* sound) {}
	void stopAllSounds() {}
};

class GameAudio : public Audio
{
public:
	GameAudio();
	~GameAudio();

	int playSound(Mix_Chunk* sound, int channel = -1, int loops = 0);
	void stopSound(Mix_Chunk* sound);
	void stopAllSounds();
private:
};

#endif