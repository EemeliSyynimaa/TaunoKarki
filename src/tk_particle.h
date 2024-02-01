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
