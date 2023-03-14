void state_physics_init(struct game_state* state)
{
    state->num_circles = 16;

    f32 area_size = 5.0f;

    // Make the first circle controllable
    state->circles[0].position.x = 0.0f;
    state->circles[0].position.y = 0.0f;
    state->circles[0].radius = 0.25f;
    state->circles[0].mass = 10.0f;

    for (u32 i = 1; i < state->num_circles; i++)
    {
        struct circle* circle = &state->circles[i];
        circle->position.x = f32_random(-area_size, area_size);
        circle->position.y = f32_random(-area_size, area_size);
        circle->radius = 0.25f;
        circle->target.x = f32_random(-area_size, area_size);
        circle->target.y = f32_random(-area_size, area_size);
        circle->mass = 1.0f;
    }

    f32 size = 7.5f;
    state->camera.screen_width = (f32)state->screen_width;
    state->camera.screen_height = (f32)state->screen_height;
    state->camera.position = (struct v3){ 0.0f, 0.0f, 5.0f };
    state->camera.projection = m4_orthographic(-size, size, -size, size,
            0.1f, 100.0f);
    state->camera.projection_inverse = m4_inverse(state->camera.projection);

    struct v3 up = { 0.0f, 1.0f, 0.0f };

    state->camera.view = m4_look_at(state->camera.position,
        (struct v3) { state->camera.position.xy, 0.0f }, up);
    state->camera.view_inverse = m4_inverse(state->camera.view);

    api.gl.glClearColor(0.25f, 0.0f, 0.0f, 0.0f);
}

void state_physics_update(struct game_state* state, struct game_input* input,
    f32 step)
{
    static f32 particle_test = 0;
    static u32 frame_number = 0;

    LOG("Frame: %i\n", frame_number++);

    circles_velocities_update(state, input, step);

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

        state->num_contacts = 0;

        circles_collisions_check(state);
        circles_collisions_resolve(state, step);

        if (!state->num_contacts)
        {
            LOG("No contacts!\n");
            break;
        }
    }

    circles_positions_update(state);
}

void state_physics_render(struct game_state* state)
{
    circles_render(state);
    collision_map_render(state);
}

struct state_interface state_physics =
{
    .init = state_physics_init,
    .update = state_physics_update,
    .render = state_physics_render
};
