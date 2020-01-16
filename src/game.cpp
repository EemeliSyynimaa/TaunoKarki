
#include <math.h>

#define TK_PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679

typedef struct v2
{
    f32 x;
    f32 y;
} v2;

typedef struct v3
{
    f32 x;
    f32 y;
    f32 z;
} v3;

typedef struct m4
{
    f32 m[4][4];
} m4;


f32 tk_radians(f32 degrees)
{
    f32 value;

    value = degrees * TK_PI / 180.0;

    return value;
}

f32 tk_degrees(f32 radians)
{
    f32 value;

    value = radians * 180.0 / TK_PI;

    return value;
}

f32 tk_atan(f32 y, f32 x)
{
    // Todo: implement own atan(2) function
    return atan2(y, x);
}


f32 tk_sin(f32 angle)
{
    // Todo: implement own sin function
    return sin(angle);
}


f32 tk_cos(f32 angle)
{
    // Todo: implement own cos function
    return cos(angle);
}

m4 tk_translate(f32 x, f32 y, f32 z)
{
    m4 m = 
    {{
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        {    x,    y,    z, 1.0f }
    }};

    return m;
}

m4 tk_rotate_x(f32 angle)
{
    f32 c = tk_cos(angle);
    f32 s = tk_sin(angle);

    m4 m = 
    {{
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f,    c,    s, 0.0f },
        { 0.0f,   -s,    c, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    }};

    return m;
}

m4 tk_rotate_y(f32 angle)
{
    f32 c = tk_cos(angle);
    f32 s = tk_sin(angle);

    m4 m = 
    {{
        { c,    0.0f,   -s, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { s,    0.0f,    c, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    }};

    return m;
}

m4 tk_rotate_z(f32 angle)
{
    f32 c = tk_cos(angle);
    f32 s = tk_sin(angle);

    m4 m = 
    {{
        {    c,    s, 0.0f, 0.0f },
        {   -s,    c, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    }};

    return m;
}

m4 tk_scale(f32 x, f32 y, f32 z)
{
    m4 m = 
    {{
        {    x, 0.0f, 0.0f, 0.0f },
        { 0.0f,    y, 0.0f, 0.0f },
        { 0.0f, 0.0f,    z, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    }};

    return m;
}

m4 tk_m4_mul(m4 a, m4 b)
{
    m4 m;

    for (u32 i = 0; i < 4; i++)
    {
        for (u32 j = 0; j < 4; j++)
        {
            m.m[i][j] =
                a.m[i][0] * b.m[0][j] +
                a.m[i][1] * b.m[1][j] + 
                a.m[i][2] * b.m[2][j] + 
                a.m[i][3] * b.m[3][j];
        }
    }

    return m;
}

m4 tk_convert_m4(glm::mat4 t)
{
    m4 m;

    for (u32 i = 0; i < 4; i++)
    {
        for (u32 j = 0; j < 4; j++)
        {
            m.m[i][j] = t[i][j];
        }
    }

    return m;
}

m4 perspective(f32 fov, f32 aspect, f32 near, f32 far)
{
    m4 m;

    // Todo: implement own perspective function
    m = tk_convert_m4(glm::perspective(tk_radians(fov), aspect, 0.1f, 100.0f));

    return m;
}

m4 look_at(v3 eye, v3 center, v3 up)
{
    m4 m;

    // Todo: implement own look at function
    m = tk_convert_m4(glm::lookAt(
        glm::vec3(eye.x, eye.y, eye.z),
        glm::vec3(center.x, center.y, center.z),
        glm::vec3(up.x, up.y, up.z)));

    return m;
}

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
    vertex vertices[4096];
    u32 indices[4096];
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
    b32 fired;
    f32 accumulator;
    u32 shader;
    u32 texture_tileset;
    u32 texture_sphere;
    u32 texture_player;
    u32 texture_enemy;
    u32 free_bullet;
    u32 num_enemies;
    s32 screen_width;
    s32 screen_height;
    m4 perspective;
    m4 view;
} game_state;

game_state state;

#define MAX_FILE_SIZE   10*1024*1024
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
    glBufferData(GL_ARRAY_BUFFER, mesh->num_vertices * sizeof(vertex), mesh->vertices, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &mesh->ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->num_indices * sizeof(u32), mesh->indices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, position));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, uv));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));
}

