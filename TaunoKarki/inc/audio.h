#ifndef AUDIO_H
#define AUDIO_H

#include "SDL/SDL_mixer.h"

class Audio
{
public:
	virtual ~Audio() {}
	virtual void playSound(int soundID) = 0;
	virtual void stopSound(int soundID) = 0;
	virtual void stopAllSounds() = 0;
};

class NullAudio : public Audio
{
public:
	void playSound(int soundID) {}
	void stopSound(int soundID) {}
	void stopAllSounds() {}
};

class GameAudio : public Audio
{
public:
	GameAudio();
	~GameAudio();

	void playSound(int soundID);
	void stopSound(int soundID);
	void stopAllSounds();
private:
};

#endif