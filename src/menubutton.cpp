#include "menubutton.h"
#include "gameobject.h"
#include "meshrenderer.h"
#include "transform.h"
#include <iostream>

MenuButton::MenuButton(GameObject* owner, int id) : Component(owner), id(id)
{
    if (id >= MenuButton::maxStates)
        MenuButton::maxStates++;

    exitGame = false;
    startGame = false;

}

MenuButton::~MenuButton()
{
}

void MenuButton::update(tk_state_player_input_t* input)
{
    if (input->menu_up && !arrowPressed)
    {
        if (--MenuButton::state < 0)
            MenuButton::state = MenuButton::maxStates - 1;

        arrowPressed = true;
    }
    else if (input->menu_down && !arrowPressed)
    {
        if (++MenuButton::state >= MenuButton::maxStates)
            MenuButton::state = 0;

        arrowPressed = true;
    }
    else if (!input->menu_down && !input->menu_up)
    {
        arrowPressed = false;
    }

    if (MenuButton::state == id)
    {
        owner->getDrawableComponent<MeshRenderer>()->setTexture(owner->gameObjectManager.getAssetManager().enemyTexture);
        owner->getComponent<Transform>()->setScale(glm::vec3(0.75f));

        if (input->menu_confirm)
        {
            if (id == 0) MenuButton::startGame = true;
            else MenuButton::exitGame = true;
        }
    }
    else
    {
        owner->getDrawableComponent<MeshRenderer>()->setTexture(owner->gameObjectManager.getAssetManager().playerTexture);
        owner->getComponent<Transform>()->setScale(glm::vec3(0.5f));
    }
}

int MenuButton::state = 0;
int MenuButton::maxStates = 1;
bool MenuButton::arrowPressed = false;
bool MenuButton::startGame = false;
bool MenuButton::exitGame = false;