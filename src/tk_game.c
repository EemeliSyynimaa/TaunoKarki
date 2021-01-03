#include "tk_platform.h"
#include "tk_math.h"
#include "tk_opengl.h"
#include "tk_file.h"

#include <string.h>

struct player
{
    struct v2 position;
    struct v2 velocity;
    f32 angle;
};

struct bullet
{
    struct v2 position;
    struct v2 velocity;
    f32 angle;
};

struct enemy
{
    struct v2 position;
    f32 angle;
};

struct vertex
{
    struct v3 position;
    struct v2 uv;
    struct v3 normal;
};

struct mesh
{
    u32 vao;
    u32 vbo;
    u32 ibo;
    u32 num_indices;
};

#define MAX_BULLETS 8
#define MAX_ENEMIES 8

struct memory_block
{
    s8* base;
    s8* current;
    s8* last;
    u64 size;
};

struct game_state
{
    struct player player;
    struct bullet bullets[MAX_BULLETS];
    struct enemy enemies[MAX_ENEMIES];
    struct mesh cube;
    struct mesh sphere;
    struct mesh wall;
    struct mesh floor;
    struct m4 perspective;
    struct m4 view;
    struct memory_block temporary;
    b32 fired;
    f32 accumulator;
    u32 shader;
    u32 texture_tileset;
    u32 texture_sphere;
    u32 texture_player;
    u32 texture_enemy;
    u32 free_bullet;
    u32 num_enemies;
    u32 level;
    s32 screen_width;
    s32 screen_height;
};

#define MAP_WIDTH       15
#define MAP_HEIGHT      15
 
#define PLAYER_HEALTH               500.0f
#define PLAYER_SPEED                0.1f
#define PLAYER_HEALTH_PER_PACK      100.f
#define PLAYER_ACCELERATION         0.001f
#define ENEMY_HEALTH                50.0
#define ENEMY_SPEED                 0.12
#define ENEMY_ACTIVATION_DISTANCE   20.0
#define ENEMY_HEALTH_PER_LEVEL      10.0
#define ENEMY_HIT_DAMAGE            50.0
#define ENEMY_HIT_DAMAGE_PER_LEVEL  5.0f
#define ENEMY_ANGLE_OF_VISION       60.0
#define MACHINEGUN_DAMAGE           25.0
#define MACHINEGUN_RELOAD_TIME      2.50
#define MACHINEGUN_CLIP_SIZE        25.0
#define MACHINEGUN_BULLET_SPEED     1.0f
#define MACHINEGUN_FIRE_RATE        0.12f
#define MACHINEGUN_BULLET_SPREAD    0.05
#define PISTOL_DAMAGE               30.0
#define PISTOL_BULLET_SPEED         1.0f
#define PISTOL_CLIP_SIZE            8.0f
#define PISTOL_RELOAD_TIME          1.5f
#define PISTOL_BULLET_SPREAD        0.01
#define SHOTGUN_DAMAGE              12.5
#define SHOTGUN_BULLET_SPEED        1.0f
#define SHOTGUN_CLIP_SIZE           7.0f
#define SHOTGUN_RELOAD_TIME         3.0f
#define SHOTGUN_FIRE_RATE           0.7f
#define SHOTGUN_BULLET_SPREAD       0.12f
#define SHOTGUN_NUMBER_OF_SHELLS    12
#define PROJECTILE_SIZE             0.1f
#define PLAYER_SIZE                 0.45f
#define WALL_SIZE                   1.0f

#define TILE_NOTHING    0
#define TILE_WALL       1
#define TILE_FLOOR      2

struct v4 color_white = {{{ 1.0, 1.0, 1.0, 1.0 }}};
struct v4 color_black = {{{ 0.0, 0.0, 0.0, 0.0 }}};
struct v4 color_red   = {{{ 1.0, 0.0, 0.0, 0.0 }}}; 
struct v4 color_blue  = {{{ 0.0, 0.0, 1.0, 0.0 }}};

