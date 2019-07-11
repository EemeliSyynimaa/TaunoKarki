#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include "component.h"
#include "Box2D\Box2D.h"
#include "transform.h"
#include "weapon.h"

class PlayerController : public Component
{
public:
    PlayerController(GameObject* owner);
    ~PlayerController();

    void update(tk_state_player_input_t* input);
    void giveWeapon(Weapon* weapon, bool instantReload = false);
    Weapon* getWeapon() { return weapon; }
    void handleItem(COLLECTIBLES item);
    
    int playerAudioChannel;
private:
    b2Body* body;
    Transform* transform;
    Weapon* weapon;
    float moveSpeed;
};

#endif