#include "locator.h"

AssetManager* Locator::assetManager;

void Locator::init()
{
    assetManager = nullptr;
}

AssetManager* Locator::getAssetManager()
{
    return assetManager;
}

void Locator::provideAssetManager(AssetManager* service)
{
    assert(service);
    assetManager = service;
}
