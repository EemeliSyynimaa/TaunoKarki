#ifndef PLAYER_H
#define PLAYER_H

#include <GL\glew.h>
#include "component.h"

class Player : public Component
{
public:
	Player(GameObject* owner);
	~Player();

	void update();
private:
};

#endif