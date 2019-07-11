#ifndef HEALTHBAR_H
#define HEALTHBAR_H

#include "guibar.h"

class HealthBar : public GuiBar
{
public:
    HealthBar(GameObject* owner);
    ~HealthBar();

    void update(tk_state_player_input_t* input);
};

#endif