u8 map_data[] =
{
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ,0,
    1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1 ,0,
    1, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1 ,1,
    1, 2, 1, 2, 2, 1, 0, 0, 0, 0, 0, 1, 2, 2 ,1,
    1, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 1, 2, 2 ,1,
    1, 1, 1, 1, 2, 1, 2, 2, 2, 1, 1, 1, 2, 2 ,1,
    0, 0, 0, 1, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2 ,1,
    0, 0, 0, 1, 2, 1, 2, 2, 2, 1, 2, 2, 2, 2 ,1,
    0, 0, 0, 1, 2, 1, 2, 2, 2, 1, 2, 2, 1, 1 ,1,
    0, 0, 0, 1, 2, 1, 1, 1, 1, 1, 2, 2, 1, 0 ,0,
    0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0 ,0,
    0, 0, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 0 ,0,
    0, 0, 1, 2, 2, 1, 2, 2, 1, 0, 0, 0, 0, 0 ,0,
    0, 0, 1, 2, 2, 1, 2, 2, 1, 0, 0, 0, 0, 0 ,0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 ,0
};

void* memory_get(struct memory_block* block, u64 size)
{
    if (block->current + size > block->base + block->size)
    {
        LOG("Not enough memory\n");

        return 0;
    }

    // Todo: check alignment
    block->last = block->current;
    block->current += size;

    return (void*)block->last;
}

void memory_free(struct memory_block* block)
{
    block->current = block->last;
}

void generate_vertex_array(struct mesh* mesh, struct vertex* vertices, 
    u32 num_vertices, u32* indices)
{
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    glGenBuffers(1, &mesh->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(struct vertex), 
        vertices, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &mesh->ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->num_indices * sizeof(u32), 
        indices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // Todo: implement offsetof
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), 
        (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex),
        (void*)12);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex),
        (void*)20);
}

void mesh_render(struct mesh* mesh, struct m4* mvp, u32 texture, u32 shader, 
    struct v4 color)
{
    glBindVertexArray(mesh->vao);

    glUseProgram(shader);

    u32 uniform_mvp = glGetUniformLocation(shader, "MVP");
    u32 uniform_texture = glGetUniformLocation(shader, "texture");
    u32 uniform_color = glGetUniformLocation(shader, "color");

    glUniform1i(uniform_texture, 0);
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, (GLfloat*)mvp);
    glUniform4fv(uniform_color, 1, (GLfloat*)&color);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, NULL);

    glUseProgram(0);
}

b32 collides_circle_to_rect(f32 circle_x, f32 circle_y, f32 circle_radius,
    f32 rect_x, f32 rect_y, f32 rect_width, f32 rect_height)
{
    f32 result;

    f32 rect_left   = rect_x - rect_width * 0.5f;
    f32 rect_right  = rect_x + rect_width * 0.5f;
    f32 rect_top    = rect_y + rect_height * 0.5f;
    f32 rect_bottom = rect_y - rect_height * 0.5f;

    f32 col_x = circle_x;

    if (circle_x < rect_left)
    {
        col_x = rect_left;
    }
    else if (circle_x > rect_right)
    {
        col_x = rect_right;
    }

    f32 col_y = circle_y;

    if (circle_y < rect_bottom)
    {
        col_y = rect_bottom;
    }
    else if (circle_y > rect_top)
    {
        col_y = rect_top;
    }

    result = f32_distance(circle_x, circle_y, col_x, col_y) < circle_radius;

    return result;
}

u32 collidable_walls_get(f32 x, f32 y, f32 radius, f32 wall_size, u8* walls)
{
    u32 result = 0;
    u32 start_x = (u32)((x + 0.5f * wall_size - radius) / wall_size);
    u32 start_y = (u32)((y + 0.5f * wall_size - radius) / wall_size);
    u32 end_x = (u32)((x + 0.5f * wall_size + radius) / wall_size);
    u32 end_y = (u32)((y + 0.5f * wall_size + radius) / wall_size);

    for (u32 y = start_y; y <= end_y; y++)
    {
        for (u32 x = start_x; x <= end_x; x++)
        {
            walls[result++] = y * MAP_WIDTH + x;
        }
    }

    return result;
}

