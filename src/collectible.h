#ifndef COLLECTIBLE_H
#define COLLECTIBLE_H

#include "component.h"
#include "transform.h"
#include "assetmanager.h"

class Collectible : public Component
{
public:
	Collectible(GameObject* owner);
	~Collectible();

	void update();
	void setType(COLLECTIBLES type) { this->type = type; }
	COLLECTIBLES getType() { return type; }
private:
	Transform* transform;
	COLLECTIBLES type;
};

#endif