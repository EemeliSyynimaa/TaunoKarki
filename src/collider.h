#ifndef COLLIDER_H
#define COLLIDER_H

#include "component.h"

class Collider : public Component
{
public:
    Collider(GameObject* owner, u16 categoryBits, u16 maskBits);

    void update(game_input* input);
    // b2FixtureDef& getFixtureDef();

protected:
    // b2FixtureDef fixtureDef;
};

#endif
