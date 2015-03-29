#ifndef PISTOL_H
#define PISTOL_H

#include "weapon.h"

class Pistol : public Weapon
{
public:
	Pistol(GameObjectManager& gameObjectManager);
	~Pistol();

	void update();
private:
	bool fired;
};

#endif