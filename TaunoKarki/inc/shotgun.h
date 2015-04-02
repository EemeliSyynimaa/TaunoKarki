#ifndef SHOTGUN_H
#define SHOTGUN_H

#include "weapon.h"

class Shotgun : public Weapon
{
public:
	Shotgun(GameObjectManager& gameObjectManager);
	~Shotgun();

	void update();
private:
	Uint32 lastShot;
	float fireRate;
	bool fired;
	size_t numberOfShells;
};

#endif