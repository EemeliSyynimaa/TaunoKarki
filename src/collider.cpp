#include "collider.h"

Collider::Collider(GameObject* owner, uint16_t categoryBits,
    uint16_t maskBits) :
    Component(owner)
{
    fixtureDef.filter.maskBits = maskBits;
    fixtureDef.filter.categoryBits = categoryBits;
}

void Collider::update(game_input* input)
{
    (void)input;
}

b2FixtureDef& Collider::getFixtureDef()
{
    return fixtureDef;
}