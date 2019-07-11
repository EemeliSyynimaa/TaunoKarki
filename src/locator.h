#ifndef LOCATOR_H
#define LOCATOR_H

#include "assetmanager.h"

class Locator
{
public:
    static void init();
    static AssetManager* getAssetManager();
    static void provideAssetManager(AssetManager* service);
private:
    static AssetManager* assetManager;
};

#endif