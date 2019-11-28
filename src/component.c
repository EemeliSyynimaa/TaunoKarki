
struct transform_t
{
    glm::vec3 position;
    glm::vec3 size;
    f32 angle;
};

struct player_t
{
    u32 id;
};

struct cube_t
{
    u32 texture;
};

u32 num_players     = 0;
u32 num_transforms  = 0;
u32 num_cubes       = 0;
#define max_players     1
#define max_transforms  100
#define max_cubes       100

player_t players[max_players];
transform_t transforms[max_transforms];
cube_t cubes[max_cubes];

u32 create_guid()
{
    static u32 guid = 0;

    return guid++;
}

player_t* component_player_create()
{
    assert(num_players < max_players);

    return &players[num_players++];
}

transform_t* component_transform_create()
{
    assert(num_transforms < max_transforms);

    return &transforms[num_transforms++];
}

cube_t* component_cube_create()
{
    assert(num_cubes < max_cubes);

    return &cubes[num_cubes++];
}

u32 create_player(glm::vec3 position, u32 texture)
{
    u32 guid = create_guid();
    transform_t* transform = component_transform_create();

    transform->size = glm::vec3(0.5f, 0.5f, 0.75f);
    transform->position = position;
    transform->angle = 0.0f;

    player_t* player = component_player_create();
    player->id = guid;

    cube_t* cube = component_cube_create();
    cube->texture = texture;

    return guid;
}