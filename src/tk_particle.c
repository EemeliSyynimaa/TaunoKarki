struct particle_emitter* particle_emitter_create(struct particle_system* system,
    struct particle_emitter_config* config, b32 active)
{
    struct particle_emitter* result = NULL;

    // Todo: assert that config has good values
    // Todo: clean

    if (system->next_permanent_emitter < MAX_PARTICLE_EMITTERS)
    {
        if (config->permanent)
        {
            result = &system->emitters[system->next_permanent_emitter++];

            system->next_temporary_emitter = MAX(system->next_temporary_emitter,
                system->next_permanent_emitter);
        }
        else
        {
            result = &system->emitters[system->next_temporary_emitter++];

            if (system->next_temporary_emitter >= MAX_PARTICLE_EMITTERS)
            {
                system->next_temporary_emitter = system->next_permanent_emitter;
            }
        }

        if (result)
        {
            struct particle_data* particles = NULL;

            if (config->permanent)
            {
                if (system->next_permanent_particle +
                    config->max_particles < MAX_PARTICLES)
                {
                    particles =
                        &system->particles[system->next_permanent_particle];
                    system->next_permanent_particle += config->max_particles;
                    system->next_temporary_particle =
                        MAX(system->next_temporary_particle,
                            system->next_permanent_particle);
                }
            }
            else
            {
                if (system->next_temporary_particle +
                    config->max_particles < MAX_PARTICLES)
                {
                    particles =
                        &system->particles[system->next_temporary_particle];
                    system->next_temporary_particle =
                        system->next_temporary_particle + config->max_particles;
                }
                else if (system->next_permanent_particle +
                    config->max_particles < MAX_PARTICLES)
                {
                    particles =
                        &system->particles[system->next_permanent_particle];
                    system->next_temporary_particle =
                        system->next_permanent_particle + config->max_particles;
                }
            }

            if (particles)
            {
                result->config = *config;
                result->particles = particles;
                result->max_particles = config->max_particles;
                result->active = active;
                result->age = 0.0f;
            }
            else
            {
                result = NULL;
            }
        }
    }

    return result;
}

f32 f32_get_value_between(f32 min, f32 max, f32 t)
{
    return min + (max - min) * t;
}

struct v3 v3_get_value_between(struct v3 min, struct v3 max, f32 t)
{
    struct v3 result = min;

    result.x += (max.x - min.x) * t;
    result.y += (max.y - min.y) * t;
    result.z += (max.z - min.z) * t;

    return result;
}

struct v4 v4_get_value_between(struct v4 min, struct v4 max, f32 t)
{
    struct v4 result = min;

    result.x += (max.x - min.x) * t;
    result.y += (max.y - min.y) * t;
    result.z += (max.z - min.z) * t;
    result.w += (max.w - min.w) * t;

    return result;
}

struct v2 particle_spawn_point_get(struct particle_emitter_config* config)
{
    struct v2 result = v2_zero;

    switch (config->type)
    {
    case PARTICLE_EMITTER_CIRCLE:
    {
        f32 angle = f32_random(config->direction_min, config->direction_max);
        struct v2 direction = v2_direction_from_angle(angle);
        f32 radius = f32_random(config->spawn_radius_min,
            config->spawn_radius_max);

        result.x = config->position.x + direction.x * radius;
        result.y = config->position.y + direction.y * radius;
    } break;
    case PARTICLE_EMITTER_POINT:
    default:
    {
        result = config->position.xy;
    } break;
    }

    return result;
}

void particle_emitter_update(struct particle_emitter* emitter, f32 dt)
{
    for (u32 i = 0; i < emitter->max_particles; i++)
    {
        struct particle_data* particle = &emitter->particles[i];

        if (particle->time > 0)
        {
            f32 step = dt;

            if ((particle->time -= dt) < 0)
            {
                step += particle->time;
                particle->time = 0.0f;
                emitter->count_particles--;
            }

            f32 t = (particle->time_start - particle->time) /
                particle->time_start;

            struct particle_emitter_config* config = &emitter->config;

            particle->position.x += particle->velocity.x * step;
            particle->position.y += particle->velocity.y * step;
            particle->scale = f32_get_value_between(config->scale_start,
                config->scale_end, t);
            particle->color.xyz = v3_get_value_between(
                config->color_start.xyz, config->color_end.xyz, t);
            particle->rotation += particle->velocity_angular * step;
            particle->opacity = f32_get_value_between(config->opacity_start,
                config->opacity_end, t);
        }
    }
}

