#include "platform.h"
#include "math.c"
#include "debug.c"
#include "opengl.h"
#include "file.h"

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

typedef struct vertex
{
    v3 position;
    v2 uv;
    v3 normal;
} vertex;

typedef struct mesh
{
    u32 vao;
    u32 vbo;
    u32 ibo;
    u32 num_indices;
} mesh;

#define MAX_BULLETS 8
#define MAX_ENEMIES 4

typedef struct memory_block
{
    s8* base;
    s8* current;
    s8* last;
    u64 size;
} memory_block;

typedef struct game_state
{
    game_player player;
    game_bullet bullets[MAX_BULLETS];
    game_enemy enemies[MAX_ENEMIES];
    mesh cube;
    mesh sphere;
    mesh wall;
    mesh floor;
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
    m4 perspective;
    m4 view;
    memory_block temporary;
} game_state;

#define MAP_WIDTH       15
#define MAP_HEIGHT      15
 
#define PLAYER_HEALTH               500.0f
#define PLAYER_SPEED                0.2f
#define PLAYER_HEALTH_PER_PACK      100.f
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

u8 map_data[] =
{
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ,0,
    1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1 ,0,
    1, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1 ,1,
    1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 1, 2, 2 ,1,
    1, 2, 2, 1, 2, 1, 1, 1, 1, 1, 0, 1, 2, 2 ,1,
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

void* memory_get(memory_block* block, u64 size)
{
    if (block->current + size > block->base + block->size)
    {
        debug_log("Not enough memory\n");

        return 0;
    }

    // Todo: check alignment
    block->last = block->current;
    block->current += size;

    return (void*)block->last;
}

void memory_free(memory_block* block)
{
    block->current = block->last;
}

void generate_vertex_array(mesh* mesh, vertex* vertices, u32 num_vertices,
    u32* indices)
{
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    glGenBuffers(1, &mesh->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(vertex), vertices,
        GL_DYNAMIC_DRAW);

    glGenBuffers(1, &mesh->ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->num_indices * sizeof(u32), 
        indices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // Todo: implement offsetof
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), 
        (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), 
        (void*)12);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
        (void*)20);
}

void mesh_render(mesh* mesh, m4* mvp, u32 texture, u32 shader)
{
    glBindVertexArray(mesh->vao);

    glUseProgram(shader);

    u32 uniform_mvp = glGetUniformLocation(shader, "MVP");
    u32 uniform_texture = glGetUniformLocation(shader, "texture");

    glUniform1i(uniform_texture, 0);
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, (GLfloat*)mvp);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, NULL);

    glUseProgram(0);
}

void map_render(game_state* state)
{
    for (u32 y = 0; y < MAP_HEIGHT; y++)
    {
        for (u32 x = 0; x < MAP_WIDTH; x++)
        {
            u8 tile = map_data[y * MAP_WIDTH + x];

            if (tile == 1)
            {
                for (u32 i = 0; i < state->level; i++)
                {
                    m4 transform = m4_translate(2*x, 2*y,
                        -1.0f * (state->level - i - 1));
                    m4 rotation = m4_rotate_z(0.0f);
                    m4 scale = m4_scale(1.0f, 1.0f, 1.0f);

                    m4 model = m4_mul(scale, rotation);
                    model = m4_mul(model, transform);

                    m4 mvp = m4_mul(model, state->view);
                    mvp = m4_mul(mvp, state->perspective);

                    mesh_render(&state->wall, &mvp, state->texture_tileset, 
                        state->shader);
                }
            }
            else if (tile == 2)
            {
                m4 transform = m4_translate(2*x, 2*y, -1.0f);
                m4 rotation = m4_rotate_z(0.0f);
                m4 scale = m4_scale(1.0f, 1.0f, 1.0f);

                m4 model = m4_mul(scale, rotation);
                model = m4_mul(model, transform);

                m4 mvp = m4_mul(model, state->view);
                mvp = m4_mul(mvp, state->perspective);

                mesh_render(&state->floor, &mvp, state->texture_tileset,
                    state->shader);
            }
        }
    }
}

