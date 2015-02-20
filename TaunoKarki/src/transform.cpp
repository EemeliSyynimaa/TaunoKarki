#include "transform.h"

Transform::Transform(GameObject* owner) : Component(owner), position(1.0f), scale(1.0f), rotation(1.0f)
{
}

Transform::~Transform()
{
}

void Transform::update()
{
}
