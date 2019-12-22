typedef struct game_player
{
    f32 x;
    f32 y;
    f32 angle;
} game_player;

typedef struct game_bullet
{
    f32 x;
    f32 y;
    f32 velocity_x;
    f32 velocity_y;
    f32 angle;
} game_bullet;

typedef struct game_enemy
{
    f32 x;
    f32 y;
    f32 angle;
} game_enemy;

typedef struct mesh
{
    void* vertices;
    void* indices;
    u32 vao;
    u32 vbo;
    u32 ibo;    
    u32 num_vertices;
    u32 num_indices;
} mesh;

#define MAX_BULLETS 8
#define MAX_ENEMIES 4

typedef struct game_state
{
    game_player player;
    game_bullet bullets[MAX_BULLETS];
    game_enemy enemies[MAX_ENEMIES];
    mesh cube;
    mesh sphere;
    mesh wall;
    AssetManager assets;
    b32 fired;
    f32 accumulator;
    u32 texture_tileset;
    u32 texture_sphere;
    u32 texture_player;
    u32 texture_enemy;
    u32 free_bullet;
    u32 num_enemies;
    s32 screen_width;
    s32 screen_height;
    glm::mat4 perspective;
    glm::mat4 view;
} game_state;

game_state state;

#define MAX_FILE_SIZE 1024*1024
#define MAP_WIDTH   15
#define MAP_HEIGHT  15

s8 file_data[MAX_FILE_SIZE];
s8 pixel_data[MAX_FILE_SIZE];

u8 map_data[] =
{
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ,0,
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 ,0,
    1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1 ,1,
    1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0 ,1,
    1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0 ,1,
    1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0 ,1,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 ,1,
    0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 ,1,
    0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1 ,1,
    0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0 ,0,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 ,0,
    0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0 ,0,
    0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0 ,0,
    0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0 ,0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 ,0
};

void generate_vertex_array(mesh* mesh)
{
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    glGenBuffers(1, &mesh->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh->num_vertices * sizeof(Vertex), mesh->vertices, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &mesh->ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->num_indices * sizeof(u32), mesh->indices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}

void mesh_render(mesh* mesh, glm::mat4* mvp, u32 texture)
{
    glBindVertexArray(mesh->vao);

    u32 program = state.assets.shaderProgram->getID();

    glUseProgram(program);

    u32 uniform_mvp = glGetUniformLocation(program, "MVP");
    u32 uniform_texture = glGetUniformLocation(program, "texture");

    glUniform1i(uniform_texture, 0);
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(*mvp));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, NULL);

    glUseProgram(0);
}

void map_render()
{
    for (u32 y = 0; y < MAP_HEIGHT; y++)
    {
        for (u32 x = 0; x < MAP_WIDTH; x++)
        {
            u8 tile = map_data[y * MAP_WIDTH + x];

            if (tile)
            {
                glm::mat4 transform = glm::translate(glm::vec3(2*x, 2*y, 0.0f));
                glm::mat4 rotation(1.0f);
                glm::mat4 scale(1.0f);

                glm::mat4 model = transform * rotation * scale;

                glm::mat4 mvp = state.perspective * state.view * model; 

                mesh_render(&state.cube, &mvp, state.texture_tileset);
            }
        }
    }
}

void enemies_update(game_input* input)
{

}

