#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include "component.h"
#include "SDL\SDL.h"
#include "Box2D\Box2D.h"
#include "transform.h"

class PlayerController : public Component
{
public:
	PlayerController(GameObject* owner);
	~PlayerController();

	void update(float deltaTime);
private:
	const Uint8* keyboardState = SDL_GetKeyboardState(NULL);
	b2Body* body;
	Transform* transform;
	float speed;
};

#endif