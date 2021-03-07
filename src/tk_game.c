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
    b32 alive;
};

struct enemy
{
    struct v2 position;
    f32 angle;
    b32 alive;
};

struct camera
{
    struct m4 perspective;
    struct m4 view;
    struct m4 perspective_inverse;
    struct m4 view_inverse;
    struct v3 position;
    f32 screen_width;
    f32 screen_height;
};

struct mouse
{
    struct v2 world;
    struct v2 screen;
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

#define MAX_BULLETS 64
#define MAX_ENEMIES 4

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
    struct camera camera;
    struct mouse mouse;
    struct mesh cube;
    struct mesh sphere;
    struct mesh wall;
    struct mesh floor;
    struct memory_block temporary;
    b32 fired;
    f32 accumulator;
    u32 shader;
    u32 shader_simple;
    u32 texture_tileset;
    u32 texture_sphere;
    u32 texture_player;
    u32 texture_enemy;
    u32 free_bullet;
    u32 num_enemies;
    u32 level;
};

u32 MAP_WIDTH  = 20;
u32 MAP_HEIGHT = 20;

f32 PLAYER_ACCELERATION = 20.0f;
f32 PROJECTILE_RADIUS   = 0.035f;
f32 PROJECTILE_SPEED    = 25.0f;
f32 PLAYER_RADIUS       = 0.25f;
f32 WALL_SIZE           = 1.0f;

u32 TILE_NOTHING = 0;
u32 TILE_WALL    = 1;
u32 TILE_FLOOR   = 2;

struct v4 color_white = {{{ 1.0, 1.0, 1.0, 1.0 }}};
struct v4 color_black = {{{ 0.0, 0.0, 0.0, 0.0 }}};
struct v4 color_red   = {{{ 1.0, 0.0, 0.0, 0.0 }}}; 
struct v4 color_blue  = {{{ 0.0, 0.0, 1.0, 0.0 }}};
struct v4 color_grey  = {{{ 0.5, 0.5, 0.5, 0.0 }}};