void enemies_update(game_state* state, game_input* input)
{

}

void enemies_render(game_state* state)
{
    for (u32 i = 0; i < MAX_ENEMIES; i++)
    {
        game_enemy* enemy = &state->enemies[i];

        m4 transform = m4_translate(enemy->x, enemy->y, 0.0f);
        m4 rotation = m4_rotate_z(enemy->angle);
        m4 scale = m4_scale(0.5f, 0.5f, 0.75f);

        m4 model = m4_mul(scale, rotation);
        model = m4_mul(model, transform);

        m4 mvp = m4_mul(model, state->view);
        mvp = m4_mul(mvp, state->perspective);

        mesh_render(&state->cube, &mvp, state->texture_enemy, state->shader);
    }
}

void bullets_update(game_state* state, game_input* input)
{
    for (u32 i = 0; i < MAX_BULLETS; i++)
    {
        game_bullet* bullet = &state->bullets[i];

        bullet->x += bullet->velocity_x;
        bullet->y += bullet->velocity_y;
    }
}

void bullets_render(game_state* state)
{
    for (u32 i = 0; i < MAX_BULLETS; i++)
    {
        game_bullet* bullet = &state->bullets[i];

        m4 transform = m4_translate(bullet->x, bullet->y, 0.0f);
        m4 rotation = m4_rotate_z(bullet->angle);
        m4 scale = m4_scale(PROJECTILE_SIZE, PROJECTILE_SIZE,
            PROJECTILE_SIZE);

        m4 model = m4_mul(scale, rotation);
        model = m4_mul(model, transform);

        m4 mvp = m4_mul(model, state->view);
        mvp = m4_mul(mvp, state->perspective);

        mesh_render(&state->sphere, &mvp, state->texture_sphere,
            state->shader);
    }
}

void player_update(game_state* state, game_input* input)
{
    f32 velocity_x = 0.0f;
    f32 velocity_y = 0.0f;
    f32 move_speed = PLAYER_SPEED;

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

    state->player.x += velocity_x;
    state->player.y += velocity_y;

    f32 mouse_x = (state->screen_width / 2.0f - input->mouse_x) * -1;
    f32 mouse_y = (state->screen_height / 2.0f - input->mouse_y);

    state->player.angle = f32_atan(mouse_y, mouse_x);

    if (input->shoot.key_down)
    {
        if (!state->fired)
        {
            if (++state->free_bullet == MAX_BULLETS)
            {
                state->free_bullet = 0;
            }

            game_bullet* bullet = &state->bullets[state->free_bullet];

            f32 dir_x = f32_cos(state->player.angle);
            f32 dir_y = f32_sin(state->player.angle);
            f32 speed = PISTOL_BULLET_SPEED;

            bullet->x = state->player.x;
            bullet->y = state->player.y;
            bullet->velocity_x = dir_x * speed;
            bullet->velocity_y = dir_y * speed;

            state->fired = true;
        }
    }
    else
    {
        state->fired = false;
    }
}

void player_render(game_state* state)
{
    m4 transform = m4_translate(state->player.x, state->player.y, 0.0f);
    m4 rotation = m4_rotate_z(state->player.angle);
    m4 scale = m4_scale(0.5f, 0.5f, 0.75f);

    m4 model = m4_mul(scale, rotation);
    model = m4_mul(model, transform);

    m4 mvp = m4_mul(model, state->view);
    mvp = m4_mul(mvp, state->perspective);

    mesh_render(&state->cube, &mvp, state->texture_player, state->shader);
}

// Todo: create single struct for header (requires packing)
typedef struct color_map_spec
{
    s16 index;
    s16 length;
    s8 size;
} color_map_spec;

typedef struct image_spec
{
    s16 x;
    s16 y;
    s16 width;
    s16 height;
    s8 depth;
    s8 desc;
} image_spec;

void tga_decode(s8* input, u64 out_size, s8* output, u32* width,
    u32* height)
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

