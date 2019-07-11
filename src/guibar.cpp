#include "guibar.h"

GuiBar::GuiBar(GameObject* owner) :
    Component(owner),
    transform(owner->getComponent<Transform>())
{
    assert(transform);
}

void GuiBar::setOffsetPosition(glm::vec3 pos)
{
    offsetPosition = pos;
}