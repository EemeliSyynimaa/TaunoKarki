#include "collider.h"

Collider::Collider(GameObject* owner, uint16_t categoryBits,
    uint16_t maskBits) :
    Component(owner)
{
    fixtureDef.filter.maskBits = maskBits;
    fixtureDef.filter.categoryBits = categoryBits;
}

void Collider::update(tk_state_player_input_t* input)
{
    (void)input;
}

b2FixtureDef& Collider::getFixtureDef()
{
    return fixtureDef;
}