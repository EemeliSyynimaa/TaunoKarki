#ifndef GUIBAR_H
#define GUIBAR_H

#include "component.h"
#include "glm/glm.hpp"
#include "transform.h"
#include "gameobject.h"

class GuiBar : public Component
{
public:
    GuiBar(GameObject* owner);

    void setOffsetPosition(glm::vec3 pos);
    virtual void update(tk_state_player_input_t*) = 0;
protected:
    Transform* transform;
    glm::vec3 offsetPosition;
};

#endif