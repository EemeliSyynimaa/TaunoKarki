#ifndef AMMOBAR_H
#define AMMOBAR_H

#include "guibar.h"

class AmmoBar : public GuiBar
{
public:
    AmmoBar(GameObject* owner);
    ~AmmoBar();

    void update(tk_state_player_input_t*);
};

#endif