void map_render(struct game_state* state)
{
    // Todo: fix map rendering glitch (a wall block randomly drawn in a 
    //       wrong place)

    u8 collidables[8] = { 0 };
    u32 collidable_count = 0;

    collidable_count = collidable_walls_get(state->player.position.x, 
        state->player.position.y, PLAYER_SIZE, WALL_SIZE, collidables);

    for (u32 y = 0; y < MAP_HEIGHT; y++)
    {
        for (u32 x = 0; x < MAP_WIDTH; x++)
        {
            u32 index = y * MAP_WIDTH + x;

            u8 tile = map_data[index];

            struct v4 color = color_white;

            for (u32 i = 0; i < collidable_count; i++)
            {
                if (index == collidables[i])
                {
                    color = color_red;
                    break;
                }
            }
            
            if (tile == TILE_WALL)
            {
                for (u32 i = 0; i < state->level; i++)
                {
                    struct m4 transform = m4_translate(x, y,
                        -1.0f * (state->level - i - 1));
                    struct m4 rotation = m4_rotate_z(0.0f);
                    struct m4 scale = m4_scale_all(WALL_SIZE * 0.5f);

                    struct m4 model = m4_mul(scale, rotation);
                    model = m4_mul(model, transform);

                    struct m4 mvp = m4_mul(model, state->view);
                    mvp = m4_mul(mvp, state->perspective);

                    mesh_render(&state->wall, &mvp, state->texture_tileset, 
                        state->shader, color);
                }
            }
            else if (tile == TILE_FLOOR)
            {
                struct m4 transform = m4_translate(x, y, -WALL_SIZE * 0.5f);
                struct m4 rotation = m4_rotate_z(0.0f);
                struct m4 scale = m4_scale_all(WALL_SIZE * 0.5f);

                struct m4 model = m4_mul(scale, rotation);
                model = m4_mul(model, transform);

                struct m4 mvp = m4_mul(model, state->view);
                mvp = m4_mul(mvp, state->perspective);

                mesh_render(&state->floor, &mvp, state->texture_tileset,
                    state->shader, color);
            }
        }
    }
}

void enemies_update(struct game_state* state, struct game_input* input)
{

}

void enemies_render(struct game_state* state)
{
    for (u32 i = 0; i < MAX_ENEMIES; i++)
    {
        struct enemy* enemy = &state->enemies[i];

        struct m4 transform = m4_translate(enemy->position.x, enemy->position.y, 
            0.0f);
        struct m4 rotation = m4_rotate_z(enemy->angle);
        struct m4 scale = m4_scale_all(PLAYER_SIZE * 0.5f);

        struct m4 model = m4_mul(scale, rotation);
        model = m4_mul(model, transform);

        struct m4 mvp = m4_mul(model, state->view);
        mvp = m4_mul(mvp, state->perspective);

        mesh_render(&state->cube, &mvp, state->texture_enemy, state->shader,
            color_white);
    }
}

void bullets_update(struct game_state* state, struct game_input* input)
{
    for (u32 i = 0; i < MAX_BULLETS; i++)
    {
        struct bullet* bullet = &state->bullets[i];

        bullet->position.x += bullet->velocity.x;
        bullet->position.y += bullet->velocity.y;
    }
}

void bullets_render(struct game_state* state)
{
    for (u32 i = 0; i < MAX_BULLETS; i++)
    {
        struct bullet* bullet = &state->bullets[i];

        struct m4 transform = m4_translate(bullet->position.x, 
            bullet->position.y, 0.0f);
        struct m4 rotation = m4_rotate_z(bullet->angle);
        struct m4 scale = m4_scale_all(PROJECTILE_SIZE);

        struct m4 model = m4_mul(scale, rotation);
        model = m4_mul(model, transform);

        struct m4 mvp = m4_mul(model, state->view);
        mvp = m4_mul(mvp, state->perspective);

        mesh_render(&state->sphere, &mvp, state->texture_sphere,
            state->shader, color_white);
    }
}

