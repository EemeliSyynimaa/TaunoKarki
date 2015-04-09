#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include "component.h"
#include "SDL\SDL.h"
#include "Box2D\Box2D.h"
#include "transform.h"
#include "weapon.h"

class PlayerController : public Component
{
public:
	PlayerController(GameObject* owner);
	~PlayerController();

	void update( );
	void giveWeapon(Weapon* weapon) { this->weapon = weapon; weapon->setOwner(this->owner); }
private:
	const Uint8* keyboardState = SDL_GetKeyboardState(NULL);
	b2Body* body;
	Transform* transform;
	Weapon* weapon;
	float moveSpeed;
};

#endif