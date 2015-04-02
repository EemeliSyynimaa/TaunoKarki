#ifndef MACHINEGUN_H
#define MACHINEGUN_H

#include "weapon.h"

class MachineGun : public Weapon
{
public:
	MachineGun(GameObjectManager& gameObjectManager);
	~MachineGun();

	void update();

private:
	float fireRate;
	Uint32 lastShot;
};

#endif