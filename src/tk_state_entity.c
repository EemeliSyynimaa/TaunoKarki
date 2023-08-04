struct state_entity_data
{
    int status;
};


void state_entity_init(void* data)
{
    struct state_entity_data* state = (struct state_entity_data*)data;
}


void state_entity_update(void* data, struct game_input* input, f32 step)
{
    struct state_entity_data* state = (struct state_entity_data*)data;
}

void state_entity_render(void* data)
{
    struct state_entity_data* state = (struct state_entity_data*)data;
}

struct state_interface state_entity_create(struct game_state* state)
{
    struct state_interface result = { 0 };
    result.init = state_entity_init;
    result.update = state_entity_update;
    result.render = state_entity_render;
    result.data = stack_alloc(&state->stack_permanent,
        sizeof(struct state_entity_data));

    memory_set(result.data, sizeof(struct state_entity_data), 0);

    struct game_state** base = (struct game_state**)result.data;

    *base = state;

    return result;
}