b32 player_collides_to_wall(f32 x, f32 y)
{
    u8 collidables[8] = { 0 };
    u32 collidable_count = 0;

    collidable_count = collidable_walls_get(x, y, PLAYER_SIZE, 
        WALL_SIZE, collidables);

    for (u32 i = 0; i < collidable_count; i++)
    {
        if (map_data[collidables[i]] == 1)
        {
            f32 tile_x = collidables[i] % MAP_WIDTH;
            f32 tile_y = collidables[i] / MAP_WIDTH;

             b32 collides = collides_circle_to_rect(x, y, PLAYER_SIZE, 
                tile_x, tile_y, WALL_SIZE, WALL_SIZE);

             if (collides)
             {
                return true;
             }            
        }
    }

    return false;
}

void player_update(struct game_state* state, struct game_input* input)
{
    struct v2 direction = { 0.0f };
    struct v2 acceleration = { 0.0f };
    struct player* player = &state->player;

    if (input->move_left.key_down)
    {
        direction.x -= 1.0f;
    }
    
    if (input->move_right.key_down)
    {
        direction.x += 1.0f;
    }
    
    if (input->move_down.key_down)
    {
        direction.y -= 1.0f;
    }

    if (input->move_up.key_down)
    {
        direction.y += 1.0f;
    }

    direction = v2_normalize(direction);

    player->velocity.x += direction.x * PLAYER_ACCELERATION;
    player->velocity.y += direction.y * PLAYER_ACCELERATION;

    if (player_collides_to_wall(
        player->position.x + player->velocity.x, 
        player->position.y))
    {
        player->velocity.x = 0.0f;
    }

    if (player_collides_to_wall(player->position.x, 
        player->position.y + player->velocity.y))
    {
        player->velocity.y = 0.0f;
    }

    player->position.x += player->velocity.x;
    player->position.y += player->velocity.y;

    f32 mouse_x = (state->screen_width / 2.0f - input->mouse_x) * -1;
    f32 mouse_y = (state->screen_height / 2.0f - input->mouse_y);

    player->angle = f32_atan(mouse_y, mouse_x);

    if (input->shoot.key_down)
    {
        if (!state->fired)
        {
            if (++state->free_bullet == MAX_BULLETS)
            {
                state->free_bullet = 0;
            }

            struct bullet* bullet = &state->bullets[state->free_bullet];

            f32 dir_x = f32_cos(player->angle);
            f32 dir_y = f32_sin(player->angle);
            f32 speed = PISTOL_BULLET_SPEED;

            bullet->position.x = player->position.x;
            bullet->position.y = player->position.y;
            bullet->velocity.x = dir_x * speed;
            bullet->velocity.y = dir_y * speed;

            state->fired = true;
        }
    }
    else
    {
        state->fired = false;
    }
}

