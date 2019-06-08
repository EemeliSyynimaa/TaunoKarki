#include "component.h"
#include "gameobject.h"

Component::Component(GameObject* owner) : owner(owner)
{
    assert(owner);
}

Component::~Component()
{
}