void particle_emitter_spawn(struct particle_emitter* emitter, f32 dt)
{
    emitter->spawn_timer += dt;

    while (emitter->spawn_timer > emitter->config.rate)
    {
        emitter->spawn_timer -= emitter->config.rate;

        struct particle_data* particle =
            &emitter->particles[emitter->next_free];

        if (++emitter->next_free >= emitter->max_particles)
        {
            emitter->next_free = 0;
        }

        struct particle_emitter_config* config = &emitter->config;

        particle->position.xy = particle_spawn_point_get(config);
        particle->position.z = config->position.z + f32_random(-0.01f, 0.01f);

        struct v3 direction = { 0.0f };

        if (config->type == PARTICLE_EMITTER_CIRCLE &&
            config->move_away_from_spawn)
        {
            direction.xy = v2_direction(config->position.xy,
                particle->position.xy);
        }
        else
        {
            direction.xy = v2_direction_from_angle(f32_random(
                config->direction_min, config->direction_max));
        }

        particle->velocity = v3_mul_f32(direction,
            f32_random(config->velocity_min, config->velocity_max));
        particle->velocity_angular = f32_random(
            config->velocity_angular_min, config->velocity_angular_max);
        particle->scale = config->scale_start;
        particle->color = config->color_start;
        particle->time_start = f32_random(config->time_min,
            config->time_max);
        particle->time = particle->time_start;
        particle->texture = config->texture;
        particle->opacity = config->opacity_start;

        emitter->count_particles++;
    }
}

void particle_emitter_render(struct particle_emitter* emitter,
    struct particle_renderer* renderer)
{
    for (u32 i = 0; i < emitter->max_particles; i++)
    {
        u32 particle_index = (emitter->next_free + i) % emitter->max_particles;

        struct particle_data* particle = &emitter->particles[particle_index];

        if (particle->time > 0)
        {
            struct m4 transform = m4_translate(
                particle->position.x, particle->position.y,
                particle->position.z);
            struct m4 rotation = m4_rotate_z(particle->rotation);
            struct m4 scale = m4_scale_all(particle->scale);
            struct m4 model = m4_mul_m4(scale, rotation);
            model = m4_mul_m4(model, transform);

            struct particle_render_data* data =
                &renderer->data[renderer->num_particles++];

            data->model = model;
            data->color = (struct v4){ particle->color.xyz, particle->opacity };
            data->texture = particle->texture;
        }
    }
}

void particle_system_update(struct particle_system* system, f32 dt)
{
    for (u32 i = 0; i < MAX_PARTICLE_EMITTERS; i++)
    {
        struct particle_emitter* emitter = &system->emitters[i];

        if (emitter->count_particles)
        {
            particle_emitter_update(emitter, dt);
        }

        if (emitter->active)
        {
            particle_emitter_spawn(emitter, dt);

            if (emitter->config.lifetime != INDEFINITE &&
                (emitter->age += dt) >= emitter->config.lifetime)
            {
                emitter->active = false;
            }
        }
    }
}

void particle_system_render(struct particle_system* system,
    struct particle_renderer* renderer)
{
    for (u32 i = 0; i < MAX_PARTICLE_EMITTERS; i++)
    {
        struct particle_emitter* emitter = &system->emitters[i];

        if (emitter->count_particles)
        {
            particle_emitter_render(emitter, renderer);
        }
    }
}