void enemies_render()
{
    for (u32 i = 0; i < MAX_ENEMIES; i++)
    {
        game_enemy* enemy = &state.enemies[i];

        glm::mat4 transform = glm::translate(glm::vec3(enemy->x, enemy->y, 0.0f));
        glm::mat4 rotation = glm::rotate(enemy->angle, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 scale = glm::scale(glm::vec3(0.5f, 0.5f, 0.75f));

        glm::mat4 model = transform * rotation * scale;

        glm::mat4 mvp = state.perspective * state.view * model;

        mesh_render(&state.cube, &mvp, state.texture_enemy);
    }
}

void bullets_update(game_input* input)
{
    for (u32 i = 0; i < MAX_BULLETS; i++)
    {
        game_bullet* bullet = &state.bullets[i];

        bullet->x += bullet->velocity_x;
        bullet->y += bullet->velocity_y;
    }
}

void bullets_render()
{
    for (u32 i = 0; i < MAX_BULLETS; i++)
    {
        game_bullet* bullet = &state.bullets[i];

        glm::mat4 transform = glm::translate(glm::vec3(bullet->x, bullet->y, 0.0f));
        glm::mat4 rotation = glm::rotate(bullet->angle, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 scale = glm::scale(glm::vec3(GLOBALS::PROJECTILE_SIZE));

        glm::mat4 model = transform * rotation * scale;

        glm::mat4 mvp = state.perspective * state.view * model;

        mesh_render(&state.sphere, &mvp, state.texture_sphere);
    }
}

void player_update(game_input* input)
{
    f32 velocity_x = 0.0f;
    f32 velocity_y = 0.0f;
    f32 move_speed = GLOBALS::PLAYER_SPEED;

    if (input->move_left.key_down)
    {
        velocity_x -= move_speed;
    }
    
    if (input->move_right.key_down)
    {
        velocity_x += move_speed;
    }

    if (input->move_up.key_down)
    {
        velocity_y += move_speed;
    }
    
    if (input->move_down.key_down)
    {
        velocity_y -= move_speed;
    }

    state.player.x += velocity_x;
    state.player.y += velocity_y;

    f32 mouse_x = (state.screen_width / 2.0f - input->mouse_x) * -1;
    f32 mouse_y = (state.screen_height / 2.0f - input->mouse_y);

    state.player.angle = glm::atan(mouse_y, mouse_x);

    if (input->shoot.key_down)
    {
        if (!state.fired)
        {
            if (++state.free_bullet == MAX_BULLETS)
            {
                state.free_bullet = 0;
            }

            game_bullet* bullet = &state.bullets[state.free_bullet];

            f32 dir_x = glm::cos(state.player.angle);
            f32 dir_y = glm::sin(state.player.angle);
            f32 speed = GLOBALS::PISTOL_BULLET_SPEED;

            bullet->x = state.player.x;
            bullet->y = state.player.y;
            bullet->velocity_x = dir_x * speed;
            bullet->velocity_y = dir_y * speed;

            state.fired = true;
        }
    }
    else
    {
        state.fired = false;
    }
}

void player_render()
{
    glm::mat4 transform = glm::translate(glm::vec3(state.player.x, state.player.y, 0.0f));
    glm::mat4 rotation = glm::rotate(state.player.angle, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 scale = glm::scale(glm::vec3(0.5f, 0.5f, 0.75f));

    glm::mat4 model = transform * rotation * scale;

    glm::mat4 mvp = state.perspective * state.view * model;

    mesh_render(&state.cube, &mvp, state.texture_player);
}

typedef struct color_map_spec
{
    // First entry index (2 bytes): index of first color map entry that is included in the file
    // Color map length (2 bytes): number of entries of the color map that are included in the file
    // Color map entry size (1 byte): number of bits per pixel
    s16 index;
    s16 length;
    s8 size;
} color_map_spec;

typedef struct image_spec
{
    // X-origin (2 bytes): absolute coordinate of lower-left corner for displays where origin is at the lower left
    // Y-origin (2 bytes): as for X-origin
    // Image width (2 bytes): width in pixels
    // Image height (2 bytes): height in pixels
    // Pixel depth (1 byte): bits per pixel
    // Image descriptor (1 byte): bits 3-0 give the alpha channel depth, bits 5-4 give direction
    s16 x;
    s16 y;
    s16 width;
    s16 height;
    s8 depth;
    s8 desc;
} image_spec;

void tga_decode(s8* input, u64 in_size, s8* output, u64* out_size, u32* width, u32* height)
{
    s8 id_length = *input++;
    s8 color_type = *input++;
    s8 image_type = *input++;
    
    color_map_spec* c_spec = (color_map_spec*)input;
    input += 5;

    image_spec* i_spec = (image_spec*)input;
    input += 10;

    // Todo: read (or skip) image id and color map stuff

    u64 bytes_per_color = i_spec->depth / 8;
    u64 byte_count = i_spec->height * i_spec->width;

    for (u64 i = 0; i < byte_count; i++)
    {
        output[2] = input[0];
        output[1] = input[1];
        output[0] = input[2];
        output[3] = input[3];

        output += bytes_per_color;
        input += bytes_per_color;
    }

    *out_size = i_spec->height * i_spec->width * bytes_per_color;
    *width = i_spec->width;
    *height = i_spec->height;
}

u32 texture_create(s8* path)
{
    u64 read_bytes = 0;
    u64 num_pixels = 0;
    u32 target = GL_TEXTURE_2D;
    u32 id = 0;
    u32 width = 0;
    u32 height = 0;

    load_file(path, file_data, MAX_FILE_SIZE, &read_bytes);
    tga_decode(file_data, read_bytes, pixel_data, &num_pixels, &width, &height);

    glGenTextures(1, &id);
    glBindTexture(target, id);

    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(target, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);

    return id;
}

void init_game(s32 screen_width, s32 screen_height)
{
    s32 version_major = 0;
    s32 version_minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &version_major);
    glGetIntegerv(GL_MINOR_VERSION, &version_minor);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    fprintf(stderr, "OpenGL %i.%i\n", version_major, version_minor);

    state.assets.loadAssets();

    state.texture_tileset = texture_create((s8*)"assets/textures/tileset.tga");
    state.texture_sphere = texture_create((s8*)"assets/textures/sphere.tga");
    state.texture_player = texture_create((s8*)"assets/textures/cube.tga");
    state.texture_enemy = texture_create((s8*)"assets/textures/enemy.tga");

    state.screen_width = screen_width;
    state.screen_height = screen_height;
    state.perspective = glm::perspective(glm::radians(60.0f), (f32)state.screen_width/(f32)state.screen_height, 0.1f, 100.0f);

    glm::vec3 position(7.0f, 6.0f, 0.0f);
    state.player.x = position.x;
    state.player.y = position.y;

    state.cube.num_vertices = state.assets.cubeMesh->getVertices().size();
    state.cube.num_indices = state.assets.cubeMesh->getIndices().size();
    state.cube.vertices = state.assets.cubeMesh->getVertices().data();
    state.cube.indices = state.assets.cubeMesh->getIndices().data();

    generate_vertex_array(&state.cube);

    state.sphere.num_vertices = state.assets.sphereMesh->getVertices().size();
    state.sphere.num_indices = state.assets.sphereMesh->getIndices().size();
    state.sphere.vertices = state.assets.sphereMesh->getVertices().data();
    state.sphere.indices = state.assets.sphereMesh->getIndices().data();

    generate_vertex_array(&state.sphere);

    state.wall.num_vertices = state.assets.wallMesh->getVertices().size();
    state.wall.num_indices = state.assets.wallMesh->getIndices().size();
    state.wall.vertices = state.assets.wallMesh->getVertices().data();
    state.wall.indices = state.assets.wallMesh->getIndices().data();

    generate_vertex_array(&state.wall);

    while (state.num_enemies < MAX_ENEMIES)
    {
        game_enemy* enemy = &state.enemies[state.num_enemies++];
            
        glm::vec3 position(5.0f - state.num_enemies*5.0f, 0.0f, 0.0f);

        enemy->x = position.x;
        enemy->y = position.y;
    }
}

void update_game(game_input* input)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    f32 step = 1.0f / 60.0f;
    state.accumulator += input->delta_time;

    while (state.accumulator >= step)
    {
        state.accumulator -= step;

        player_update(input);
        enemies_update(input);
        bullets_update(input);
    }

    state.view = glm::lookAt(
        glm::vec3(state.player.x, state.player.y, 20.0f),
        glm::vec3(state.player.x, state.player.y, 0),
        glm::vec3(0, 1, 0));

    map_render();
    player_render();
    enemies_render();
    bullets_render();
}