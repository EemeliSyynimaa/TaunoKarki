typedef void type_state_init(void*);
typedef void type_state_update(void*, struct game_input*, f32);
typedef void type_state_render(void*);

struct state_interface
{
    type_state_init* init;
    type_state_update* update;
    type_state_render* render;
    void* data;
};
