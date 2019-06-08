#ifndef SCENE_H
#define SCENE_H

#include "SDL/SDL_events.h"

struct game_state_t;

class Scene
{
public:
    Scene(game_state_t* state);
    virtual ~Scene() {};

    virtual void update(float deltaTime) = 0;
    virtual void draw() = 0;
    virtual void handleEvent(SDL_Event& event) = 0;
protected:
    game_state_t* state;

    // Variables for fixed timestep
    float accumulator;
};

#endif
