#ifndef PISTOL_H
#define PISTOL_H

#include "weapon.h"

class Pistol : public Weapon
{
public:
	Pistol(GameObjectManager& gameObjectManager);
	~Pistol();

	void update();
	Weapon* getCopy() { return new Pistol(*this); }
private:
	bool fired;
};

#endif