void player_render(struct game_state* state)
{
    struct m4 transform = m4_translate(state->player.position.x, 
        state->player.position.y, 0.0f);
    struct m4 rotation = m4_rotate_z(state->player.angle);
    struct m4 scale = m4_scale_xyz(PLAYER_SIZE, PLAYER_SIZE, 0.25f);

    struct m4 model = m4_mul(scale, rotation);
    model = m4_mul(model, transform);

    struct m4 mvp = m4_mul(model, state->view);
    mvp = m4_mul(mvp, state->perspective);

    mesh_render(&state->sphere, &mvp, state->texture_sphere, state->shader,
        color_white);

    {
        f32 length = state->player.velocity.x / PLAYER_SPEED * 0.5f * 
            PLAYER_SIZE;
        f32 pos_x = state->player.position.x + length;

        if (length < 0)
        {
            length = f32_abs(length);
        }

        transform = m4_translate(pos_x, state->player.position.y, 1.0f);
        rotation = m4_rotate_z(0.0f);
        scale = m4_scale_xyz(length, 0.01f, 0.01f);

        model = m4_mul(scale, rotation);
        model = m4_mul(model, transform);

        mvp = m4_mul(model, state->view);
        mvp = m4_mul(mvp, state->perspective);

        mesh_render(&state->floor, &mvp, state->texture_tileset,
            state->shader, color_blue);
    }

    {
        f32 length = state->player.velocity.y / PLAYER_SPEED * 0.5f * 
            PLAYER_SIZE;
        f32 pos_y = state->player.position.y + length;

        if (length < 0)
        {
            length = f32_abs(length);
        }

        transform = m4_translate(state->player.position.x, pos_y, 1.0f);
        rotation = m4_rotate_z(0.0f);
        scale = m4_scale_xyz(0.01f, length, 0.01f);

        model = m4_mul(scale, rotation);
        model = m4_mul(model, transform);

        mvp = m4_mul(model, state->view);
        mvp = m4_mul(mvp, state->perspective);

        mesh_render(&state->floor, &mvp, state->texture_tileset,
            state->shader, color_blue);
    }

    // {
    //     transform = m4_translate(state->player.position.x, 
    //         state->player.position.y, 1.0f);
    //     rotation = m4_rotate_z(0.0f);
    //     scale = m4_scale_xyz(0.01f, PLAYER_SIZE, 0.01f);

    //     model = m4_mul(scale, rotation);
    //     model = m4_mul(model, transform);

    //     mvp = m4_mul(model, state->view);
    //     mvp = m4_mul(mvp, state->perspective);

    //     mesh_render(&state->floor, &mvp, state->texture_tileset,
    //         state->shader, color_blue);
    // }
}

// Todo: create single struct for header (requires packing)
struct color_map_spec
{
    s16 index;
    s16 length;
    s8 size;
};

struct image_spec
{
    s16 x;
    s16 y;
    s16 width;
    s16 height;
    s8 depth;
    s8 desc;
};

void tga_decode(s8* input, u64 out_size, s8* output, u32* width,
    u32* height)
{
    input += 8;
    struct image_spec* i_spec = (struct image_spec*)input;
    input += 10;

    // Todo: read (or skip) image id and color map stuff

    u64 bytes_per_color = i_spec->depth / 8;
    u64 byte_count = i_spec->height * i_spec->width;

    for (u64 i = 0; i < byte_count && i < out_size; i++)
    {
        output[2] = input[0];
        output[1] = input[1];
        output[0] = input[2];
        output[3] = input[3];

        output += bytes_per_color;
        input += bytes_per_color;
    }

    *width = i_spec->width;
    *height = i_spec->height;
}

u32 texture_create(struct memory_block* block, char* path)
{
    u64 read_bytes = 0;
    u32 target = GL_TEXTURE_2D;
    u32 id = 0;
    u32 width = 0;
    u32 height = 0;

    file_handle file;
    u64 file_size = 0;
    s8* file_data = 0;
    s8* pixel_data = 0; 

    file_open(&file, path, true);
    file_size_get(&file, &file_size);

    file_data = memory_get(block, file_size);
    pixel_data = memory_get(block, file_size);

    file_read(&file, file_data, file_size, &read_bytes);
    file_close(&file);

    tga_decode(file_data, read_bytes, pixel_data, &width,
        &height);

    glGenTextures(1, &id);
    glBindTexture(target, id);

    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(target, 0, GL_RGBA, width, height, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, pixel_data);

    memory_free(block);
    memory_free(block);

    return id;
}

b32 str_compare(char* str1, char* str2)
{
    while (*str1++ == *str2++)
    {
        if (*str1 == '\0')
        {
            return true;
        }
    }

    return false;
}

b32 is_digit(char c)
{
    return c >= '0' && c <= '9';
}

b32 is_space(char c)
{
    return (c >= 9 && c <= 13) || c == 32;
}

