#ifndef DAMAGE_H
#define DAMAGE_H

#include "component.h"

class Damage : public Component
{
public:
	Damage(GameObject* gameObject, float damage = 0) : Component(gameObject), damage(damage) {};
	~Damage() {};

	void update( ) {};
	void setDamage(float damage) { this->damage = damage; }
	float getDamage() { return damage; }
private:
	float damage;
};

#endif