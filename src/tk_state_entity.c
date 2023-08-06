// Components
enum
{
    ENTITY_COMPONENT_TRANSFORM,
    ENTITY_COMPONENT_HEALTH,
    ENTITY_COMPONENT_RIGID_BODY,
    ENTITY_COMPONENT_WEAPON,
    ENTITY_COMPONENT_COLLIDER,
    ENTITY_COMPONENT_CUBE_RENDERER,
    ENTITY_COMPONENT_CIRCLE_RENDERER,
    ENTITY_COMPONENT_ENEMY,
    ENTITY_COMPONENT_PLAYER,
    ENTITY_COMPONENT_ITEM,
    ENTITY_COMPONENT_PROJECTILE,
    ENTITY_COMPONENT_COUNT
};

struct entity_component_transform
{
    struct v2 position;
    struct v2 velocity;
};

struct entity_component_health
{
    f32 current;
    f32 max;
};

struct entity_component_rigid_body
{
    u32 type;
};

struct entity_component_weapon
{
    u32 type;
};

struct entity_component_collider
{
    u32 type;
};

struct entity_component_cube_renderer
{
    u32 type;
};

struct entity_component_circle_renderer
{
    u32 type;
};

struct entity_component_enemy
{
    u32 state;
};

struct entity_component_player
{
    u32 id;
};

struct entity_component_item
{
    u32 type;
};

struct entity_component_projectile
{
    u32 owner;
};

#define MAX_COMPONENTS 32
#define MAX_ENTITIES 128

// Entities
struct entity_type_enemy
{
    struct entity_component_transform transform;
    struct entity_component_health health;
    struct entity_component_rigid_body body;
    struct entity_component_collider collider;
    struct entity_component_weapon weapon;
    struct entity_component_cube_renderer cube_renderer;

    struct entity_component_enemy enemy;
};

struct entity_type_player
{
    struct entity_component_transform transform;
    struct entity_component_health health;
    struct entity_component_rigid_body body;
    struct entity_component_collider collider;
    struct entity_component_weapon weapon;
    struct entity_component_cube_renderer cube_renderer;

    struct entity_component_player enemy;
};

struct entity_type_wall
{
    struct entity_component_transform transform;
    struct entity_component_rigid_body body;
    struct entity_component_collider collider;
    struct entity_component_cube_renderer cube_renderer;
};

struct entity_type_item
{
    struct entity_component_transform transform;
    struct entity_component_rigid_body body;
    struct entity_component_collider collider;
    struct entity_component_cube_renderer cube_renderer;

    struct entity_component_item item;
};

struct entity_type_projectile
{
    struct entity_component_transform transform;
    struct entity_component_rigid_body body;
    struct entity_component_collider collider;
    struct entity_component_circle_renderer circle_renderer;

    struct entity_component_projectile projectile;
};

// Entity systems
// Note: one per entity type
// Note: act as a storage pool for different entity types
struct entity_system_enemy;
struct entity_system_player;
struct entity_system_wall;
struct entity_system_item;
struct entity_system_bullet;

struct entity_system
{
    struct object_pool* pool;
};

void entity_system_init();
void entity_system_reset();
void entity_system_insert();
void entity_system_update();

// Note: API for creating different type of entities
// Todo: probably should be possible to create multiple entities simultaneously
// Todo: return value: pointer to the entity or entity id?
void entity_system_enemy_create();
void entity_system_player_create();
void entity_system_wall_create();
void entity_system_item_create();
void entity_system_bullet_create();

// Note: API for updating different type of entities
void entity_system_enemy_update();
void entity_system_player_update();
void entity_system_wall_update();
void entity_system_item_update();
void entity_system_bullet_update();

struct entity_storage
{
    u32 entity_keys[MAX_ENTITIES];
    u32 entity_data[MAX_ENTITIES][32];
    struct object_pool component_data[ENTITY_COMPONENT_COUNT];
    struct memory_block block;

    // Todo: replace with linked list?
    u32 next_free;
};

u32 entity_storage_get_free(struct entity_storage* storage)
{
    u32 result = storage->next_free++;

    if (storage->next_free >= MAX_ENTITIES)
    {
        storage->next_free = 0;
    }

    return result;
}

void entity_storage_register_component(struct entity_storage* storage,
    u32 entity_id, u32 entity_data_size)
{
    LOG("%d: %d\n", entity_id, entity_data_size);
    struct object_pool* pool = &storage->component_data[entity_id];
    object_pool_init(pool, entity_data_size, MAX_COMPONENTS, &storage->block);
}

struct state_entity_data
{
    struct game_state* base;
    struct entity_storage storage;
    u32 status;
};

u32 entity_enemy_create(struct entity_storage* storage)
{
    u32 result = entity_storage_get_free(storage);

    u32* key = &storage->entity_keys[result];
    u32* data = storage->entity_data[result];

    (*key) = (
        1 << ENTITY_COMPONENT_TRANSFORM |       // 0 = 1
        1 << ENTITY_COMPONENT_HEALTH |          // 1 = 2
        1 << ENTITY_COMPONENT_RIGID_BODY |      // 2 = 4
        1 << ENTITY_COMPONENT_WEAPON |          // 3 = 8
        1 << ENTITY_COMPONENT_COLLIDER |        // 4 = 16
        1 << ENTITY_COMPONENT_CUBE_RENDERER |   // 5 = 32
        1 << ENTITY_COMPONENT_ENEMY             // 7 = 128
    );

    LOG("CREATED ENTITY WITH COMPONENTS %u\n", *key);

    return result;
}

void state_entity_init(void* data)
{
    struct state_entity_data* state = (struct state_entity_data*)data;

    // Todo: setting memoryblock is craaaazy amount of work
    state->storage.block.base = stack_alloc(&state->base->stack_permanent,
        MEGABYTES(1));
    state->storage.block.size = MEGABYTES(1);
    state->storage.block.current = state->storage.block.base;

    entity_storage_register_component(&state->storage,
        ENTITY_COMPONENT_TRANSFORM, sizeof(struct entity_component_transform));

    u32 enemy = entity_enemy_create(&state->storage);

    LOG("enemey %d\n", enemy);
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