void mesh_render(mesh* mesh, m4* mvp, u32 texture)
{
    glBindVertexArray(mesh->vao);

    glUseProgram(state.shader);

    u32 uniform_mvp = glGetUniformLocation(state.shader, "MVP");
    u32 uniform_texture = glGetUniformLocation(state.shader, "texture");

    glUniform1i(uniform_texture, 0);
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, (GLfloat*)mvp);

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
                m4 transform = tk_translate(2*x, 2*y, 0.0f);
                m4 rotation = tk_rotate_z(0.0f);
                m4 scale = tk_scale(1.0f, 1.0f, 1.0f);

                m4 model = tk_m4_mul(scale, rotation);
                model = tk_m4_mul(model, transform);

                m4 mvp = tk_m4_mul(model, state.view);
                mvp = tk_m4_mul(mvp, state.perspective);

                mesh_render(&state.wall, &mvp, state.texture_tileset);
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

        m4 transform = tk_translate(enemy->x, enemy->y, 0.0f);
        m4 rotation = tk_rotate_z(enemy->angle);
        m4 scale = tk_scale(0.5f, 0.5f, 0.75f);

        m4 model = tk_m4_mul(scale, rotation);
        model = tk_m4_mul(model, transform);

        m4 mvp = tk_m4_mul(model, state.view);
        mvp = tk_m4_mul(mvp, state.perspective);

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

        m4 transform = tk_translate(bullet->x, bullet->y, 0.0f);
        m4 rotation = tk_rotate_z(bullet->angle);
        m4 scale = tk_scale(PROJECTILE_SIZE, PROJECTILE_SIZE, PROJECTILE_SIZE);

        m4 model = tk_m4_mul(scale, rotation);
        model = tk_m4_mul(model, transform);

        m4 mvp = tk_m4_mul(model, state.view);
        mvp = tk_m4_mul(mvp, state.perspective);

        mesh_render(&state.sphere, &mvp, state.texture_sphere);
    }
}

