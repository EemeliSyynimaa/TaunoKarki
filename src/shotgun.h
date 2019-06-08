#ifndef SHOTGUN_H
#define SHOTGUN_H

#include "weapon.h"

class Shotgun : public Weapon
{
public:
    Shotgun(GameObjectManager& gameObjectManager);
    ~Shotgun();

    void update();
    Weapon* getCopy() { return new Shotgun(*this); }
private:
    Uint32 lastShot;
    bool fired;
    size_t numberOfShells;
};

#endif