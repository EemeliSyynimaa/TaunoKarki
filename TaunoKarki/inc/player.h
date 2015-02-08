#ifndef PLAYER_H
#define PLAYER_H

#include <GL\glew.h>
#include "component.h"

class Player : public Component
{
public:
	Player();
	~Player();

	void update();
private:
	GLuint IBO = 0;
	GLuint VBO = 0;
	GLuint program;
};

#endif