void player_update(game_input* input)
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

    state.player.x += velocity_x;
    state.player.y += velocity_y;

    f32 mouse_x = (state.screen_width / 2.0f - input->mouse_x) * -1;
    f32 mouse_y = (state.screen_height / 2.0f - input->mouse_y);

    state.player.angle = tk_atan(mouse_y, mouse_x);

    if (input->shoot.key_down)
    {
        if (!state.fired)
        {
            if (++state.free_bullet == MAX_BULLETS)
            {
                state.free_bullet = 0;
            }

            game_bullet* bullet = &state.bullets[state.free_bullet];

            f32 dir_x = tk_cos(state.player.angle);
            f32 dir_y = tk_sin(state.player.angle);
            f32 speed = PISTOL_BULLET_SPEED;

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
    m4 transform = tk_translate(state.player.x, state.player.y, 0.0f);
    m4 rotation = tk_rotate_z(state.player.angle);
    m4 scale = tk_scale(0.5f, 0.5f, 0.75f);

    m4 model = tk_m4_mul(scale, rotation);
    model = tk_m4_mul(model, transform);

    m4 mvp = tk_m4_mul(model, state.view);
    mvp = tk_m4_mul(mvp, state.perspective);

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

s64 int_parse(s8* data, u64* size = NULL)

{
    // Todo: ignore leading whitespaces
    u64 value = 0;
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

f64 float_parse(s8* data, u64* size = NULL)
{
    // Todo: ignore leading whitespaces
    // Todo: int parser has duplicate code
    f64 value = 0;
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

v3 in_vertices[4096];
v3 in_normals[4096];
v2 in_uvs[4096];
u32 in_faces[4096*3];

u32 num_vertices;
u32 num_normals;
u32 num_uvs;
u32 num_faces;

void mesh_create(s8* path, mesh* mesh)
{
    u64 read_bytes = 0;

    load_file(path, file_data, MAX_FILE_SIZE, &read_bytes);

    s8* data = file_data;
    s8 str[255] = {0};

    for (u64 i = 0; i < read_bytes && *data != '\0'; i++)
    {
        u64 str_size = string_read(data, str, 255);

        if (str_compare(str, (s8*)"v"))
        {
            v3* v = &in_vertices[num_vertices++];

            fprintf(stderr, "v");
            
            data += str_size;
            str_size = string_read(data, str, 255);
            v->x = float_parse(str);
            fprintf(stderr, " %f", v->x);

            data += str_size;
            str_size = string_read(data, str, 255);
            v->y = float_parse(str);
            fprintf(stderr, " %f", v->y);

            data += str_size;
            str_size = string_read(data, str, 255);
            v->z = float_parse(str);
            fprintf(stderr, " %f\n", v->z);
        }
        else if (str_compare(str, (s8*)"vt"))
        {
            v2* uv = &in_uvs[num_uvs++];

            fprintf(stderr, "vt");            

            data += str_size;
            str_size = string_read(data, str, 255);
            uv->x = float_parse(str);
            fprintf(stderr, " %f", uv->x);

            data += str_size;
            str_size = string_read(data, str, 255);
            uv->y = float_parse(str);
            fprintf(stderr, " %f\n", uv->y);
        }
        else if (str_compare(str, (s8*)"vn"))
        {
            v3* n = &in_normals[num_normals++];

            fprintf(stderr, "vn");

            data += str_size;
            str_size = string_read(data, str, 255);
            n->x = float_parse(str);
            fprintf(stderr, " %f", n->x);

            data += str_size;
            str_size = string_read(data, str, 255);
            n->y = float_parse(str);
            fprintf(stderr, " %f", n->y);

            data += str_size;
            str_size = string_read(data, str, 255);
            n->z = float_parse(str);
            fprintf(stderr, " %f\n", n->z);
        }
        else if (str_compare(str, (s8*)"f"))
        {
            fprintf(stderr, "f");

            for (u32 i = 0; i < 3; i++)
            {
                u32* face = &in_faces[num_faces];

                data += str_size;
                str_size = string_read(data, str, 255);

                s8* s = str;

                u64 bytes_read = 0;
                face[0] = (s32)int_parse(s, &bytes_read); 
                fprintf(stderr, " %d", face[0]);
                s += bytes_read;
                fprintf(stderr, "%c", *s++);
                face[1] = (s32)int_parse(s, &bytes_read); 
                fprintf(stderr, "%d", face[1]);
                s += bytes_read;
                fprintf(stderr, "%c", *s++);
                face[2] = (s32)int_parse(s, &bytes_read); 
                fprintf(stderr, "%d", face[2]);

                num_faces += 3;
            }

            fprintf(stderr, "\n");
        }
        else
        {
            data += str_size;
        }
    }

    for (u32 i = 0; i < num_faces; i += 3)
    {
        u32* face = &in_faces[i];

        vertex v;

        v.position = in_vertices[face[0] - 1];
        v.uv = in_uvs[face[1] - 1];
        v.normal = in_normals[face[2] - 1];

        b32 found = false;

        for (u32 j = 0; j < mesh->num_vertices; j++)
        {
            vertex other = mesh->vertices[j];

            // Todo: fix this
            // if (v.position == other.position && v.uv == other.uv && v.normal == other.normal)
            // {
            //     mesh->indices[mesh->num_indices++] = j;

            //     found = true;
            //     break;
            // }
        }

        if (!found)
        {
            mesh->vertices[mesh->num_vertices++] = v;
            mesh->indices[mesh->num_indices++] = mesh->num_vertices - 1;
        }
    }

    num_vertices = 0;
    num_faces = 0;
    num_normals = 0;
    num_uvs = 0;

    generate_vertex_array(mesh);
}

u32 program_create(s8* vertex_shader_path, s8* fragment_shader_path)
{
    u64 read_bytes = 0;

    u32 result = 0;
    u32 program = glCreateProgram();
    u32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    u32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    assert(program);
    assert(vertex_shader);
    assert(fragment_shader);

    load_file(vertex_shader_path, file_data, MAX_FILE_SIZE, &read_bytes);

    const GLchar* temp = (const GLchar*)file_data;

    glShaderSource(vertex_shader, 1, &temp, 0);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, (GLint*)&result);
    assert(result);

    memset((void*)file_data, 0, MAX_FILE_SIZE);

    load_file(fragment_shader_path, file_data, MAX_FILE_SIZE, &read_bytes);
    
    temp = (const GLchar*)file_data;

    glShaderSource(fragment_shader, 1, &temp, 0);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, (GLint*)&result);
    assert(result);

    memset((void*)file_data, 0, MAX_FILE_SIZE);

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, (GLint*)&result);
    assert(result);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
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

    state.shader = program_create((s8*)"assets/shaders/vertex.glsl", (s8*)"assets/shaders/fragment.glsl");

    state.texture_tileset = texture_create((s8*)"assets/textures/tileset.tga");
    state.texture_sphere = texture_create((s8*)"assets/textures/sphere.tga");
    state.texture_player = texture_create((s8*)"assets/textures/cube.tga");
    state.texture_enemy = texture_create((s8*)"assets/textures/enemy.tga");

    state.screen_width = screen_width;
    state.screen_height = screen_height;
    state.perspective = perspective(60.0f, (f32)state.screen_width/(f32)state.screen_height, 0.1f, 100.0f);

    state.player.x = 7.0f;
    state.player.y = 6.0f;

    // state.cube.num_vertices = state.assets.cubeMesh->getVertices().size();
    // state.cube.num_indices = state.assets.cubeMesh->getIndices().size();
    // state.cube.vertices = state.assets.cubeMesh->getVertices().data();
    // state.cube.indices = state.assets.cubeMesh->getIndices().data();

    // generate_vertex_array(&state.cube);

    // state.sphere.num_vertices = state.assets.sphereMesh->getVertices().size();
    // state.sphere.num_indices = state.assets.sphereMesh->getIndices().size();
    // state.sphere.vertices = state.assets.sphereMesh->getVertices().data();
    // state.sphere.indices = state.assets.sphereMesh->getIndices().data();

    // generate_vertex_array(&state.sphere);

    // state.wall.num_vertices = state.assets.wallMesh->getVertices().size();
    // state.wall.num_indices = state.assets.wallMesh->getIndices().size();
    // state.wall.vertices = state.assets.wallMesh->getVertices().data();
    // state.wall.indices = state.assets.wallMesh->getIndices().data();

    // generate_vertex_array(&state.wall);

    mesh_create((s8*)"assets/meshes/cube.mesh", &state.cube);
    mesh_create((s8*)"assets/meshes/sphere.mesh", &state.sphere);
    mesh_create((s8*)"assets/meshes/wall.mesh", &state.wall);

    // s8 str[] = "testi";
    // s8 str2[] = "testi";

    // b32 cool = str_compare(str, str2);

    // fprintf(stderr, "%s and %s are %s\n", str, str2, cool ? "the same" : "not the same");

    // int_parse((s8*)"102900235");

    // float_parse((s8*)"-123.41234");
    // float_parse((s8*)"1243434.344423");
    // float_parse((s8*)"1.00000000");
    // float_parse((s8*)"-0.999999999");
    // float_parse((s8*)"-1.0");
    // float_parse((s8*)"0.0");
    // float_parse((s8*)"-1233");
    // float_parse((s8*)"24");

    // float_parse((s8*)"3.012e+01");
    // float_parse((s8*)"3.012e01");
    // float_parse((s8*)"3.012e1");
    // float_parse((s8*)"1.2e-3");
    // float_parse((s8*)"1.2e-0003");

    // s8* str1 = (s8*)"-10.135443 52.3445";
    // s8 str[32] = {0};

    // u64 bytes_read = string_read(str1, str);

    // fprintf(stderr, "%s\n", str);

    // float_parse(str);

    // memset(str, 0, 32);

    // str1 += bytes_read;

    // fprintf(stderr, "%llu\n", string_read(str1, str));

    // fprintf(stderr, "%s\n", str);

    // float_parse(str);

    while (state.num_enemies < MAX_ENEMIES)
    {
        game_enemy* enemy = &state.enemies[state.num_enemies++];
            
        enemy->x = 5.0f - state.num_enemies * 5.0f;
        enemy->y = 0.0f;
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

    state.view = look_at({state.player.x, state.player.y, 20.0f},
        {state.player.x, state.player.y, 0},
        {0, 1, 0});

    map_render();
    player_render();
    enemies_render();
    bullets_render();
}