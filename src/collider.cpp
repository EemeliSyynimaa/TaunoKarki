#include "collider.h"

Collider::Collider(GameObject* owner, u16 categoryBits, u16 maskBits) :
    Component(owner)
{
    // fixtureDef.filter.maskBits = maskBits;
    // fixtureDef.filter.categoryBits = categoryBits;
}

void Collider::update(game_input* input)
{
    (void)input;
}

// b2FixtureDef& Collider::getFixtureDef()
// {
//     return fixtureDef;
// }