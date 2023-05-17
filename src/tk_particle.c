#define MAX_PARTICLES 1024*32
#define MAX_PARTICLE_EMITTERS 128
#define INDEFINITE -1

enum
{
    GFX_CIRCLE_FILLED = 56,
    GFX_CIRCLE = 57,
    GFX_RECT_FILLED = 58,
    GFX_STAR = 59,
    GFX_STAR_FILLED = 60,
};

struct particle_vertex_data
{
    struct v2 position;
    struct v2 uv;
};

struct particle_render_data
{
    struct m4 model;
    struct v4 color;
    u32 texture;
};

enum
{
    PARTICLE_EMITTER_POINT,
    PARTICLE_EMITTER_CIRCLE
};

// Todo: currently support only two variables, min and max
struct particle_emitter_config
{
    struct v4 color_start;
    struct v4 color_end;
    struct v3 position;

    u32 type;
    u32 max_particles;
    f32 rate;
    f32 lifetime;

    b32 permanent;

    f32 velocity_min;
    f32 velocity_max;
    f32 velocity_angular_min;
    f32 velocity_angular_max;
    f32 time_min;
    f32 time_max;
    f32 direction_min;
    f32 direction_max;

    f32 opacity_start;
    f32 opacity_end;
    f32 scale_start;
    f32 scale_end;

    // Todo: only used by circle emitter
    f32 spawn_radius_min;
    f32 spawn_radius_max;
    b32 move_away_from_spawn;

    u8 texture;
};

struct particle_emitter
{
    struct particle_emitter_config config;
    struct particle_data* particles;
    u32 max_particles;
    u32 count_particles;
    u32 next_free;
    f32 spawn_timer;
    f32 age;
    b32 active;
};

struct particle_data
{
    struct v4 color;
    struct v3 position;
    struct v3 velocity;
    f32 velocity_angular;
    f32 scale;
    f32 rotation;
    f32 time_start;
    f32 time;
    f32 opacity;
    u8 texture;
};

struct particle_system
{
    struct particle_data particles[MAX_PARTICLES];
    struct particle_emitter emitters[MAX_PARTICLE_EMITTERS];
    u32 next_permanent_particle;
    u32 next_permanent_emitter;
    u32 next_temporary_particle;
    u32 next_temporary_emitter;
};

struct particle_renderer
{
    struct particle_render_data data[MAX_PARTICLES];
    u32 vao;
    u32 vbo_vertices;
    u32 vbo_particles;
    u32 ibo;
    u32 ubo;
    u32 num_indices;
    u32 num_particles;
    u32 shader;
    u32 texture;
    b32 initialized;
};

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
