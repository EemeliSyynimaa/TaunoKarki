#ifndef SHOTGUN_H
#define SHOTGUN_H

#include "weapon.h"

class Shotgun : public Weapon
{
public:
    Shotgun(GameObjectManager& gameObjectManager);
    ~Shotgun();

    void update(f32 delta_time);
    Weapon* getCopy() { return new Shotgun(*this); }
private:
    f32 lastShot;
    bool fired;
    size_t numberOfShells;
};

#endif