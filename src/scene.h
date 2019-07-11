#ifndef SCENE_H
#define SCENE_H

class Scene
{
public:
    Scene(tk_game_state_t* state);
    virtual ~Scene() {};

    virtual void update(float deltaTime, tk_state_player_input_t* input) = 0;
    virtual void draw() = 0;
protected:
    tk_game_state_t* state;

    // Variables for fixed timestep
    float accumulator;
};

#endif
