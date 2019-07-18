#ifndef MACHINEGUN_H
#define MACHINEGUN_H

#include "weapon.h"

class MachineGun : public Weapon
{
public:
    MachineGun(GameObjectManager& gameObjectManager);
    ~MachineGun();

    void update(f32 delta_time);
    Weapon* getCopy() { return new MachineGun(*this); }
private:
    f32 lastShot;
};

#endif