void particle_renderer_init(struct particle_renderer* renderer, u32 shader,
    u32 texture)
{
    renderer->shader = shader;
    renderer->texture = texture;

    struct particle_vertex_data vertices[] =
    {
        // Top right
        {
            { 1.0f, 1.0f },
            { 0.0f, 0.0f }
        },
        // Top left
        {
            { -1.0f, 1.0f },
            { 1.0f, 0.0f }
        },
        // Bottom left
        {
            { -1.0f, -1.0f },
            { 1.0f, 1.0f }
        },
        // Bottom right
        {
            { 1.0f, -1.0f },
            { 0.0f, 1.0f }
        }
    };

    u32 indices[] =
    {
        0, 1, 2, 0, 2, 3
    };

    renderer->num_indices = 6;

    api.gl.glGenVertexArrays(1, &renderer->vao);
    api.gl.glBindVertexArray(renderer->vao);

    api.gl.glGenBuffers(1, &renderer->vbo_vertices);
    api.gl.glGenBuffers(1, &renderer->vbo_particles);
    api.gl.glGenBuffers(1, &renderer->ibo);

    api.gl.glEnableVertexAttribArray(0);
    api.gl.glEnableVertexAttribArray(1);
    api.gl.glEnableVertexAttribArray(2);
    api.gl.glEnableVertexAttribArray(3);
    api.gl.glEnableVertexAttribArray(4);
    api.gl.glEnableVertexAttribArray(5);
    api.gl.glEnableVertexAttribArray(6);
    api.gl.glEnableVertexAttribArray(7);

    api.gl.glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_vertices);
    api.gl.glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
        GL_STATIC_DRAW);
    api.gl.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
        sizeof(struct particle_vertex_data), (void*)0);
    api.gl.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        sizeof(struct particle_vertex_data), (void*)sizeof(struct v2));

    api.gl.glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_particles);
    api.gl.glBufferData(GL_ARRAY_BUFFER, sizeof(renderer->data),
        renderer->data, GL_DYNAMIC_DRAW);
    api.gl.glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct particle_render_data), (void*)(sizeof(struct v4) * 0));
    api.gl.glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct particle_render_data), (void*)(sizeof(struct v4) * 1));
    api.gl.glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct particle_render_data), (void*)(sizeof(struct v4) * 2));
    api.gl.glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct particle_render_data), (void*)(sizeof(struct v4) * 3));
    api.gl.glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE,
        sizeof(struct particle_render_data), (void*)(sizeof(struct v4) * 4));
    api.gl.glVertexAttribIPointer(7, 1, GL_UNSIGNED_INT,
        sizeof(struct particle_render_data), (void*)(sizeof(struct v4) * 5));

    api.gl.glVertexAttribDivisor(2, 1);
    api.gl.glVertexAttribDivisor(3, 1);
    api.gl.glVertexAttribDivisor(4, 1);
    api.gl.glVertexAttribDivisor(5, 1);
    api.gl.glVertexAttribDivisor(6, 1);
    api.gl.glVertexAttribDivisor(7, 1);

    api.gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
    api.gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        renderer->num_indices * sizeof(u32), indices, GL_STATIC_DRAW);

    api.gl.glBindVertexArray(0);

    renderer->initialized = !gl_check_error("particle_renderer_init");
}

void particle_renderer_sort(struct particle_renderer* renderer)
{
    if (renderer->initialized && renderer->num_particles)
    {
        for (u32 i = 0; i < renderer->num_particles - 1; i++)
        {
            for (u32 j = i + 1; j < renderer->num_particles; j++)
            {
                if (renderer->data[i].model.m[3][2] >
                    renderer->data[j].model.m[3][2])
                {
                    struct particle_render_data data = renderer->data[i];
                    renderer->data[i] = renderer->data[j];
                    renderer->data[j] = data;
                }
            }
        }
    }
}

void particle_renderer_flush(struct particle_renderer* renderer,
    struct m4* view, struct m4* projection)
{
    if (renderer->initialized && renderer->num_particles)
    {
        // LOG("Rendering %u particles\n", renderer->num_particles);
        api.gl.glBindVertexArray(renderer->vao);
        api.gl.glUseProgram(renderer->shader);

        u32 uniform_texture = api.gl.glGetUniformLocation(renderer->shader,
            "uniform_texture");
        u32 uniform_vp = api.gl.glGetUniformLocation(renderer->shader,
            "uniform_vp");

        struct m4 vp = m4_mul_m4(*view, *projection);

        api.gl.glUniform1i(uniform_texture, 0);
        api.gl.glUniformMatrix4fv(uniform_vp, 1, GL_FALSE, (GLfloat*)&vp);

        api.gl.glActiveTexture(GL_TEXTURE0);
        api.gl.glBindTexture(GL_TEXTURE_2D_ARRAY, renderer->texture);

        api.gl.glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_particles);
        api.gl.glBufferSubData(GL_ARRAY_BUFFER, 0,
            renderer->num_particles * sizeof(struct particle_render_data),
            renderer->data);

        api.gl.glDrawElementsInstanced(GL_TRIANGLES, renderer->num_indices,
            GL_UNSIGNED_INT, NULL, renderer->num_particles);

        api.gl.glUseProgram(0);
        api.gl.glBindVertexArray(0);

        renderer->num_particles = 0;
    }
}
