#ifndef STATICBODY_H
#define STATICBODY_H

#include "component.h"
#include "transform.h"
#include "collider.h"

class StaticBody : public Component
{
public:
    StaticBody(GameObject* owner);
    ~StaticBody();

    void update(game_input* input);
private:
    Transform* transform;
    Collider* collider;
};

#endif