s32 s32_parse(char* str, u64* size)
{
    // Todo: ignore leading whitespaces
    s32 value = 0;
    u64 bytes = 0;
    b32 negative = false;
    
    if (*str == '-')
    {
        negative = true;
        str++;
        bytes++;
    }

    while (is_digit(*str))
    {  
        u8 val = *str++ - '0';

        value *= 10.0;
        value += val;

        bytes++;
    }

    if (negative)
    {
        value *= -1;
    }

    if (size)
    {
        *size = bytes;
    }

    return value;
}

f32 f32_parse(char* str, u64* size)
{
    // Todo: ignore leading whitespaces
    // Todo: int parser has duplicate code
    f32 value = 0;
    b32 negative = false;
    u64 bytes = 0;

    if (*str == '-')
    {
        negative = true;
        str++;
        bytes++;
    }

    while (is_digit(*str))
    {  
        u8 val = *str++ - '0';

        value *= 10.0;
        value += val;
        bytes++;
    }

    if (*str++ == '.')
    {
        s32 num_decimals = 0;
        bytes++;
        
        while (is_digit(*str))
        {  
            u8 val = *str++ - '0';

            value *= 10.0;
            value += val;

            num_decimals--;
            bytes++;
        }

        if (*str++ == 'e')
        {
            b32 negative_exponent = false;

            bytes++;

            if (*str == '-')
            {
                negative_exponent = true;
                str++;
                bytes++;
            }
            else if (*str == '+')
            {
                negative_exponent = false;
                str++;
                bytes++;
            }

            u32 exponent = 0;

            while (is_digit(*str))
            {
                u8 val = *str++ - '0';

                exponent *= 10.0;
                exponent += val;

                bytes++;
            }

            num_decimals += negative_exponent ? -exponent : exponent;
        }

        while (num_decimals < 0)
        {
            value *= 0.1;

            num_decimals++;
        }
    
        while (num_decimals > 0)
        {
            value *= 10;

            num_decimals--;
        }
    }

    if (negative)
    {
        value *= -1;
    }

    if (size)
    {
        *size = bytes;
    }

    return value;
}

u64 string_read(char* data, char* str, u64 max_size)
{
    u64 bytes_read = 0;

    while (is_space(*data) && *data != '\0')
    {
        data++;
        bytes_read++;
    }

    u64 str_size = 0;

    while (!is_space(*data) && *data != '\0' && str_size < max_size - 1)
    {
        *str++ = *data++;

        str_size++;
        bytes_read++;
    }

    *str = '\0';

    return bytes_read;
}

