#ifndef MENUBUTTON_H
#define MENUBUTTON_H

#include "component.h"

class MenuButton : public Component
{
public:
    MenuButton(GameObject* owner, int id);
    ~MenuButton();

    void update(tk_state_player_input_t* input);
    bool selected() { return id == state; }
    static bool exitGame;
    static bool startGame;
private:
    int id;
    static int state;
    static int maxStates;
    static bool arrowPressed;
};

#endif