u32 texture_create(memory_block* block, s8* path)
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

    file_open(&file, path);
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

b32 str_compare(s8* str1, s8* str2)
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

b32 is_digit(s8 c)
{
    return c >= '0' && c <= '9';
}

b32 is_space(s8 c)
{
    return (c >= 9 && c <= 13) || c == 32;
}

s32 s32_parse(s8* data, u64* size)
{
    // Todo: ignore leading whitespaces
    s32 value = 0;
    u64 bytes = 0;
    b32 negative = false;
    
    if (*data == '-')
    {
        negative = true;
        data++;
        bytes++;
    }

    while (is_digit(*data))
    {  
        u8 val = *data++ - '0';

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

f32 f32_parse(s8* data, u64* size)
{
    // Todo: ignore leading whitespaces
    // Todo: int parser has duplicate code
    f32 value = 0;
    b32 negative = false;
    u64 bytes = 0;

    if (*data == '-')
    {
        negative = true;
        data++;
        bytes++;
    }

    while (is_digit(*data))
    {  
        u8 val = *data++ - '0';

        value *= 10.0;
        value += val;
        bytes++;
    }

    if (*data++ == '.')
    {
        s32 num_decimals = 0;
        bytes++;
        
        while (is_digit(*data))
        {  
            u8 val = *data++ - '0';

            value *= 10.0;
            value += val;

            num_decimals--;
            bytes++;
        }

        if (*data++ == 'e')
        {
            s32 num_exponents = 0;
            b32 negative_exponent = false;

            bytes++;

            if (*data == '-')
            {
                negative_exponent = true;
                data++;
                bytes++;
            }
            else if (*data == '+')
            {
                negative_exponent = false;
                data++;
                bytes++;
            }

            u32 exponent = 0;

            while (is_digit(*data))
            {
                u8 val = *data++ - '0';

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

u64 string_read(s8* data, s8* str, u64 max_size)
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

void mesh_create(memory_block* block, s8* path, mesh* mesh)
{
    // Todo: remove statics
    static v3 in_vertices[4096];
    static v3 in_normals[4096];
    static v2 in_uvs[4096];
    static u32 in_faces[4096*3];
    static vertex vertices[4096];
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
    file_open(&file, path);
    file_size_get(&file, &file_size);

    file_data = memory_get(block, file_size);

    file_read(&file, file_data, file_size, &read_bytes);
    file_close(&file);

    s8* data = file_data;
    s8 str[255] = {0};

    for (u64 i = 0; i < read_bytes && *data != '\0'; i++)
    {
        u64 str_size = string_read(data, str, 255);

        if (str_compare(str, (s8*)"v"))
        {
            v3* v = &in_vertices[num_in_vertices++];

            // debug_log("v");
            
            data += str_size;
            str_size = string_read(data, str, 255);
            v->x = f32_parse(str, NULL);
            // debug_log(" %f", v->x);

            data += str_size;
            str_size = string_read(data, str, 255);
            v->y = f32_parse(str, NULL);
            // debug_log(" %f", v->y);

            data += str_size;
            str_size = string_read(data, str, 255);
            v->z = f32_parse(str, NULL);
            // debug_log(" %f\n", v->z);
        }
        else if (str_compare(str, (s8*)"vt"))
        {
            v2* uv = &in_uvs[num_in_uvs++];

            // debug_log("vt");            

            data += str_size;
            str_size = string_read(data, str, 255);
            uv->x = f32_parse(str, NULL);
            // debug_log(" %f", uv->x);

            data += str_size;
            str_size = string_read(data, str, 255);
            uv->y = f32_parse(str, NULL);
            // debug_log(" %f\n", uv->y);
        }
        else if (str_compare(str, (s8*)"vn"))
        {
            v3* n = &in_normals[num_in_normals++];

            // debug_log("vn");

            data += str_size;
            str_size = string_read(data, str, 255);
            n->x = f32_parse(str, NULL);
            // debug_log(" %f", n->x);

            data += str_size;
            str_size = string_read(data, str, 255);
            n->y = f32_parse(str, NULL);
            // debug_log(" %f", n->y);

            data += str_size;
            str_size = string_read(data, str, 255);
            n->z = f32_parse(str, NULL);
            // debug_log(" %f\n", n->z);
        }
        else if (str_compare(str, (s8*)"f"))
        {
            // debug_log("f");

            for (u32 i = 0; i < 3; i++)
            {
                u32* face = &in_faces[num_in_faces];

                data += str_size;
                str_size = string_read(data, str, 255);

                s8* s = str;

                u64 bytes_read = 0;
                face[0] = s32_parse(s, &bytes_read); 
                // debug_log(" %d", face[0]);
                s += bytes_read + 1;
                // debug_log("%c", *s++);
                face[1] = s32_parse(s, &bytes_read);
                // debug_log("%d", face[1]);
                s += bytes_read + 1;
                // debug_log("%c", *s++);
                face[2] = s32_parse(s, &bytes_read); 
                // debug_log("%d", face[2]);

                num_in_faces += 3;
            }

            // debug_log("\n");
        }
        else
        {
            data += str_size;
        }
    }

    for (u32 i = 0; i < num_in_faces; i += 3)
    {
        u32* face = &in_faces[i];

        vertex v;

        v.position = in_vertices[face[0] - 1];
        v.uv = in_uvs[face[1] - 1];
        v.normal = in_normals[face[2] - 1];

        b32 found = false;

        for (u32 j = 0; j < num_vertices; j++)
        {
            vertex other = vertices[j];

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

u32 program_create(memory_block* block, s8* vertex_shader_path,
    s8* fragment_shader_path)
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

    file_open(&file, vertex_shader_path);
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

    file_open(&file, fragment_shader_path);
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

void game_init(game_memory* memory, s32 screen_width, s32 screen_height,
    opengl_functions* gl, file_functions* file)
{
    game_state* state = (game_state*)memory->base;

    opengl_functions_set(gl);
    file_functions_set(file);

    if (!memory->initialized)
    {
        s32 version_major = 0;
        s32 version_minor = 0;

        glGetIntegerv(GL_MAJOR_VERSION, &version_major);
        glGetIntegerv(GL_MINOR_VERSION, &version_minor);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glClearColor(0.2f, 0.65f, 0.4f, 0.0f);

        debug_log("OpenGL %i.%i\n", version_major, version_minor);

        state->temporary.base = (s8*)state + sizeof(game_state);
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

        state->screen_width = screen_width;
        state->screen_height = screen_height;
        state->perspective = m4_perspective(60.0f, 
            (f32)state->screen_width/(f32)state->screen_height, 0.1f, 100.0f);

        state->player.x = 7.0f;
        state->player.y = 6.0f;

        state->level = 10;
        
        mesh_create(&state->temporary, "assets/meshes/cube.mesh",
            &state->cube);
        mesh_create(&state->temporary, "assets/meshes/sphere.mesh",
            &state->sphere);
        mesh_create(&state->temporary, "assets/meshes/wall.mesh",
            &state->wall);
        mesh_create(&state->temporary, "assets/meshes/floor.mesh",
            &state->floor);

        while (state->num_enemies < MAX_ENEMIES)
        {
            game_enemy* enemy = &state->enemies[state->num_enemies++];
                
            enemy->x = 5.0f - state->num_enemies * 5.0f;
            enemy->y = 0.0f;
        }

        memory->initialized = true;
    }

    if (!memory->initialized)
    {
        debug_log("game_init: end of init, memory not initalized!\n");
    }
}

void game_update(game_memory* memory, game_input* input)
{
    if (memory->initialized)
    {
        game_state* state = (game_state*)memory->base;

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

        state->view = m4_translate(-state->player.x, -state->player.y, -20.0f);

        map_render(state);
        player_render(state);
        enemies_render(state);
        bullets_render(state);
    }
    else
    {
        debug_log("game_update: memory not initialized!\n");
    }
}