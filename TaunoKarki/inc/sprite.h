#ifndef SPRITE_H
#define SPRITE_H

#include <iostream>
#include "component.h"
#include "texture.h"

class Sprite : public Component
{
public:
	Sprite();
	Sprite(Texture* texture);
	~Sprite();
	void reset();
	void update();
private:
	Texture* texture;
};

#endif