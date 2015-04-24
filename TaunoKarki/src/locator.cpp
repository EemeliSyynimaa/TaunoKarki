#include "locator.h"

Audio* Locator::audio;
NullAudio Locator::null;
AssetManager* Locator::assetManager;

void Locator::init()
{
	audio = &null;
	assetManager = nullptr;
}

AssetManager* Locator::getAssetManager()
{
	return assetManager;
}

Audio* Locator::getAudio()
{
	return audio;
}

void Locator::provideAudio(Audio* service)
{
	if (service) audio = service;
	else audio = &null;
}
void Locator::provideAssetManager(AssetManager* service)
{
	assert(service);
	assetManager = service;
}