void mesh_create(struct memory_block* block, char* path, struct mesh* mesh)
{
    // Todo: remove statics
    static struct v3 in_vertices[4096];
    static struct v3 in_normals[4096];
    static struct v2 in_uvs[4096];
    static struct vertex vertices[4096];
    static u32 in_faces[4096*3];
    static u32 indices[4096];

    u32 num_in_vertices = 0;
    u32 num_in_normals = 0;
    u32 num_in_uvs = 0;
    u32 num_in_faces = 0;
    u32 num_vertices = 0;

    // Todo:
    // X read file size
    // X reserve space for file data
    // - reserve space for vertices, faces, texture coords and normals
    //   - can it be calculated beforehand?
    //   - read each line and count each face, vertex normal etc and then
    //     reserve memory accordingly?
    // - reserve space for vertices and indices
    // - count indices

    u64 read_bytes = 0;
    u64 file_size = 0;
    s8* file_data = 0;

    file_handle file;
    file_open(&file, path, true);
    file_size_get(&file, &file_size);

    file_data = memory_get(block, file_size);

    file_read(&file, file_data, file_size, &read_bytes);
    file_close(&file);

    char* data = (char*)file_data;
    char str[255] = {0};

    for (u64 i = 0; i < read_bytes && *data != '\0'; i++)
    {
        u64 str_size = string_read(data, str, 255);

        if (str_compare(str, "v"))
        {
            struct v3* v = &in_vertices[num_in_vertices++];

            // LOG("v");
            
            data += str_size;
            str_size = string_read(data, str, 255);
            v->x = f32_parse(str, NULL);
            // LOG(" %f", v->x);

            data += str_size;
            str_size = string_read(data, str, 255);
            v->y = f32_parse(str, NULL);
            // LOG(" %f", v->y);

            data += str_size;
            str_size = string_read(data, str, 255);
            v->z = f32_parse(str, NULL);
            // LOG(" %f\n", v->z);
        }
        else if (str_compare(str, "vt"))
        {
            struct v2* uv = &in_uvs[num_in_uvs++];

            // LOG("vt");            

            data += str_size;
            str_size = string_read(data, str, 255);
            uv->x = f32_parse(str, NULL);
            // LOG(" %f", uv->x);

            data += str_size;
            str_size = string_read(data, str, 255);
            uv->y = f32_parse(str, NULL);
            // LOG(" %f\n", uv->y);
        }
        else if (str_compare(str, "vn"))
        {
            struct v3* n = &in_normals[num_in_normals++];

            // LOG("vn");

            data += str_size;
            str_size = string_read(data, str, 255);
            n->x = f32_parse(str, NULL);
            // LOG(" %f", n->x);

            data += str_size;
            str_size = string_read(data, str, 255);
            n->y = f32_parse(str, NULL);
            // LOG(" %f", n->y);

            data += str_size;
            str_size = string_read(data, str, 255);
            n->z = f32_parse(str, NULL);
            // LOG(" %f\n", n->z);
        }
        else if (str_compare(str, "f"))
        {
            // LOG("f");

            for (u32 i = 0; i < 3; i++)
            {
                u32* face = &in_faces[num_in_faces];

                data += str_size;
                str_size = string_read(data, str, 255);

                char* s = str;

                u64 bytes_read = 0;
                face[0] = s32_parse(s, &bytes_read); 
                // LOG(" %d", face[0]);
                s += bytes_read + 1;
                // LOG("%c", *s++);
                face[1] = s32_parse(s, &bytes_read);
                // LOG("%d", face[1]);
                s += bytes_read + 1;
                // LOG("%c", *s++);
                face[2] = s32_parse(s, &bytes_read); 
                // LOG("%d", face[2]);

                num_in_faces += 3;
            }

            // LOG("\n");
        }
        else
        {
            data += str_size;
        }
    }

    for (u32 i = 0; i < num_in_faces; i += 3)
    {
        u32* face = &in_faces[i];

        struct vertex v;

        v.position = in_vertices[face[0] - 1];
        v.uv = in_uvs[face[1] - 1];
        v.normal = in_normals[face[2] - 1];

        b32 found = false;

        for (u32 j = 0; j < num_vertices; j++)
        {
            // struct vertex other = vertices[j];

            // Todo: fix this
            // if (v.position == other.position && v.uv == other.uv &&
            //     v.normal == other.normal)
            // {
            //     mesh->indices[mesh->num_indices++] = j;

            //     found = true;
            //     break;
            // }
        }

        if (!found)
        {
            vertices[num_vertices++] = v;
            indices[mesh->num_indices++] = num_vertices - 1;
        }
    }

    memory_free(block);

    generate_vertex_array(mesh, vertices, num_vertices, indices);
}

u32 program_create(struct memory_block* block, char* vertex_shader_path,
    char* fragment_shader_path)
{
    u64 read_bytes = 0;
    u64 file_size = 0;
    s8* file_data = 0;

    u32 result = 0;
    u32 program = glCreateProgram();
    u32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    u32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    // Todo: implement assert
    // assert(program);
    // assert(vertex_shader);
    // assert(fragment_shader);

    // Note: glShaderSource requires for each string to be null terminated.
    // If read directly from a file, each line ends in CR LF (0d 0a).
    // These should be replaced with 0.
    file_handle file;

    file_open(&file, vertex_shader_path, true);
    file_size_get(&file, &file_size);

    file_data = memory_get(block, file_size);

    file_read(&file, file_data, file_size, &read_bytes);
    file_close(&file);

    const GLchar* temp = (const GLchar*)file_data;

    glShaderSource(vertex_shader, 1, &temp, 0);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, (GLint*)&result);
    // assert(result);

    // Todo: remove memset
    memset((void*)file_data, 0, file_size);

    memory_free(block);

    file_open(&file, fragment_shader_path, true);
    file_size_get(&file, &file_size);

    file_data = memory_get(block, file_size);

    file_read(&file, file_data, file_size, &read_bytes);
    file_close(&file);

    temp = (const GLchar*)file_data;

    glShaderSource(fragment_shader, 1, &temp, 0);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, (GLint*)&result);
    // assert(result);

    // Todo: remove memset
    memset((void*)file_data, 0, file_size);

    memory_free(block);

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, (GLint*)&result);
    // assert(result);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

