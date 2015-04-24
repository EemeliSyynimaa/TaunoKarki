#ifndef LOCATOR_H
#define LOCATOR_H

#include "audio.h"
#include "assetmanager.h"

class Locator
{
public:
	static void init();
	static Audio* getAudio();
	static AssetManager* getAssetManager();
	static void provideAudio(Audio* service);
	static void provideAssetManager(AssetManager* service);
private:
	static Audio* audio;
	static NullAudio null;
	static AssetManager* assetManager;
};

#endif