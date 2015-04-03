#ifndef LOCATOR_H
#define LOCATOR_H

#include "audio.h"

class Locator
{
public:
	static void init() { audio = &null; }
	static Audio* getAudio() { return audio; }
	static void provideAudio(Audio* service) 
	{ 
		if (service) audio = service;
		else audio = &null; 
	}
private:
	static Audio* audio;
	static NullAudio null;
};

#endif