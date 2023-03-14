struct state_physics_data
{
    struct game_state* base;
    struct circle circles[MAX_CIRCLES];
    struct contact contacts[MAX_CONTACTS];
    struct line_segment lines[MAX_COLLISION_SEGMENTS];
    u32 num_circles;
    u32 num_lines;
};

void state_physics_init(struct state_physics_data* data)
{
    data->num_circles = 16;

    f32 area_size = 5.0f;

    // Make the first circle controllable
    data->circles[0].position.x = 0.0f;
    data->circles[0].position.y = 0.0f;
    data->circles[0].radius = 0.25f;
    data->circles[0].mass = 10.0f;

    for (u32 i = 1; i < data->num_circles; i++)
    {
        struct circle* circle = &data->circles[i];
        circle->position.x = f32_random(-area_size, area_size);
        circle->position.y = f32_random(-area_size, area_size);
        circle->radius = 0.25f;
        circle->target.x = f32_random(-area_size, area_size);
        circle->target.y = f32_random(-area_size, area_size);
        circle->mass = 1.0f;
    }

    f32 size = 7.5f;
    data->base->camera.position = (struct v3){ 0.0f, 0.0f, 5.0f };
    data->base->camera.projection = m4_orthographic(-size, size, -size, size,
            0.1f, 100.0f);
    data->base->camera.projection_inverse =
        m4_inverse(data->base->camera.projection);

    struct v3 up = { 0.0f, 1.0f, 0.0f };

    data->base->camera.view = m4_look_at(data->base->camera.position,
        (struct v3) { data->base->camera.position.xy, 0.0f }, up);
    data->base->camera.view_inverse = m4_inverse(data->base->camera.view);

    api.gl.glClearColor(0.25f, 0.0f, 0.0f, 0.0f);
}

void state_physics_update(struct state_physics_data* data,
    struct game_input* input, f32 step)
{
    static f32 particle_test = 0;
    static u32 frame_number = 0;

    LOG("Frame: %i\n", frame_number++);

    circles_velocities_update(data->circles, data->num_circles, input, step);

    f32 max_iterations = 10;

    for (u32 i = 0; i < max_iterations; i++)
    {
        LOG("Checking collisions, iteration %d\n", i + 1);

        // Todo: sometimes, for some reasons, a collision
        // between two objects happens again in the following
        // iteration. This shouldn't occur as the collision
        // is already resolved and the velocities and positions
        // are updated! Investigate

        // Todo: sometimes collisions are handled poorly
        // when multiple circles are touching...?

        u32 num_contacts = circles_collisions_check(data->circles,
            data->num_circles, data->contacts, data->lines, data->num_lines);

        if (num_contacts)
        {
            circles_collisions_resolve(data->contacts, num_contacts, step);
        }
        else
        {
            LOG("No contacts!\n");
            break;
        }
    }

    circles_positions_update(data->circles, data->num_circles);
}

void state_physics_render(struct state_physics_data* data)
{
    circles_render(data->circles, data->num_circles, data->base);
    // collision_map_render(state);
}

struct state_interface state_physics_create(struct game_state* state)
{
    struct state_interface result = { 0 };
    result.init = state_physics_init;
    result.update = state_physics_update;
    result.render = state_physics_render;
    result.data = stack_alloc(&state->stack, sizeof(struct state_physics_data));

    memory_set(result.data, sizeof(struct state_physics_data), 0);

    struct game_state** base = (struct game_state**)result.data;

    *base = state;

    return result;
}
