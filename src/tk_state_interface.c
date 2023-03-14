typedef void type_state_init(struct game_state*);
typedef void type_state_update(struct game_state*, struct game_input*, f32);
typedef void type_state_render(struct game_state*);

struct state_interface
{
    type_state_init* init;
    type_state_update* update;
    type_state_render* render;
};
