#ifndef MENUBUTTON_H
#define MENUBUTTON_H

#include "SDL\SDL.h"
#include "component.h"

class MenuButton : public Component
{
public:
	MenuButton(GameObject* owner, int id);
	~MenuButton();

	void update();
	static bool exitGame;
	static bool startGame;
private:
	int id;
	static int state;
	static int maxStates;
	static bool arrowPressed;
	const Uint8* keyboardState = SDL_GetKeyboardState(NULL);
};

#endif