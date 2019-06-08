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

void MenuButton::update()
{
    if ((keyboardState[SDL_SCANCODE_UP] || keyboardState[SDL_SCANCODE_W]) && !arrowPressed)
    {
        if (--MenuButton::state < 0)
            MenuButton::state = MenuButton::maxStates - 1;

        arrowPressed = true;
    }
    else if ((keyboardState[SDL_SCANCODE_DOWN] || keyboardState[SDL_SCANCODE_S]) && !arrowPressed)
    {
        if (++MenuButton::state >= MenuButton::maxStates)
            MenuButton::state = 0;

        arrowPressed = true;
    }
    else if (!keyboardState[SDL_SCANCODE_UP] && !keyboardState[SDL_SCANCODE_DOWN] && !keyboardState[SDL_SCANCODE_S] && !keyboardState[SDL_SCANCODE_W]) arrowPressed = false;

    if (MenuButton::state == id)
    {
        owner->getDrawableComponent<MeshRenderer>()->setTexture(owner->gameObjectManager.getAssetManager().enemyTexture);
        owner->getComponent<Transform>()->setScale(glm::vec3(0.75f));

        if (keyboardState[SDL_SCANCODE_SPACE] || keyboardState[SDL_SCANCODE_RETURN])
            if (id == 0) MenuButton::startGame = true;
            else MenuButton::exitGame = true;
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