void game_init(struct game_memory* memory, struct game_init* init)
{
    struct game_state* state = (struct game_state*)memory->base;

    _log = *init->log;
    opengl_functions_set(init->gl);
    file_functions_set(init->file);

    // Todo: should we check if copied functions are valid before use?

    s32 version_major = 0;
    s32 version_minor = 0;

    glGetIntegerv(GL_MAJOR_VERSION, &version_major);
    glGetIntegerv(GL_MINOR_VERSION, &version_minor);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    LOG("OpenGL %i.%i\n", version_major, version_minor);

    if (!memory->initialized)
    {
        state->temporary.base = (s8*)state + sizeof(struct game_state);
        state->temporary.last = state->temporary.base;
        state->temporary.current = state->temporary.base;
        state->temporary.size = 100*1024*1024;

        state->shader = program_create(&state->temporary,
            "assets/shaders/vertex.glsl",
            "assets/shaders/fragment.glsl");

        state->texture_tileset = texture_create(&state->temporary,
            "assets/textures/tileset.tga");
        state->texture_sphere = texture_create(&state->temporary,
            "assets/textures/sphere.tga");
        state->texture_player = texture_create(&state->temporary,
            "assets/textures/cube.tga");
        state->texture_enemy = texture_create(&state->temporary, 
            "assets/textures/enemy.tga");

        mesh_create(&state->temporary, "assets/meshes/cube.mesh",
            &state->cube);
        mesh_create(&state->temporary, "assets/meshes/sphere.mesh",
            &state->sphere);
        mesh_create(&state->temporary, "assets/meshes/wall.mesh",
            &state->wall);
        mesh_create(&state->temporary, "assets/meshes/floor.mesh",
            &state->floor);

        memory->initialized = true;
    }

    state->screen_width = init->screen_width;
    state->screen_height = init->screen_height;
    state->perspective = m4_perspective(60.0f, 
        (f32)state->screen_width/(f32)state->screen_height, 0.1f, 100.0f);

    state->num_enemies = MAX_ENEMIES;

    for (u32 i = 0; i < state->num_enemies; i++)
    {
        struct enemy* enemy = &state->enemies[i];
            
        enemy->position.x = 1.0f + i * 1.5f;
        enemy->position.y = -1.0f;
    }

    state->level = 6;

    state->player.position.x = 3.0f;
    state->player.position.y = 3.0f;

    glClearColor(0.2f, 0.65f, 0.4f, 0.0f);

    if (!memory->initialized)
    {
        LOG("game_init: end of init, memory not initalized!\n");
    }
}

void game_update(struct game_memory* memory, struct game_input* input)
{
    if (memory->initialized)
    {
        struct game_state* state = (struct game_state*)memory->base;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        f32 step = 1.0f / 60.0f;
        state->accumulator += input->delta_time;

        while (state->accumulator >= step)
        {
            state->accumulator -= step;

            player_update(state, input);
            enemies_update(state, input);
            bullets_update(state, input);
        }

        state->view = m4_translate(-state->player.position.x, 
            -state->player.position.y, -5.0f);

        map_render(state);
        player_render(state);
        enemies_render(state);
        bullets_render(state);

        // LOG("Player x=%2.3f y=%2.3f\n", state->player.x, state->player.y);
    }
    else
    {
        LOG("game_update: memory not initialized!\n");
    }
}
