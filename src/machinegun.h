#ifndef MACHINEGUN_H
#define MACHINEGUN_H

#include "weapon.h"

class MachineGun : public Weapon
{
public:
    MachineGun(GameObjectManager& gameObjectManager);
    ~MachineGun();

    void update();
    Weapon* getCopy() { return new MachineGun(*this); }
private:
    Uint32 lastShot;
};

#endif