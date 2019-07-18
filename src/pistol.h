#ifndef PISTOL_H
#define PISTOL_H

#include "weapon.h"

class Pistol : public Weapon
{
public:
    Pistol(GameObjectManager& gameObjectManager);
    ~Pistol();

    void update(f32 delta_time);
    Weapon* getCopy() { return new Pistol(*this); }
private:
    bool fired;
};

#endif