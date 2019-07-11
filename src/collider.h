#ifndef COLLIDER_H
#define COLLIDER_H

#include "component.h"
#include "Box2D\Box2D.h"

class Collider : public Component
{
public:
    Collider(GameObject* owner, uint16_t categoryBits,
        uint16_t maskBits);

    void update(tk_state_player_input_t* input);
    b2FixtureDef& getFixtureDef();

protected:
    b2FixtureDef fixtureDef;
};

#endif