u8 map_data[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 
    1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 0, 
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 1, 2, 1, 0, 
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 1, 0, 
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 1, 2, 1, 0, 
    1, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2, 1, 0, 
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 1, 2, 1, 0, 
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 1, 0, 
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 1, 2, 1, 0, 
    1, 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 0, 
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 
    1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 
    1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 
    1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 1, 
    1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 1, 
    1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
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

b32 collision_point_to_rect(f32 x, f32 y, f32 min_x, f32 max_x, f32 min_y, 
    f32 max_y)
{
    f32 result;

    result = (x >= min_x && x <= max_x && y >= min_y && y <= max_y);

    return result;
}

void map_render(struct game_state* state)
{
    // Todo: fix map rendering glitch (a wall block randomly drawn in a 
    //       wrong place)
    for (u32 y = 0; y < MAP_HEIGHT; y++)
    {
        for (u32 x = 0; x < MAP_WIDTH; x++)
        {
            u32 index = y * MAP_WIDTH + x;

            u8 tile = map_data[index];

            struct v4 color = color_white;

            f32 top = y + TILE_WALL * 0.5f;
            f32 bottom = y - TILE_WALL * 0.5f;
            f32 left = x - TILE_WALL * 0.5f;
            f32 right = x + TILE_WALL * 0.5f;

            if (collision_point_to_rect(state->mouse.world.x, 
                state->mouse.world.y, left, right, bottom, top))
            {
                color = color_red;
            }

            if (tile == TILE_WALL)
            {
                for (u32 i = 0; i < state->level; i++)
                {
                    struct m4 transform = m4_translate(x, y,
                        -1.0f * (state->level - i - 1));
                    struct m4 rotation = m4_rotate_z(0.0f);
                    struct m4 scale = m4_scale_all(WALL_SIZE * 0.5f);

                    struct m4 model = m4_mul_m4(scale, rotation);
                    model = m4_mul_m4(model, transform);

                    struct m4 mvp = m4_mul_m4(model, state->camera.view);
                    mvp = m4_mul_m4(mvp, state->camera.perspective);

                    mesh_render(&state->wall, &mvp, state->texture_tileset, 
                        state->shader, color);
                }
            }
            else if (tile == TILE_FLOOR)
            {
                struct m4 transform = m4_translate(x, y, -WALL_SIZE);
                struct m4 rotation = m4_rotate_z(0.0f);
                struct m4 scale = m4_scale_all(WALL_SIZE * 0.5f);

                struct m4 model = m4_mul_m4(scale, rotation);
                model = m4_mul_m4(model, transform);

                struct m4 mvp = m4_mul_m4(model, state->camera.view);
                mvp = m4_mul_m4(mvp, state->camera.perspective);

                mesh_render(&state->floor, &mvp, state->texture_tileset,
                    state->shader, color);
            }
        }
    }
}

b32 collision_circle_to_circle(struct v2 position_a, f32 radius_a,
    struct v2 position_b, f32 radius_b)
{
    f32 result = v2_distance(position_a, position_b) < (radius_a + radius_b);

    return result;
}

b32 collision_circle_to_rect(struct v2 circle, f32 circle_radius, f32 rect_top, 
    f32 rect_bottom, f32 rect_left, f32 rect_right, struct v2* col)
{
    f32 result;

    struct v2 collision_point = circle;

    if (circle.x < rect_left)
    {
        collision_point.x = rect_left;
    }
    else if (circle.x > rect_right)
    {
        collision_point.x = rect_right;
    }

    if (circle.y < rect_bottom)
    {
        collision_point.y = rect_bottom;
    }
    else if (circle.y > rect_top)
    {
        collision_point.y = rect_top;
    }

    result = collision_circle_to_circle(circle, circle_radius, collision_point,
         0.0f);

    if (result && col)
    {
        col->x = circle.x > 0 ? rect_right : rect_left;
        col->y = circle.y > 0 ? rect_top : rect_bottom;
    }

    return result;
}

b32 collision_wall_resolve(f32 wall_x, f32 pos_x, f32 pos_y, f32 move_delta_x, 
    f32 move_delta_y, f32 wall_size, f32* move_time)
{
    b32 result = false;

    if (move_delta_x != 0.0f)
    {
        f32 diff = wall_x - pos_x;
        f32 t = diff / move_delta_x;
        f32 y = pos_y + move_delta_y * t;

        if (y >= -wall_size && y <= wall_size)
        {
            if (t < *move_time && t >= 0.0f)
            {
                // Todo: what is the best amount for the epsilon?
                f32 epsilon = 0.01f;
                *move_time = MAX(0.0f, t - epsilon);

                result = true;
            }
        }
    }

    return result;
}

b32 collision_corner_resolve(struct v2 rel, struct v2 move_delta, f32 radius, 
    f32* move_time, struct v2* normal)
{
    b32 result = false;

    struct v2 rel_new = 
    {
        rel.x + move_delta.x,
        rel.y + move_delta.y
    };

    struct v2 col = { 0.0f };

    f32 wall_half = WALL_SIZE * 0.5f;

    if (collision_circle_to_rect(rel_new, radius, wall_half, -wall_half,
        -wall_half, wall_half, &col))
    {
        // 1. find the closest point on the line from 
        //    relative position - relative_position_new
        struct v2 plr_to_col = 
        { 
            col.x - rel.x,
            col.y - rel.y
        };

        struct v2 plr_to_new = 
        { 
            rel_new.x - rel.x,
            rel_new.y - rel.y
        };

        f32 dot = v2_dot(plr_to_col, plr_to_new);
        f32 length_squared = v2_length_squared(plr_to_new);
        f32 temp = dot / length_squared;

        struct v2 closest =
        {
            rel.x + plr_to_new.x * temp,
            rel.y + plr_to_new.y * temp
        };

        // 2. calculate distance from closest point to the perfect point
        f32 distance_closest_to_collision = v2_distance(closest, col);

        f32 distance_closest_to_perfect = f32_sqrt(f32_square(radius) - 
            f32_square(distance_closest_to_collision));

        // 3. calculate distance from relative point to perfect point
        f32 distance_closest_to_relative = v2_distance(closest, rel);

        f32 distance_relative_to_perfect = distance_closest_to_relative - 
            distance_closest_to_perfect;

        struct v2 velocity_direction = v2_normalize(move_delta);

        struct v2 perfect = 
        {
            rel.x + velocity_direction.x * distance_relative_to_perfect,
            rel.y + velocity_direction.y * distance_relative_to_perfect
        };

        // 4. calculate t and normal
        f32 distance_relative_to_new = v2_length(plr_to_new);
        f32 distance = v2_distance(rel, rel_new);

        f32 t = distance_relative_to_perfect / distance_relative_to_new;

        if (t < *move_time)
        {
            // Todo: what is the best amount for the epsilon?
            f32 epsilon = 0.01f;
            *move_time = MAX(0.0f, t - epsilon);

            normal->x = perfect.x - col.x;
            normal->y = perfect.y - col.y;

            *normal = v2_normalize(*normal);

            result = true;
        }
    }

    return result;
}

void enemies_update(struct game_state* state, struct game_input* input, f32 dt)
{

}

void enemies_render(struct game_state* state)
{
    for (u32 i = 0; i < MAX_ENEMIES; i++)
    {
        struct enemy* enemy = &state->enemies[i];

        if (enemy->alive)
        {
            struct m4 transform = m4_translate(enemy->position.x, 
                enemy->position.y, 0.0f);
            struct m4 rotation = m4_rotate_z(enemy->angle);
            struct m4 scale = m4_scale_xyz(PLAYER_RADIUS, PLAYER_RADIUS, 0.25f);

            struct m4 model = m4_mul_m4(scale, rotation);
            model = m4_mul_m4(model, transform);

            struct m4 mvp = m4_mul_m4(model, state->camera.view);
            mvp = m4_mul_m4(mvp, state->camera.perspective);

            mesh_render(&state->cube, &mvp, state->texture_enemy, state->shader,
                color_white);
        }
    }
}

b32 check_tile_collisions(struct v2* pos, struct v2* vel, struct v2 move_delta, 
    f32 radius, f32 bounce_factor)
{
    b32 result = false;

    f32 wall_low = WALL_SIZE * 0.5f;
    f32 wall_high = wall_low + radius;

    f32 margin_x = move_delta.x + radius;
    f32 margin_y = move_delta.y + radius;

    f32 min_x = pos->x + wall_low - margin_x - WALL_SIZE;
    f32 min_y = pos->y + wall_low - margin_y - WALL_SIZE;

    u32 start_x = min_x < 0.0f ? 0 : (u32)min_x / WALL_SIZE;
    u32 start_y = min_y < 0.0f ? 0 : (u32)min_y / WALL_SIZE;
    u32 end_x = (u32)((pos->x + wall_low + margin_x) / WALL_SIZE) + 1;
    u32 end_y = (u32)((pos->y + wall_low + margin_y) / WALL_SIZE) + 1;
    
    f32 time_remaining = 1.0f;

    for (u32 i = 0; i < 4 && time_remaining > 0.0f; i++)
    {
        f32 time = 1.0f;
        struct v2 normal = { 0.0f };
        
        for (u32 y = start_y; y <= end_y; y++)
        {
            for (u32 x = start_x; x <= end_x; x++)
            {
                if (map_data[y * MAP_WIDTH + x] != 1)
                {
                    continue;
                }

                struct v2 rel = 
                {
                    pos->x - x,
                    pos->y - y
                };

                struct v2 rel_new = 
                {
                    rel.x + move_delta.x,
                    rel.y + move_delta.y
                };

                if (collision_wall_resolve(-wall_high, rel.y, rel.x, 
                    move_delta.y, move_delta.x, wall_low, &time))
                {
                    normal.x = 0.0f;
                    normal.y = -1.0f;
                    result = true;
                }
                if (collision_wall_resolve(-wall_high, rel.x, rel.y, 
                    move_delta.x, move_delta.y, wall_low, &time))
                {
                    normal.x = -1.0f;
                    normal.y = 0.0f;
                    result = true;
                }
                if (collision_wall_resolve(wall_high, rel.y, rel.x, 
                    move_delta.y, move_delta.x, wall_low, &time))
                {
                    normal.x = 0.0f;
                    normal.y = 1.0f;
                    result = true;
                }
                if (collision_wall_resolve(wall_high, rel.x, rel.y, 
                    move_delta.x, move_delta.y, wall_low, &time))
                {
                    normal.x = 1.0f;
                    normal.y = 0.0f;
                    result = true;
                }

                if (v2_length(normal) == 0.0f)
                {
                    if (collision_corner_resolve(rel, move_delta, radius, &time, 
                        &normal))
                    {
                        result = true;
                    }
                }
            }
        }

        pos->x += move_delta.x * time;
        pos->y += move_delta.y * time;

        f32 vel_dot = bounce_factor * v2_dot(*vel, normal);

        vel->x -= vel_dot * normal.x;
        vel->y -= vel_dot * normal.y;

        f32 move_delta_dot = bounce_factor * v2_dot(move_delta, normal);

        move_delta.x -= move_delta_dot * normal.x;
        move_delta.y -= move_delta_dot * normal.y;

        time_remaining -= time * time_remaining;
    }

    return result;
}

void bullets_update(struct game_state* state, struct game_input* input, f32 dt)
{
    for (u32 i = 0; i < MAX_BULLETS; i++)
    {
        struct bullet* bullet = &state->bullets[i];

        if (bullet->alive)
        {
            struct v2 move_delta = 
            {
                bullet->velocity.x * dt, 
                bullet->velocity.y * dt
            };

            // Todo: projectiles sometimes get stuck in the wall instead of
            // bouncing
            if (check_tile_collisions(&bullet->position, &bullet->velocity, 
                    move_delta, PROJECTILE_RADIUS, 2))
            {
                bullet->alive = false;
            }

            for (u32 j = 0; j < state->num_enemies; j++)
            {
                struct enemy* enemy = &state->enemies[j];

                if (enemy->alive && collision_circle_to_circle(bullet->position, 
                    PROJECTILE_RADIUS, state->enemies[j].position,
                    PLAYER_RADIUS))
                {
                    bullet->alive = false;
                    enemy->alive = false;
                }
            }
        }
    }
}

void bullets_render(struct game_state* state)
{
    for (u32 i = 0; i < MAX_BULLETS; i++)
    {
        struct bullet* bullet = &state->bullets[i];

        if (bullet->alive)
        {
            struct m4 transform = m4_translate(bullet->position.x,  
                bullet->position.y, 0.0f);
            struct m4 rotation = m4_rotate_z(bullet->angle);
            struct m4 scale = m4_scale_all(PROJECTILE_RADIUS);

            struct m4 model = m4_mul_m4(scale, rotation);
            model = m4_mul_m4(model, transform);

            struct m4 mvp = m4_mul_m4(model, state->camera.view);
            mvp = m4_mul_m4(mvp, state->camera.perspective);

            mesh_render(&state->sphere, &mvp, state->texture_sphere,
                state->shader, color_white);
        }
    }
}

void player_update(struct game_state* state, struct game_input* input, f32 dt)
{
    struct v2 direction = { 0.0f };
    struct v2 acceleration = { 0.0f };
    struct v2 move_delta = { 0.0f };
    struct player* player = &state->player;

    struct v2 dir = { state->mouse.world.x - player->position.x, 
        state->mouse.world.y - player->position.y };
    dir = v2_normalize(dir);

    player->angle = f32_atan(dir.y, dir.x);

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

    f32 length = v2_length(direction);

    if (length > 1.0f)
    {
        direction.x /= length;
        direction.y /= length;
    }

    acceleration.x = direction.x * PLAYER_ACCELERATION;
    acceleration.y = direction.y * PLAYER_ACCELERATION;

    acceleration.x += -player->velocity.x * 5.0f;
    acceleration.y += -player->velocity.y * 5.0f;

    move_delta.x = 0.5f * acceleration.x * f32_square(dt) + player->velocity.x 
        * dt;
    move_delta.y = 0.5f * acceleration.y * f32_square(dt) + player->velocity.y 
        * dt;

    player->velocity.x = player->velocity.x + acceleration.x * dt;
    player->velocity.y = player->velocity.y + acceleration.y * dt;

    check_tile_collisions(&player->position, &player->velocity, move_delta,
        PLAYER_RADIUS, 1);

    if (input->shoot.key_down)
    {
        if (!state->fired)
        {
            if (++state->free_bullet == MAX_BULLETS)
            {
                state->free_bullet = 0;
            }

            struct bullet* bullet = &state->bullets[state->free_bullet];

            f32 speed = PROJECTILE_SPEED;

            bullet->position.x = player->position.x;
            bullet->position.y = player->position.y;
            bullet->velocity.x = dir.x * speed;
            bullet->velocity.y = dir.y * speed;
            bullet->alive = true;

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
    struct player* player = &state->player;
    struct m4 transform = m4_translate(player->position.x, player->position.y,
     0.0f);
    struct m4 rotation = m4_rotate_z(player->angle);
    struct m4 scale = m4_scale_xyz(PLAYER_RADIUS, PLAYER_RADIUS, 0.25f);

    struct m4 model = m4_mul_m4(scale, rotation);
    model = m4_mul_m4(model, transform);

    struct m4 mvp = m4_mul_m4(model, state->camera.view);
    mvp = m4_mul_m4(mvp, state->camera.perspective);

    mesh_render(&state->cube, &mvp, state->texture_player, state->shader,
        color_white);

    // Render velocity vector
    {
        f32 max_speed = 7.0f;
        f32 length = v2_length(player->velocity) / max_speed;
        f32 angle = f32_atan(player->velocity.x, player->velocity.y);

        transform = m4_translate(
            player->position.x + player->velocity.x / max_speed, 
            player->position.y + player->velocity.y / max_speed, -0.5f);

        rotation = m4_rotate_z(-angle);
        scale = m4_scale_xyz(0.05f, length, 0.01f);

        model = m4_mul_m4(scale, rotation);
        model = m4_mul_m4(model, transform);

        mvp = m4_mul_m4(model, state->camera.view);
        mvp = m4_mul_m4(mvp, state->camera.perspective);

        mesh_render(&state->floor, &mvp, state->texture_tileset, 
            state->shader_simple, color_grey);
    }

    // // Render aim vector
    {
        struct v2 vec = 
        { 
            state->mouse.world.x - player->position.x,
            state->mouse.world.y - player->position.y
        };

        f32 length = v2_length(vec) * 0.5f;
        f32 angle = f32_atan(vec.x, vec.y);

        struct v2 direction = v2_normalize(vec);

        transform = m4_translate(
            player->position.x + direction.x * length, 
            player->position.y + direction.y * length, 0.0f);
        rotation = m4_rotate_z(-angle);
        scale = m4_scale_xyz(0.01f, length, 0.01f);

        model = m4_mul_m4(scale, rotation);
        model = m4_mul_m4(model, transform);

        mvp = m4_mul_m4(model, state->camera.view);
        mvp = m4_mul_m4(mvp, state->camera.perspective);

        mesh_render(&state->floor, &mvp, state->texture_tileset, 
            state->shader_simple, color_red);
    }
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

void tga_decode(s8* input, u64 out_size, s8* output, u32* width, u32* height)
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

        state->shader_simple = program_create(&state->temporary,
            "assets/shaders/vertex.glsl",
            "assets/shaders/fragment_simple.glsl");

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

    state->camera.screen_width = init->screen_width;
    state->camera.screen_height = init->screen_height;
    state->camera.perspective = m4_perspective(60.0f, 
        (f32)state->camera.screen_width/(f32)state->camera.screen_height, 
        0.1f, 100.0f);
    state->camera.perspective_inverse = m4_inverse(state->camera.perspective);

    state->num_enemies = MAX_ENEMIES;

    for (u32 i = 0; i < state->num_enemies; i++)
    {
        struct enemy* enemy = &state->enemies[i];
            
        enemy->position.x = 2.0f + i * 2.0f;
        enemy->position.y = 8.0f;
        enemy->alive = true;
    }

    state->level = 6;

    state->player.position.x = 3.0f;
    state->player.position.y = 3.0f;

    state->mouse.world = state->player.position;

    struct v2 temp = 
    {
        state->mouse.world.x - state->player.position.x,
        state->mouse.world.y - state->player.position.y
    };

    state->camera.position.x = state->player.position.x + temp.x * 0.5f;
    state->camera.position.y = state->player.position.y + temp.y * 0.5f;
    state->camera.position.z = 10.0f;

    state->camera.view = m4_translate(-state->camera.position.x, 
        -state->camera.position.y, -state->camera.position.z);

    state->camera.view_inverse = m4_inverse(state->camera.view);

    glClearColor(0.2f, 0.65f, 0.4f, 0.0f);

    if (!memory->initialized)
    {
        LOG("game_init: end of init, memory not initalized!\n");
    }
}

struct v2 calculate_world_pos(f32 pos_x, f32 pos_y, struct camera* camera)
{
    struct v2 result = { 0.0f };

    struct v4 ndc = 
    {
        pos_x / (camera->screen_width * 0.5f) - 1.0f,
        (pos_y / (camera->screen_height * 0.5f) - 1.0f) * -1.0f
    };

    struct v4 clip = { ndc.x, ndc.y, -1.0f, 1.0f };
    struct v4 view = m4_mul_v4(camera->perspective_inverse, clip);
    view.z = -1.0f;
    view.w = 0.0f;

    struct v4 world = m4_mul_v4(camera->view_inverse, view);

    struct v3 temp = { world.x, world.y, world.z };
    temp = v3_normalize(temp);

    f32 c = f32_abs(camera->position.z / temp.z);

    result.x = temp.x * c + camera->position.x;
    result.y = temp.y * c + camera->position.y;

    return result;
}

struct v2 calculate_screen_pos(f32 pos_x, f32 pos_y, struct camera* camera)
{
    struct v2 result = { 0.0f };
    struct v4 world = { pos_x, pos_y, 0.0f, 1.0f };

    struct v4 view = m4_mul_v4(camera->view, world);
    struct v4 clip = m4_mul_v4(camera->perspective, view);
    struct v2 ndc = { clip.x / clip.z, clip.y / clip.z };

    result.x = (ndc.x + 1.0f) * (camera->screen_width * 0.5f);
    result.y = (ndc.y * -1.0f + 1.0f) * (camera->screen_height * 0.5f);

    return result;
}

void game_update(struct game_memory* memory, struct game_input* input)
{
    if (memory->initialized)
    {
        struct game_state* state = (struct game_state*)memory->base;
        struct camera* camera = &state->camera;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        f32 step = 1.0f / 120.0f;
        state->accumulator += input->delta_time;

        state->mouse.world = calculate_world_pos(input->mouse_x, 
            input->mouse_y, camera);
            
        while (state->accumulator >= step)
        {
            state->accumulator -= step;

            player_update(state, input, step);
            enemies_update(state, input, step);
            bullets_update(state, input, step);

            camera->position.x = state->player.position.x;
            camera->position.y = state->player.position.y - 5.0f;
            camera->position.z = 7.5f;

            struct v3 target = 
            {
                state->player.position.x,
                state->player.position.y,
                0.0f
            };

            struct v3 up = { 0.0f, 1.0f, 0.0f };

            camera->view = m4_look_at(camera->position, target, up);
            camera->view_inverse = m4_inverse(camera->view);

            state->mouse.world = calculate_world_pos(input->mouse_x, 
                input->mouse_y, camera);
        }

        map_render(state);
        player_render(state);
        enemies_render(state);
        bullets_render(state);
    }
    else
    {
        LOG("game_update: memory not initialized!\n");
    }
}
