#ifndef COLLECTIBLE_H
#define COLLECTIBLE_H

#include "component.h"
#include "transform.h"

class Collectible : public Component
{
public:
	Collectible(GameObject* owner);
	~Collectible();

	void update();
	void setType(unsigned int type) { this->type = type; }
	unsigned int getType() { return type; }
private:
	Transform* transform;
	unsigned int type;
};

#endif