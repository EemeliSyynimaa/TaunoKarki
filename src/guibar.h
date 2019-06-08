#ifndef GUIBAR_H
#define GUIBAR_H

#include "component.h"
#include "glm/glm.hpp"
#include "transform.h"
#include "gameobject.h"

class GuiBar : public Component
{
public:
    GuiBar(GameObject* owner) : Component(owner), transform(owner->getComponent<Transform>())
    {
        assert(transform);
    }
    ~GuiBar() {}

    void setOffsetPosition(glm::vec3 pos) { offsetPosition = pos; }
    virtual void update() = 0;
protected:
    Transform* transform;
    glm::vec3 offsetPosition;
};

#endif