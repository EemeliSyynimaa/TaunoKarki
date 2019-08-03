#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include "component.h"
#include "transform.h"
#include "weapon.h"

class PlayerController : public Component
{
public:
    PlayerController(GameObject* owner);
    ~PlayerController();

    void update(game_input* input);
    void giveWeapon(Weapon* weapon, bool instantReload = false);
    Weapon* getWeapon() { return weapon; }
    void handleItem(COLLECTIBLES item);

private:
    Transform* transform;
    Weapon* weapon;
    float moveSpeed;
};

#endif