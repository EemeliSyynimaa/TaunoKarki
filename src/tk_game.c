#include "tk_platform.h"
#include "tk_math.h"
#include "tk_state_interface.h"
#include "tk_random.h"
#include "tk_memory.h"

// Todo: global for now
struct api api;

#include "tk_mesh.c"
#include "tk_resources.c"
#include "tk_camera.c"

// Todo: clean this, not very nice
struct v4 colors[] =
{
    { 0.0,  0.0,  0.0,  1.0 },
    { 0.0,  0.0,  0.5,  1.0 },
    { 0.0,  0.0,  1.0,  1.0 },
    { 0.0,  0.5,  0.0,  1.0 },
    { 0.0,  0.5,  0.5,  1.0 },
    { 0.0,  1.0,  0.0,  1.0 },
    { 0.0,  1.0,  1.0,  1.0 },
    { 0.5,  0.0,  0.0,  1.0 },
    { 0.5,  0.0,  0.5,  1.0 },
    { 0.5,  0.5,  0.0,  1.0 },
    { 0.5,  0.5,  0.5,  1.0 },
    { 0.75, 0.75, 0.75, 1.0 },
    { 1.0,  0.0,  0.0,  1.0 },
    { 1.0,  0.0,  1.0,  1.0 },
    { 1.0,  1.0,  0.0,  1.0 },
    { 1.0,  1.0,  1.0,  1.0 }
};

enum
{
    BLACK,
    NAVY,
    BLUE,
    GREEN,
    TEAL,
    LIME,
    AQUA,
    MAROON,
    PURPLE,
    OLIVE,
    GREY,
    SILVER,
    RED,
    FUCHSIA,
    YELLOW,
    WHITE
};

// Todo: create mesh_renderer that can be configured to work with particles,
// cubes, sprites and more
#include "tk_primitive_renderer.c"
#include "tk_sprite_renderer.c"
#include "tk_cube_renderer.c"
#include "tk_particle.c"

#include "tk_physics.c"
#include "tk_level.c"
#include "tk_input.c"
#include "tk_collision.c"

// Todo: where to store these?
enum
{
    WEAPON_NONE = 0,
    WEAPON_SHOTGUN = 1,
    WEAPON_MACHINEGUN = 2,
    WEAPON_PISTOL = 3,
    WEAPON_COUNT = 4
};

struct weapon
{
    struct v2 position;
    struct v2 velocity;
    struct v2 direction;
    u32 type;
    u32 level;
    u32 ammo;
    u32 ammo_max;
    f32 last_shot;
    f32 fire_rate;
    f32 reload_time;
    f32 spread;
    f32 projectile_size;
    f32 projectile_speed;
    f32 projectile_damage;
    b32 fired;
    b32 reloading;
};

struct gun_shot
{
    struct v2 position;
    f32 volume; // Todo: change name
};

#include "tk_pathfind.c"
#include "tk_entity.c"
#include "tk_world.c"

#define MAX_BULLETS 64
#define MAX_ENEMIES 64
#define MAX_ITEMS 256
#define MAX_WALL_CORNERS 512
#define MAX_WALL_FACES 512
#define MAX_GUN_SHOTS 64

struct game_state
{
    struct physics_world world;
    struct player player;
    struct bullet bullets[MAX_BULLETS];
    struct enemy enemies[MAX_ENEMIES];
    struct item items[MAX_ITEMS];
    struct particle_line particle_lines[MAX_PARTICLES];
    struct camera camera;
    struct mouse mouse;
    struct mesh sphere;
    struct mesh wall;
    struct mesh floor;
    struct mesh triangle;
    struct memory_block stack;
    struct v2 wall_corners[MAX_WALL_CORNERS];
    struct line_segment wall_faces[MAX_WALL_FACES];
    struct cube_renderer cube_renderer;
    struct sprite_renderer sprite_renderer;
    struct particle_renderer particle_renderer;
    struct particle_system particle_system;
    struct gun_shot gun_shots[MAX_GUN_SHOTS];
    struct level level;
    struct level level_mask;
    struct collision_map cols;
    struct state_interface* state_current;

    struct mesh_render_info render_info_health_bar;
    struct mesh_render_info render_info_ammo_bar;
    struct mesh_render_info render_info_weapon_bar;

    b32 render_debug;
    b32 level_change;
    b32 level_cleared;
    b32 player_in_start_room;
    f32 accumulator;
    f32 level_clear_notify;
    u32 shader;
    u32 shader_simple;
    u32 shader_cube;
    u32 shader_sprite;
    u32 shader_particle;
    u32 texture_tileset;
    u32 texture_sphere;
    u32 texture_cube;
    u32 texture_sprite;
    u32 texture_particle;
    u32 free_bullet;
    u32 free_item;
    u32 free_particle_line;
    u32 free_particle_sphere;
    u32 num_enemies;
    u32 num_wall_corners;
    u32 num_wall_faces;
    u32 num_gun_shots;
    u32 ticks_per_second;
    u32 level_current;
    u32 random_seed;
};

f32 PLAYER_ACCELERATION = 40.0f;
f32 PLAYER_RADIUS       = 0.25f;
f32 PLAYER_HEALTH_MAX   = 100.0f;

f32 ENEMY_ACCELERATION           = 35.0f;
f32 ENEMY_LINE_OF_SIGHT_HALF     = F64_PI * 0.25f; // 45 degrees
f32 ENEMY_LINE_OF_SIGHT_DISTANCE = 8.0f;
f32 ENEMY_REACTION_TIME_MIN      = 0.25f;
f32 ENEMY_REACTION_TIME_MAX      = 0.75f;
f32 ENEMY_GUN_FIRE_HEAR_DISTANCE = 10.0f;
f32 ENEMY_HEALTH_MAX             = 100.0f;
f32 ENEMY_LOOK_AROUND_DELAY_MIN  = 0.5f;
f32 ENEMY_LOOK_AROUND_DELAY_MAX  = 2.0f;

f32 ITEM_RADIUS = 0.1;
f32 ITEM_ALIVE_TIME = 10.0f;
f32 ITEM_FLASH_TIME = 2.0f;
f32 ITEM_FLASH_SPEED = 0.125f;

f32 CAMERA_ACCELERATION = 15.0f;

enum
{
    ITEM_NONE = 0,
    ITEM_HEALTH = 1,
    ITEM_SHOTGUN = 2,
    ITEM_MACHINEGUN = 3,
    ITEM_PISTOL = 4,
    ITEM_WEAPON_LEVEL_UP = 5,
    ITEM_COUNT = 6
};

u32 ITEAM_HEALTH_AMOUNT = 25;

f32 PROJECTILE_RADIUS = 0.035f;
f32 PROJECTILE_SPEED  = 50.0f;

f32 FRICTION  = 10.0f;

u32 WEAPON_LEVEL_MAX = 10;

void collision_map_render(struct game_state* state, struct line_segment* cols,
    u32 num_cols)
{
    struct mesh_render_info info = { 0 };
    info.color = colors[RED];
    info.mesh = &state->floor;
    info.texture = state->texture_tileset;
    info.shader = state->shader_simple;

    for (u32 i = 0; i < num_cols; i++)
    {
        line_render(&info, cols[i].start, cols[i].end, WALL_SIZE + 0.01f,
            0.025f, state->camera.perspective, state->camera.view);
    }
}

void particle_line_create(struct game_state* state, struct v2 start,
    struct v2 end, struct v4 color_start, struct v4 color_end, f32 time)
{
    if (++state->free_particle_line == MAX_PARTICLES)
    {
        state->free_particle_line = 0;
    }

    struct particle_line* particle =
        &state->particle_lines[state->free_particle_line];

    *particle = (struct particle_line){ 0 };
    particle->start = start;
    particle->end = end;
    particle->color_start = color_start;
    particle->color_end = color_end;
    particle->alive = true;
    particle->time_start = time;
    particle->time_current = particle->time_start;
}

void bullet_create(struct game_state* state, struct v2 position,
    struct v2 start_velocity, struct v2 direction, f32 speed, f32 damage,
    b32 player_owned, f32 size)
{
    if (++state->free_bullet == MAX_BULLETS)
    {
        state->free_bullet = 0;
    }

    f32 color = f32_random(0.75f, 1.0f);

    struct bullet* bullet = &state->bullets[state->free_bullet];

    if (!bullet->alive)
    {
        bullet->body = rigid_body_get(&state->world);
    }

    bullet->body->bullet = true;
    bullet->body->position = position;
    bullet->body->velocity = start_velocity;
    bullet->body->velocity.x += direction.x * speed;
    bullet->body->velocity.y += direction.y * speed;
    bullet->alive = true;
    bullet->damage = damage;
    bullet->player_owned = player_owned;
    bullet->start = bullet->body->position;
    bullet->color = (struct v4){ color, color, color, 1.0f };

    // Todo: add a fancy particle effect here
}

struct item* item_create(struct game_state* state, struct v2 position, u32 type)
{
    struct item* result = NULL;

    if (type > ITEM_NONE && type < ITEM_COUNT)
    {
        if (++state->free_item == MAX_ITEMS)
        {
            state->free_item = 0;
        }

        result = &state->items[state->free_item];

        if (!result->alive)
        {
            result->body = rigid_body_get(&state->world);
        }

        result->body->position = position;
        result->alive = ITEM_ALIVE_TIME;
        result->type = type;

        result->cube.faces[0].texture = 4 + type - 1;

        u32 color = OLIVE;

        switch (type)
        {
            case ITEM_HEALTH:
            {
                color = WHITE;
            } break;
            case ITEM_WEAPON_LEVEL_UP:
            {
                color = LIME;
            } break;
        }

        for (u32 i = 0; i < 6; i++)
        {
            result->cube.faces[i].color = color;
        }
    }

    return result;
}

void gun_shot_register(struct game_state* state, struct v2 position, f32 volume)
{
    if (state->num_gun_shots < MAX_GUN_SHOTS)
    {
        struct gun_shot* shot = &state->gun_shots[state->num_gun_shots++];

        shot->position = position;
        shot->volume = volume;
    }
}

struct weapon weapon_create(u32 type)
{
    struct weapon weapon = { 0 };

    switch (type)
    {
        case WEAPON_PISTOL:
        {
            weapon.type = WEAPON_PISTOL;
            weapon.level = 1;
            weapon.last_shot = 0.0f;
            weapon.fired = false;
            weapon.ammo_max = weapon.ammo = 12;
            weapon.fire_rate = 0.0f;
            weapon.reload_time = 0.8f / weapon.ammo;
            weapon.reloading = false;
            weapon.spread = 0.0125f;
            weapon.projectile_size = PROJECTILE_RADIUS;
            weapon.projectile_speed = PROJECTILE_SPEED;
            weapon.projectile_damage = 20.0f;
        } break;
        case WEAPON_MACHINEGUN:
        {
            weapon.type = WEAPON_MACHINEGUN;
            weapon.level = 1;
            weapon.last_shot = 0.0f;
            weapon.fired = false;
            weapon.ammo_max = weapon.ammo = 40;
            weapon.fire_rate = 0.075f;
            weapon.reload_time = 1.1f / weapon.ammo;
            weapon.reloading = false;
            weapon.spread = 0.035f;
            weapon.projectile_size = PROJECTILE_RADIUS * 0.75f;
            weapon.projectile_speed = PROJECTILE_SPEED;
            weapon.projectile_damage = 10.0f;
        } break;
        case WEAPON_SHOTGUN:
        {
            weapon.type = WEAPON_SHOTGUN;
            weapon.level = 1;
            weapon.last_shot = 0.0f;
            weapon.fired = false;
            weapon.ammo_max = weapon.ammo = 8;
            weapon.fire_rate = 0.5f;
            weapon.reload_time = 3.0f / weapon.ammo;
            weapon.reloading = false;
            weapon.spread = 0.125f;
            weapon.projectile_size = PROJECTILE_RADIUS * 0.25f;
            weapon.projectile_speed = PROJECTILE_SPEED;
            weapon.projectile_damage = 7.5f;
        } break;
    }

    return weapon;
}

void weapon_shoot(struct game_state* state, struct weapon* weapon, bool player)
{
    if (weapon->type == WEAPON_PISTOL)
    {
        if (weapon->last_shot <= 0.0f && weapon->ammo && !weapon->fired &&
            !weapon->reloading)
        {
            struct v2 randomized = v2_rotate(weapon->direction,
                f32_random(-weapon->spread, weapon->spread));

            bullet_create(state, weapon->position, weapon->velocity, randomized,
                weapon->projectile_speed, weapon->projectile_damage, player,
                weapon->projectile_size);

            weapon->fired = true;

            if (--weapon->ammo == 0)
            {
                weapon->reloading = true;
                weapon->last_shot = weapon->reload_time;
            }
            else
            {
                weapon->last_shot = weapon->fire_rate;
            }

            gun_shot_register(state, weapon->position, 10.0f);
        }
    }
    else if (weapon->type == WEAPON_MACHINEGUN)
    {
        if (weapon->last_shot <= 0.0f && weapon->ammo && !weapon->fired &&
            !weapon->reloading)
        {
            struct v2 randomized = v2_rotate(weapon->direction,
                f32_random(-weapon->spread, weapon->spread));

            bullet_create(state, weapon->position, weapon->velocity, randomized,
                weapon->projectile_speed, weapon->projectile_damage, player,
                weapon->projectile_size);

            if (--weapon->ammo == 0)
            {
                weapon->reloading = true;
                weapon->last_shot = weapon->reload_time;
            }
            else
            {
                weapon->last_shot = weapon->fire_rate;
            }

            gun_shot_register(state, weapon->position, 10.0f);
        }
    }
    else if (weapon->type == WEAPON_SHOTGUN)
    {
        // Todo: shotgun should be able to interrupt reloading and shoot
        // automatically after the next shell is loaded
        if (weapon->ammo && !weapon->fired && weapon->last_shot <= 0.0f)
        {
            weapon->reloading = false;

            for (u32 i = 0; i < 10; i++)
            {
                struct v2 randomized = v2_rotate(weapon->direction,
                    f32_random(-weapon->spread, weapon->spread));

                f32 speed = f32_random(
                    0.75f * weapon->projectile_speed, weapon->projectile_speed);

                bullet_create(state, weapon->position, weapon->velocity,
                    randomized, speed, weapon->projectile_damage, player,
                    weapon->projectile_size);
            }

            weapon->fired = true;

            if (--weapon->ammo == 0)
            {
                weapon->reloading = true;
                weapon->last_shot = weapon->reload_time;
            }
            else
            {
                weapon->last_shot = weapon->fire_rate;
            }

            gun_shot_register(state, weapon->position, 10.0f);
        }
    }
}

void weapon_reload(struct weapon* weapon, b32 full_reload)
{
    if (weapon->type == WEAPON_PISTOL)
    {
        weapon->ammo = 0;
        weapon->reloading = true;
        weapon->last_shot = weapon->reload_time;
    }
    else if (weapon->type == WEAPON_MACHINEGUN)
    {
        weapon->ammo = 0;
        weapon->reloading = true;
        weapon->last_shot = weapon->reload_time;
    }
    else if (weapon->type == WEAPON_SHOTGUN)
    {
        weapon->ammo = full_reload ? 0 : weapon->ammo;

        if (weapon->ammo < weapon->ammo_max)
        {
            weapon->reloading = true;
            weapon->last_shot = weapon->reload_time;
        }
    }
}

b32 weapon_level_up(struct weapon* weapon)
{
    b32 result = false;

    if (weapon->level < WEAPON_LEVEL_MAX)
    {
        weapon->level++;

        weapon->ammo_max = (u32)(weapon->ammo_max * 1.1f);
        weapon->projectile_damage *= 1.1f;
        weapon->reload_time *= 0.9f;
        weapon->fire_rate *= 0.9f;
        result = true;
    }

    return result;
}

b32 enemy_sees_player(struct collision_map* cols, struct enemy* enemy,
    struct player* player)
{
    b32 result = false;

    // Todo: maybe use direction_in_range() here instead
    struct v2 direction_player = v2_direction(enemy->body->position,
        player->body->position);
    struct v2 direction_current = v2_direction_from_angle(
        enemy->body->angle);

    f32 angle_player = v2_angle(direction_player, direction_current);

    // Todo: enemy AI goes nuts if two enemies are on top of each other and all
    // collisions are checked. Check collision against static (walls) and player
    // for now
    f32 player_ray_cast = ray_cast_body(cols, enemy->eye_position,
        player->body, NULL, COLLISION_STATIC | COLLISION_PLAYER);

    result = enemy->state != ENEMY_STATE_SLEEP && player->alive &&
        angle_player < ENEMY_LINE_OF_SIGHT_HALF && player_ray_cast > 0.0f &&
        player_ray_cast < ENEMY_LINE_OF_SIGHT_DISTANCE;

    return result;
}

b32 enemy_hears_gun_shot(struct game_state* state, struct enemy* enemy)
{
    b32 result = false;
    f32 closest = ENEMY_GUN_FIRE_HEAR_DISTANCE;

    for (u32 i = 0; i < state->num_gun_shots; i++)
    {
        struct gun_shot* shot = &state->gun_shots[i];

        f32 distance = v2_distance(enemy->body->position, shot->position);

        if (distance < closest && distance < shot->volume)
        {
            enemy->gun_shot_position = shot->position;
            closest = distance;
            result = true;
        }
    }

    return result;
}

f32 enemy_reaction_time_get(u32 state)
{
    f32 result = 0.0f;
    f32 reaction_multiplier = 1.0f;

    switch (state)
    {
        case ENEMY_STATE_SHOOT:
        case ENEMY_STATE_RUSH_TO_TARGET:
        case ENEMY_STATE_REACT_TO_PLAYER_SEEN:
        case ENEMY_STATE_REACT_TO_BEING_SHOT_AT:
        case ENEMY_STATE_REACT_TO_GUN_SHOT:
        {
            // Lower reaction time
            reaction_multiplier = 0.5f;
        } break;
        case ENEMY_STATE_SLEEP:
        {
            // Higher reaction time
            reaction_multiplier = 2.0f;
        } break;
        default:
        {
            // Normal reaction time
            reaction_multiplier = 1.0f;
        } break;
    }

    result = f32_random(ENEMY_REACTION_TIME_MIN, ENEMY_REACTION_TIME_MAX) *
        reaction_multiplier;

    return result;
}

f32 turn_amount_calculate(f32 angle_from, f32 angle_to)
{
    f32 circle = F64_PI * 2.0f;
    f32 result = 0.0f;

    while (angle_to < 0.0f)
    {
        angle_to += circle;
    }

    while (angle_from < 0.0f)
    {
        angle_from += circle;
    }

    f32 diff_clockwise = 0.0f;
    f32 diff_counter_clockwise = 0.0f;

    if (angle_to < angle_from)
    {
        diff_clockwise = angle_to + circle - angle_from;
        diff_counter_clockwise = angle_from - angle_to;
    }
    else
    {
        diff_clockwise = angle_to - angle_from;
        diff_counter_clockwise = angle_from + circle -
            angle_to;
    }

    result = MIN(diff_clockwise, diff_counter_clockwise);
    result *= diff_clockwise > diff_counter_clockwise ? 1.0f : -1.0f;

    return result;
}

void enemy_look_towards_angle(struct enemy* enemy, f32 angle)
{
    enemy->turn_amount = turn_amount_calculate(enemy->body->angle, angle);
}

void enemy_look_towards_direction(struct enemy* enemy, struct v2 direction)
{
    enemy_look_towards_angle(enemy, f32_atan(direction.y, direction.x));
}

void enemy_look_towards_position(struct enemy* enemy, struct v2 position)
{
    struct v2 direction = v2_direction(enemy->body->position, position);

    enemy_look_towards_angle(enemy, f32_atan(direction.y, direction.x));
}

void enemy_calculate_path_to_target(struct collision_map* cols,
    struct level* level, struct enemy* enemy)
{
    enemy->path_length = path_find(level, enemy->body->position,
        enemy->target, enemy->path, MAX_PATH);
    path_trim(cols, enemy->body->position, enemy->path, &enemy->path_length);
    enemy_look_towards_position(enemy, enemy->path[0]);
    enemy->path_index = 0;
}

f32 enemy_turn_speed_get(struct enemy* enemy)
{
    f32 result = 0.0f;
    f32 speed_multiplier = 1.0f;

    switch (enemy->state)
    {
        case ENEMY_STATE_SHOOT:
        case ENEMY_STATE_RUSH_TO_TARGET:
        case ENEMY_STATE_REACT_TO_PLAYER_SEEN:
        case ENEMY_STATE_REACT_TO_BEING_SHOT_AT:
        case ENEMY_STATE_REACT_TO_GUN_SHOT:
        case ENEMY_STATE_LOOK_FOR_PLAYER:
        {
            // Fast turning
            speed_multiplier = 1.5f;
        } break;
        case ENEMY_STATE_SLEEP:
        {
            // Don't turn when sleeping
            speed_multiplier = 0.0f;
        } break;
        case ENEMY_STATE_LOOK_AROUND:
        case ENEMY_STATE_WANDER_AROUND:
        {
            // Chilling speed
            speed_multiplier = 0.5f;
        } break;
        default:
        {
            // Normal turn speed
            speed_multiplier = 1.0f;
        } break;
    };

    // One full turn per second * multiplier
    result = (f32)F64_PI * 2.0f * speed_multiplier;

    return result;
}

f32 enemy_look_around_delay_get(struct enemy* enemy)
{
    f32 result = f32_random(ENEMY_LOOK_AROUND_DELAY_MIN,
        ENEMY_LOOK_AROUND_DELAY_MAX);

    if (enemy->state == ENEMY_STATE_LOOK_FOR_PLAYER)
    {
        result *= 0.5f;
    }

    return result;
}

void enemy_state_transition(struct game_state* state, struct enemy* enemy,
    u32 state_new)
{
    if (state_new == enemy->state)
    {
        return;
    }

    u32 state_old = enemy->state;

    enemy->state_timer = 0.0f;
    enemy->path_length = 0;
    enemy->state = state_new;

    struct level* level = &state->level;

    // LOG("Enemy transition from %s to %s\n", enemy_state_str[state_old],
    //     enemy_state_str[state_new]);

    switch (state_new)
    {
        case ENEMY_STATE_REACT_TO_PLAYER_SEEN:
        {
            enemy->acceleration = 0.0f;
            enemy->state_timer = enemy_reaction_time_get(state_old);
        } break;
        case ENEMY_STATE_REACT_TO_GUN_SHOT:
        {
            enemy->acceleration = 0.0f;
            enemy->state_timer = enemy_reaction_time_get(state_old);
            enemy_look_towards_position(enemy, enemy->gun_shot_position);
        } break;
        case ENEMY_STATE_RUSH_TO_TARGET:
        {
            enemy->acceleration = ENEMY_ACCELERATION;
            enemy_calculate_path_to_target(&state->cols, level, enemy);
        } break;
        case ENEMY_STATE_WANDER_AROUND:
        {
            enemy->acceleration = ENEMY_ACCELERATION * 0.5f;
            enemy->target = tile_random_get(level, TILE_FLOOR);
            enemy_calculate_path_to_target(&state->cols, level, enemy);
        } break;
        case ENEMY_STATE_LOOK_AROUND:
        {
            enemy->acceleration = 0.0f;
            enemy->turns_left = 3;
        } break;
        case ENEMY_STATE_REACT_TO_BEING_SHOT_AT:
        {
            enemy_look_towards_direction(enemy, enemy->hit_direction);
            enemy->state_timer = enemy_reaction_time_get(state_old);
        } break;
        case ENEMY_STATE_LOOK_FOR_PLAYER:
        {
            enemy->acceleration = 0.0f;
            enemy->turns_left = 5;
        } break;
    }
}

void enemies_update(struct game_state* state, struct game_input* input, f32 dt)
{
    // Todo: clean this function...
    for (u32 i = 0; i < MAX_ENEMIES; i++)
    {
        struct enemy* enemy = &state->enemies[i];

        if (enemy->alive)
        {
            if (enemy->health < 0.0f)
            {
                enemy->alive = false;

                rigid_body_free(enemy->body);

                {
                    u32 count = 0;

                    for (u32 i = 0; i < state->num_enemies; i++)
                    {
                        count += state->enemies[i].alive;
                    }

                    if (count)
                    {
                        LOG("Enemies left %u of %u\n", count,
                            state->num_enemies);
                    }
                    else
                    {
                        state->level_cleared = true;
                        state->level_clear_notify = 2.5f;
                    }
                }

                item_create(state, enemy->body->position,
                    ITEM_HEALTH + enemy->weapon.type);

                u32 random_item_count = u32_random(0, 3);

                for (u32 j = 0; j < random_item_count; j++)
                {
                    struct v2 position = enemy->body->position;

                    f32 offset_max = 0.25f;
                    f32 offset_x = f32_random(-offset_max, offset_max);
                    f32 offset_y = f32_random(-offset_max, offset_max);

                    position.x += offset_x;
                    position.y += offset_y;

                    u32 random_item_type = u32_random(0, 2);
                    random_item_type =
                        random_item_type ? ITEM_HEALTH : ITEM_WEAPON_LEVEL_UP;

                    item_create(state, position, random_item_type);
                }

                continue;
            }

            enemy->player_in_view = enemy_sees_player(&state->cols, enemy,
                &state->player);

            enemy->gun_shot_heard = enemy_hears_gun_shot(state, enemy);

            if (enemy->player_in_view)
            {
                enemy->player_last_seen_position = state->player.body->position;
                enemy->player_last_seen_direction = v2_normalize(
                    state->player.body->velocity);

                if (enemy->state != ENEMY_STATE_SHOOT &&
                    enemy->state != ENEMY_STATE_SLEEP)
                {
                    enemy_state_transition(state, enemy,
                        ENEMY_STATE_REACT_TO_PLAYER_SEEN);
                }
            }
            else if (enemy->gun_shot_heard)
            {
                // Todo: enemy should react only to the closest source of noise
                if (enemy->state == ENEMY_STATE_SLEEP)
                {
                    enemy_state_transition(state, enemy,
                        ENEMY_STATE_LOOK_FOR_PLAYER);
                }
                else if (enemy->state == ENEMY_STATE_RUSH_TO_TARGET)
                {
                    enemy->target = enemy->gun_shot_position;
                    enemy_calculate_path_to_target(&state->cols, &state->level,
                        enemy);
                }
                else if (enemy->state != ENEMY_STATE_SHOOT)
                {
                    enemy_state_transition(state, enemy,
                        ENEMY_STATE_REACT_TO_GUN_SHOT);
                }
            }
            else if (enemy->got_hit)
            {
                // Todo: react only if the source of bullet is not visible?
                if (enemy->state == ENEMY_STATE_SLEEP)
                {
                    enemy_state_transition(state, enemy,
                        ENEMY_STATE_LOOK_FOR_PLAYER);
                }
                else if (enemy->state != ENEMY_STATE_SHOOT)
                {
                    enemy_state_transition(state, enemy,
                        ENEMY_STATE_REACT_TO_BEING_SHOT_AT);
                }

                enemy->got_hit = false;
            }

            switch (enemy->state)
            {
                case ENEMY_STATE_SLEEP:
                {
                } break;
                case ENEMY_STATE_REACT_TO_PLAYER_SEEN:
                {
                    if (enemy->state_timer_finished)
                    {
                        if (enemy->player_in_view)
                        {
                            enemy_state_transition(state, enemy,
                                ENEMY_STATE_SHOOT);
                        }
                        else
                        {
                            enemy->target = enemy->player_last_seen_position;
                            enemy_state_transition(state, enemy,
                                ENEMY_STATE_RUSH_TO_TARGET);
                        }
                    }
                    else if (enemy->player_in_view)
                    {
                        enemy_look_towards_position(enemy,
                            state->player.body->position);
                    }
                } break;
                case ENEMY_STATE_REACT_TO_GUN_SHOT:
                {
                    if (enemy->state_timer_finished)
                    {
                        if (ray_cast_position(&state->cols, enemy->eye_position,
                            enemy->gun_shot_position, NULL,
                            COLLISION_STATIC | COLLISION_PLAYER))
                        {
                            enemy_state_transition(state, enemy,
                                ENEMY_STATE_LOOK_FOR_PLAYER);
                        }
                        else
                        {
                            enemy->target = enemy->gun_shot_position;
                            enemy_state_transition(state, enemy,
                                ENEMY_STATE_RUSH_TO_TARGET);

                            // Here we can skip the last node since we only need
                            // to see the location where gun shot occured
                            if (enemy->path_length > 0)
                            {
                                enemy->path_length--;
                            }
                        }
                    }
                } break;
                case ENEMY_STATE_SHOOT:
                {
                    enemy->acceleration = 0.0f;

                    if (enemy->player_in_view)
                    {
                        enemy->player_last_seen_position =
                            state->player.body->position;
                        enemy->player_last_seen_direction = v2_normalize(
                            state->player.body->velocity);

                        struct v2 target_forward = v2_direction(
                            enemy->body->position,
                            enemy->player_last_seen_position);

                        if (v2_length(enemy->body->velocity))
                        {
                            // Todo: calculate right with cross product
                            f32 angle = F64_PI*1.5f;
                            f32 tsin = f32_sin(angle);
                            f32 tcos = f32_cos(angle);

                            struct v2 target_right =
                            {
                                target_forward.x * tcos - target_forward.y *
                                    tsin,
                                target_forward.x * tsin + target_forward.y *
                                    tcos
                            };

                            f32 initial_velocity = v2_dot(enemy->body->velocity,
                                target_right);

                            struct v2 desired_velocity = { 0.0f };

                            if (PROJECTILE_SPEED > initial_velocity)
                            {
                                desired_velocity.x = -initial_velocity;
                            }
                            else
                            {
                                desired_velocity.x = PROJECTILE_SPEED;
                            }

                            if (desired_velocity.x)
                            {
                                desired_velocity.y = f32_sqrt(
                                    (PROJECTILE_SPEED * PROJECTILE_SPEED) -
                                    (desired_velocity.x * desired_velocity.x));
                            }
                            else
                            {
                                desired_velocity.y = -PROJECTILE_SPEED;
                            }

                            struct v2 target_right_rotate_back =
                            {
                                target_right.x, -target_right.y
                            };

                            struct v2 target_forward_rotate_back =
                            {
                                -target_forward.x, target_forward.y
                            };

                            struct v2 final_direction =
                            {
                                v2_dot(desired_velocity,
                                    target_right_rotate_back),
                                v2_dot(desired_velocity,
                                    target_forward_rotate_back)
                            };

                            enemy->direction_aim =
                                v2_normalize(final_direction);
                        }
                        else
                        {
                            enemy->direction_aim = target_forward;
                        }

                        enemy_look_towards_direction(enemy, target_forward);

                        struct weapon* weapon = &enemy->weapon;

                        weapon->direction = enemy->direction_aim;
                        weapon->position = enemy->eye_position;
                        weapon->velocity = enemy->body->velocity;

                        weapon_shoot(state, weapon, false);

                        if (weapon->fired && enemy->trigger_release <= 0.0f)
                        {
                            enemy->trigger_release = 0.5f;
                        }
                    }
                    else if (!state->player.alive)
                    {
                        enemy_state_transition(state, enemy,
                            ENEMY_STATE_LOOK_FOR_PLAYER);
                    }
                    else
                    {
                        enemy_state_transition(state, enemy,
                            ENEMY_STATE_REACT_TO_PLAYER_SEEN);
                    }
                } break;
                case ENEMY_STATE_RUSH_TO_TARGET:
                {
                    if (!enemy->path_length)
                    {
                        enemy_look_towards_position(enemy, enemy->target);
                        enemy_state_transition(state, enemy,
                            ENEMY_STATE_LOOK_FOR_PLAYER);
                    }
                } break;
                case ENEMY_STATE_LOOK_AROUND:
                {
                    if (enemy->state_timer_finished)
                    {
                        if (enemy->turns_left)
                        {
                            f32 diff = f32_random(-F64_PI, F64_PI);
                            f32 angle_new = enemy->body->angle + diff;

                            enemy_look_towards_angle(enemy, angle_new);

                            enemy->turns_left--;
                        }
                        else
                        {
                            enemy_state_transition(state, enemy,
                                ENEMY_STATE_WANDER_AROUND);
                        }
                    }
                    else if (enemy->state_timer <= 0.0f && !enemy->turn_amount)
                    {
                        enemy->state_timer = enemy_look_around_delay_get(enemy);
                    }
                } break;
                case ENEMY_STATE_WANDER_AROUND:
                {
                    if (!enemy->path_length)
                    {
                        enemy_state_transition(state, enemy,
                            ENEMY_STATE_LOOK_AROUND);
                    }
                } break;
                case ENEMY_STATE_REACT_TO_BEING_SHOT_AT:
                {
                    if (enemy->state_timer_finished)
                    {
                        f32 length = ray_cast_direction(&state->cols,
                            enemy->eye_position, enemy->hit_direction, NULL,
                            COLLISION_STATIC);

                        // Skip one tile so we don't find a path into a wall
                        length = MAX(0, length - 1.0f);

                        enemy->target = v2_add(enemy->eye_position,
                            v2_mul_f32(enemy->hit_direction, length));

                        enemy_state_transition(state, enemy,
                            ENEMY_STATE_RUSH_TO_TARGET);

                        enemy->hit_direction = v2_zero;
                    }
                } break;
                case ENEMY_STATE_LOOK_FOR_PLAYER:
                {
                    if (!v2_is_zero(enemy->player_last_seen_direction))
                    {
                        enemy_look_towards_direction(enemy,
                                enemy->player_last_seen_direction);
                        enemy->player_last_seen_direction = v2_zero;
                        enemy->state_timer = enemy_look_around_delay_get(enemy);
                    }
                    else
                    {
                        if (enemy->state_timer_finished)
                        {
                            if (enemy->turns_left)
                            {
                                f32 diff = f32_random(
                                    -F64_PI, F64_PI);
                                f32 angle_new = enemy->body->angle + diff;

                                enemy_look_towards_angle(enemy, angle_new);

                                enemy->turns_left--;
                            }
                            else
                            {
                                enemy_state_transition(state, enemy,
                                    ENEMY_STATE_WANDER_AROUND);
                            }
                        }
                        else if (enemy->state_timer <= 0.0f &&
                            !enemy->turn_amount)
                        {
                            enemy->state_timer =
                                enemy_look_around_delay_get(enemy);
                        }
                    }
                } break;
            }

            enemy->state_timer_finished = false;

            if (enemy->state_timer > 0.0f)
            {
                enemy->state_timer -= dt;

                if (enemy->state_timer < 0.0f)
                {
                    enemy->state_timer_finished = true;
                }
            }

            if (enemy->path_length)
            {
                struct v2 current = enemy->path[enemy->path_index];

                f32 distance_to_current = v2_distance(enemy->body->position,
                    current);
                f32 epsilon = 0.25f;

                if (distance_to_current < epsilon)
                {
                    if (++enemy->path_index >= enemy->path_length)
                    {
                        enemy->path_length = 0;
                        enemy->acceleration = 0.0f;
                    }
                    else
                    {
                        enemy_look_towards_position(enemy,
                            enemy->path[enemy->path_index]);
                    }
                }
                else
                {
                    struct v2 direction = v2_normalize(v2_direction(
                        enemy->body->position, current));

                    f32 length = v2_length(direction);

                    if (length > 1.0f)
                    {
                        direction.x /= length;
                        direction.y /= length;
                    }

                    enemy->direction_move = direction;
                    enemy->direction_aim = direction;
                }
            }

            enemy->body->acceleration = v2_mul_f32(enemy->direction_move,
                enemy->acceleration);

            if (enemy->turn_amount)
            {
                f32 speed = enemy_turn_speed_get(enemy) * dt;
                f32 remaining = MIN(f32_abs(enemy->turn_amount), speed);

                if (enemy->turn_amount > 0)
                {
                    enemy->body->angle -= remaining;
                    enemy->turn_amount = MAX(enemy->turn_amount - remaining, 0);
                }
                else
                {
                    enemy->body->angle += remaining;
                    enemy->turn_amount = MIN(enemy->turn_amount + remaining, 0);
                }
            }

            struct v2 eye = { PLAYER_RADIUS + 0.0001f, 0.0f };

            enemy->eye_position = v2_rotate(eye, enemy->body->angle);
            enemy->eye_position.x += enemy->body->position.x;
            enemy->eye_position.y += enemy->body->position.y;

            {
                f32 vision_cone_update_speed = 3.0f;
                f32 vision_cone_size_min = 0.25f;
                f32 vision_cone_size_max = 1.0f;

                if (enemy->player_in_view)
                {
                    enemy->vision_cone_size -= dt * vision_cone_update_speed;
                    enemy->vision_cone_size = MAX(enemy->vision_cone_size,
                        vision_cone_size_min);
                }
                else
                {
                    enemy->vision_cone_size += dt * vision_cone_update_speed;
                    enemy->vision_cone_size = MIN(enemy->vision_cone_size,
                        vision_cone_size_max);
                }
            }

            // Todo: clean this, maybe move somewhere else
            {
                struct weapon* weapon = &enemy->weapon;

                if (enemy->trigger_release > 0.0f)
                {
                    enemy->trigger_release -= dt;

                    if (enemy->trigger_release <= 0.0f)
                    {
                        weapon->fired = false;
                    }
                }

                if (weapon->last_shot > 0.0f)
                {
                    weapon->last_shot -= dt;
                }

                if (weapon->reloading)
                {
                    if (weapon->last_shot <= 0.0f)
                    {
                        weapon->ammo++;

                        if (weapon->ammo == weapon->ammo_max)
                        {
                            weapon->reloading = false;
                        }
                        else
                        {
                            weapon->last_shot = weapon->reload_time;
                        }
                    }
                }
            }
        }
    }
}

b32 insert_corner(struct v2 corner, struct v2 corners[], u32 max, u32* count)
{
    b32 result = false;

    if (*count < max)
    {
        b32 found = false;

        for (u32 i = 0; i < *count; i++)
        {
            if (v2_equals(corner, corners[i]))
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            corners[(*count)++] = corner;
        }

        result = true;
    }

    return result;
}

void get_wall_corners(struct level* level, struct v2 corners[], u32 max,
    u32* count)
{
    for (u32 y = 0; y < level->height; y++)
    {
        for (u32 x = 0; x < level->width; x++)
        {
            struct v2 tile = { (f32)x, (f32)y };

            if (tile_is_of_type(level, tile, TILE_WALL))
            {
                f32 t = WALL_SIZE * 0.5f;

                if (!insert_corner((struct v2){ tile.x + t, tile.y + t},
                    corners, max, count))
                {
                    return;
                }

                if (!insert_corner((struct v2){ tile.x - t, tile.y - t},
                    corners, max, count))
                {
                    return;
                }

                if (!insert_corner((struct v2){ tile.x + t, tile.y - t},
                    corners, max, count))
                {
                    return;
                }

                if (!insert_corner((struct v2){ tile.x - t, tile.y + t},
                    corners, max, count))
                {
                    return;
                }
            }
        }
    }
}

b32 line_segment_equals(struct line_segment a, struct line_segment b)
{
    b32 result = v2_equals(a.start, b.start) && v2_equals(a.end, b.end);

    return result;
}

b32 line_segment_empty(struct line_segment a)
{
    b32 result = a.start.x == 0.0f && a.start.y == 0.0f && a.end.x == 0.0f &&
        a.end.y == 0.0f;

    return result;
}

b32 insert_face(struct line_segment* face, struct line_segment faces[], u32 max,
    u32* count)
{
    b32 result = false;

    if (line_segment_empty(*face))
    {
        result = true;
    }
    else if (*count < max)
    {
        b32 found = false;

        for (u32 i = 0; i < *count; i++)
        {
            if (line_segment_equals(*face, faces[i]))
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            faces[(*count)++] = *face;
            *face = (struct line_segment) { 0.0f };
        }

        result = true;
    }

    return result;
}

void collision_map_static_calculate(struct level* level,
    struct line_segment faces[], u32 max, u32* count)
{
    for (u32 y = 0; y < level->height; y++)
    {
        struct line_segment face_top    = { 0.0f };
        struct line_segment face_bottom = { 0.0f };

        for (u32 x = 0; x <= level->width; x++)
        {
            struct v2 tile = { (f32)x, (f32)y };
            face_top.type = COLLISION_STATIC;
            face_bottom.type = COLLISION_STATIC;

            if (tile_is_of_type(level, tile, TILE_WALL))
            {
                f32 t = WALL_SIZE * 0.5f;

                struct v2 tile_top    = { tile.x, tile.y + WALL_SIZE };
                struct v2 tile_bottom = { tile.x, tile.y - WALL_SIZE };

                if (tile_inside_level_bounds(level, tile_bottom) &&
                    tile_is_free(level, tile_bottom))
                {
                    if (line_segment_empty(face_bottom))
                    {
                        face_bottom.start =
                            (struct v2){ tile.x - t, tile.y - t };
                    }

                    face_bottom.end = (struct v2){ tile.x + t, tile.y - t };
                }
                else if (!insert_face(&face_bottom, faces, max, count))
                {
                    return;
                }

                if (tile_inside_level_bounds(level, tile_top) &&
                    tile_is_free(level, tile_top))
                {
                    if (line_segment_empty(face_top))
                    {
                        face_top.start = (struct v2){ tile.x - t, tile.y + t };
                    }

                    face_top.end = (struct v2){ tile.x + t, tile.y + t };
                }
                else if (!insert_face(&face_top, faces, max, count))
                {
                    return;
                }
            }
            else
            {
                if (!insert_face(&face_top, faces, max, count))
                {
                    return;
                }

                if (!insert_face(&face_bottom, faces, max, count))
                {
                    return;
                }
            }
        }
    }

    for (u32 x = 0; x < level->width; x++)
    {
        struct line_segment face_left  = { 0.0f };
        struct line_segment face_right = { 0.0f };

        for (u32 y = 0; y < level->height; y++)
        {
            struct v2 tile = { (f32)x, (f32)y };
            face_left.type = COLLISION_STATIC;
            face_right.type = COLLISION_STATIC;

            if (tile_is_of_type(level, tile, TILE_WALL))
            {
                f32 t = WALL_SIZE * 0.5f;

                struct v2 tile_left  = { tile.x - WALL_SIZE, tile.y };
                struct v2 tile_right = { tile.x + WALL_SIZE, tile.y };

                if (tile_inside_level_bounds(level, tile_left) &&
                    tile_is_free(level, tile_left))
                {
                    if (line_segment_empty(face_left))
                    {
                        face_left.start =
                            (struct v2){ tile.x - t, tile.y - t };
                    }

                    face_left.end = (struct v2){ tile.x - t, tile.y + t };
                }
                else if (!insert_face(&face_left, faces, max, count))
                {
                    return;
                }

                if (tile_inside_level_bounds(level, tile_right) &&
                    tile_is_free(level, tile_right))
                {
                    if (line_segment_empty(face_right))
                    {
                        face_right.start =
                            (struct v2){ tile.x + t, tile.y - t };
                    }

                    face_right.end = (struct v2){ tile.x + t, tile.y + t };
                }
                else if (!insert_face(&face_right, faces, max, count))
                {
                    return;
                }
            }
            else
            {
                if (!insert_face(&face_right, faces, max, count))
                {
                    return;
                }

                if (!insert_face(&face_left, faces, max, count))
                {
                    return;
                }
            }
        }
    }
}

void collision_map_dynamic_calculate(struct game_state* state)
{
    state->cols.num_dynamics = 0;
    struct line_segment segments[4] = { 0 };

    if (state->player.alive)
    {
        get_body_rectangle(state->player.body, PLAYER_RADIUS, PLAYER_RADIUS,
            segments);

        for (u32 i = 0; i < 4 && state->cols.num_dynamics < MAX_STATICS; i++)
        {
            segments[i].type = COLLISION_PLAYER;
            state->cols.dynamics[state->cols.num_dynamics++] = segments[i];
        }
    }

    for (u32 i = 0; i < state->num_enemies; i++)
    {
        struct enemy* enemy = &state->enemies[i];

        if (enemy->alive)
        {
            get_body_rectangle(enemy->body, PLAYER_RADIUS, PLAYER_RADIUS,
                segments);

            for (u32 i = 0; i < 4 && state->cols.num_dynamics < MAX_STATICS;
                i++)
            {
                segments[i].type = COLLISION_ENEMY;
                state->cols.dynamics[state->cols.num_dynamics++] = segments[i];
            }
        }
    }
}

void get_wall_corners_from_faces(struct v2 corners[], u32 max, u32* count,
    struct line_segment faces[], u32 num_faces)
{
    for (u32 i = 0; i < num_faces; i++)
    {
        if (!insert_corner(faces[i].start, corners, max, count))
        {
            return;
        }

        if (!insert_corner(faces[i].end, corners, max, count))
        {
            return;
        }
    }
}

b32 direction_in_range(struct v2 dir, struct v2 left, struct v2 right)
{
    b32 result = v2_cross(left, dir) < 0.0f && v2_cross(right, dir) > 0.0f;

    return result;
}

void reorder_corners_ccw(struct v2* corners, u32 count, struct v2 position)
{
    for (u32 i = 0; i < count - 1; i++)
    {
        for (u32 j = i + 1; j < count; j++)
        {
            if (f32_triangle_area_signed(position, corners[i], corners[j]) < 0)
            {
                v2_swap(&corners[i], &corners[j]);
            }
        }
    }
}

void line_of_sight_render(struct game_state* state, struct v2 position,
    f32 angle_start, f32 angle_max, struct v4 color, b32 render_lines)
{
    struct v2 corners[MAX_WALL_CORNERS] = { 0 };
    u32 num_corners = 0;

    struct v2 collision = { 0 };
    struct v2 dir_left = v2_direction_from_angle(angle_start + angle_max);
    struct v2 dir_right = v2_direction_from_angle(angle_start - angle_max);

    ray_cast_direction(&state->cols, position, dir_right, &collision,
        COLLISION_ALL);
    corners[num_corners++] = collision;

    ray_cast_direction(&state->cols, position, dir_left, &collision,
        COLLISION_ALL);
    corners[num_corners++] = collision;

    for (u32 i = 0; i < state->cols.num_dynamics &&
        num_corners < MAX_WALL_CORNERS; i++)
    {
        struct v2 temp = state->cols.dynamics[i].start;

        if (direction_in_range(v2_direction(position, temp), dir_left,
            dir_right))
        {
            corners[num_corners++] = temp;
        }
    }

    for (u32 i = 0; i < state->num_wall_corners &&
        num_corners < MAX_WALL_CORNERS; i++)
    {
        struct v2 temp = state->wall_corners[i];

        if (direction_in_range(v2_direction(position, temp), dir_left,
            dir_right))
        {
            corners[num_corners++] = temp;
        }
    }

    static u32 lel = 0;

    struct v2 finals[MAX_WALL_CORNERS] = { 0 };
    u32 num_finals = 0;

    struct mesh_render_info info = { 0 };
    info.color = colors[RED];
    info.mesh = &state->floor;
    info.texture = state->texture_tileset;
    info.shader = state->shader_simple;

    struct mesh_render_info info_triangle = { 0 };
    info_triangle.color = color;
    info_triangle.shader = state->shader_simple;

    for (u32 i = 0; i < num_corners; i++)
    {
        struct v2 direction = v2_direction(position, corners[i]);
        f32 angle = f32_atan(direction.y, direction.x);
        f32 t = 0.00001f;

        ray_cast_direction(&state->cols, position, direction, &collision,
            COLLISION_ALL);

        struct v2 collision_temp = { 0 };
        finals[num_finals++] = collision;

        if (render_lines)
        {
            line_render(&info, position, collision,0.005f, 0.005f,
                state->camera.perspective, state->camera.view);
        }

        ray_cast_direction(&state->cols, position,
            v2_direction_from_angle(angle + t), &collision_temp, COLLISION_ALL);

        if (v2_distance(collision_temp, collision) > 0.001f)
        {
            if (render_lines)
            {
                line_render(&info, position, collision_temp, 0.005f, 0.005f,
                    state->camera.perspective, state->camera.view);
            }

            finals[num_finals++] = collision_temp;
        }

        ray_cast_direction(&state->cols, position,
            v2_direction_from_angle(angle - t), &collision_temp, COLLISION_ALL);

        if (v2_distance(collision_temp, collision) > 0.001f)
        {
            if (render_lines)
            {
                line_render(&info, position, collision_temp, 0.005f, 0.005f,
                    state->camera.perspective, state->camera.view);
            }

            finals[num_finals++] = collision_temp;
        }
    }

    reorder_corners_ccw(finals, num_finals, position);

    for (u32 i = 0; i < num_finals-1; i++)
    {
        triangle_render(&info_triangle, position, finals[i], finals[i+1],
            0.0025f, state->camera.perspective, state->camera.view);
    }
}

void enemies_render(struct game_state* state)
{
    for (u32 i = 0; i < MAX_ENEMIES; i++)
    {
        struct enemy* enemy = &state->enemies[i];

        if (enemy->alive)
        {
            switch (enemy->state)
            {
                case ENEMY_STATE_SHOOT:
                {
                    enemy->cube.faces[0].texture = 15;
                } break;
                case ENEMY_STATE_WANDER_AROUND:
                case ENEMY_STATE_LOOK_AROUND:
                {
                    enemy->cube.faces[0].texture = 13;
                } break;
                case ENEMY_STATE_SLEEP:
                {
                    enemy->cube.faces[0].texture = 12;
                } break;
                case ENEMY_STATE_RUSH_TO_TARGET:
                case ENEMY_STATE_REACT_TO_PLAYER_SEEN:
                case ENEMY_STATE_REACT_TO_GUN_SHOT:
                case ENEMY_STATE_REACT_TO_BEING_SHOT_AT:
                case ENEMY_STATE_LOOK_FOR_PLAYER:
                default:
                {
                    enemy->cube.faces[0].texture = 14;
                } break;
            }

            struct m4 transform = m4_translate(enemy->body->position.x,
                enemy->body->position.y, PLAYER_RADIUS);
            struct m4 rotation = m4_rotate_z(enemy->body->angle);
            struct m4 scale = m4_scale_xyz(PLAYER_RADIUS, PLAYER_RADIUS, 0.25f);
            struct m4 model = m4_mul_m4(scale, rotation);
            model = m4_mul_m4(model, transform);

            enemy->cube.model = model;

            cube_renderer_add(&state->cube_renderer, &enemy->cube);

            if (state->render_debug)
            {
                // Todo: calculate temporarily here
                struct v2 eye = { PLAYER_RADIUS + 0.0001f, 0.0f };
                eye = v2_rotate(eye, enemy->body->angle);
                eye = v2_add(eye, enemy->body->position);

                line_of_sight_render(state, eye, enemy->body->angle,
                    enemy->vision_cone_size * ENEMY_LINE_OF_SIGHT_HALF,
                    enemy->player_in_view ? colors[PURPLE] : colors[TEAL],
                    true);
            }

            health_bar_render(&state->render_info_health_bar, &state->camera,
                enemy->body->position, enemy->health, ENEMY_HEALTH_MAX);
            ammo_bar_render(&state->render_info_ammo_bar, &state->camera,
                enemy->body->position, (f32)enemy->weapon.ammo,
                (f32)enemy->weapon.ammo_max);

#if 0
            // Render velocity vector
            if (state->render_debug)
            {
                f32 max_speed = 7.0f;
                f32 length = v2_length(enemy->body->velocity) / max_speed;
                f32 angle = f32_atan(enemy->body->velocity.x,
                    enemy->body->velocity.y);

                transform = m4_translate(
                    enemy->body->position.x + enemy->body->velocity.x /
                        max_speed,
                    enemy->body->position.y + enemy->body->velocity.y /
                        max_speed,
                    0.012f);

                rotation = m4_rotate_z(-angle);
                scale = m4_scale_xyz(0.05f, length, 0.01f);

                model = m4_mul_m4(scale, rotation);
                model = m4_mul_m4(model, transform);

                struct m4 mvp = m4_mul_m4(model, state->camera.view);
                mvp = m4_mul_m4(mvp, state->camera.perspective);

                mesh_render(&state->floor, &mvp, state->texture_tileset,
                    state->shader_simple, colors[GREY]);
            }

            // Render aim vector
            if (state->render_debug)
            {
                f32 length = 1.0f;
                f32 angle = f32_atan(enemy->direction_aim.x,
                    enemy->direction_aim.y);

                transform = m4_translate(
                    enemy->body->position.x + enemy->direction_aim.x / length,
                    enemy->body->position.y + enemy->direction_aim.y / length,
                    0.011f);

                rotation = m4_rotate_z(-angle);
                scale = m4_scale_xyz(0.025f, length, 0.01f);

                model = m4_mul_m4(scale, rotation);
                model = m4_mul_m4(model, transform);

                struct m4 mvp = m4_mul_m4(model, state->camera.view);
                mvp = m4_mul_m4(mvp, state->camera.perspective);

                mesh_render(&state->floor, &mvp, state->texture_tileset,
                    state->shader_simple, colors[YELLOW]);
            }

            // Render look vector
            if (state->render_debug)
            {
                f32 length = 1.0f;
                f32 angle = f32_atan(enemy->direction_look.x,
                    enemy->direction_look.y);

                transform = m4_translate(
                    enemy->body->position.x + enemy->direction_look.x / length,
                    enemy->body->position.y + enemy->direction_look.y / length,
                    0.010f);

                rotation = m4_rotate_z(-angle);
                scale = m4_scale_xyz(0.025f, length, 0.01f);

                model = m4_mul_m4(scale, rotation);
                model = m4_mul_m4(model, transform);

                struct m4 mvp = m4_mul_m4(model, state->camera.view);
                mvp = m4_mul_m4(mvp, state->camera.perspective);

                mesh_render(&state->floor, &mvp, state->texture_tileset,
                    state->shader_simple, colors[BLUE]);
            }

            // Render path
            if (state->render_debug)
            {
                if (enemy->path_length)
                {
                    struct v2 current = enemy->body->position;

                    for (u32 j = enemy->path_index; j < enemy->path_length; j++)
                    {
                        struct v2 next = enemy->path[j];

                        line_render(state, current, next, colors[i],
                            0.005f, 0.02f);

                        current = next;
                    }
                }
            }
#endif
        }
    }
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
                bullet->body->velocity.x * dt,
                bullet->body->velocity.y * dt
            };

            f32 distance_target = v2_length(move_delta);
            u32 flags = COLLISION_STATIC;

            if (bullet->player_owned)
            {
                flags |= COLLISION_ENEMY;
            }
            else
            {
                flags |= COLLISION_PLAYER;
            }

            f32 distance = ray_cast_direction(&state->cols,
                bullet->body->position, v2_normalize(move_delta), NULL, flags);

            f32 fraction = 1.0f;

            if (distance < distance_target)
            {
                fraction = distance / distance_target;
            }

            bullet->body->position.x += move_delta.x * fraction;
            bullet->body->position.y += move_delta.y * fraction;

            if (distance < distance_target)
            {
                rigid_body_free(bullet->body);

                bullet->alive = false;

                f32 angle = f32_atan(-bullet->body->velocity.y,
                    -bullet->body->velocity.x);
                f32 spread = 0.75f;
                f32 color = f32_random(0.25f, 0.75f);

                struct particle_emitter_config config;
                config.position.xy = bullet->body->position;
                config.position.z = 0.5f;
                config.permanent = false;
                config.type = PARTICLE_EMITTER_POINT;
                config.spawn_radius_min = 0.125f;
                config.spawn_radius_max = 0.25f;
                config.move_away_from_spawn = false;
                config.rate = 0.025f;
                config.max_particles = 5;
                config.velocity_min = 1.25f;
                config.velocity_max = 2.25f;
                config.velocity_angular_min = -2.5f;
                config.velocity_angular_max = 2.5f;
                config.scale_start = 0.0125;
                config.scale_end = 0.05f;
                config.color_start = (struct v4){ color, color, color, 1.0f };
                config.color_end = (struct v4){ .xyz = colors[GREY].xyz, 0.0f };
                config.opacity_start = 1.0f;
                config.opacity_end = 0.25f;
                config.time_min = 0.125f;
                config.time_max = 0.25f;
                config.texture = GFX_CIRCLE_FILLED;
                config.direction_min = angle - spread;
                config.direction_max = angle + spread;
                config.lifetime = 0.1f;

                f32 target_size = PLAYER_RADIUS + PROJECTILE_RADIUS;

                // Todo: we might not need to do this if the ray cast system
                // could differentiate whether the ray hit a wall or an entity
                if (bullet->player_owned)
                {
                    for (u32 j = 0; j < state->num_enemies; j++)
                    {
                        struct enemy* enemy = &state->enemies[j];
                        struct line_segment segments[4] = { 0 };

                        get_body_rectangle(enemy->body, target_size,
                            target_size, segments);

                        struct v2 corners[] =
                        {
                            segments[0].start,
                            segments[1].start,
                            segments[2].start,
                            segments[3].start
                        };

                        if (enemy->alive && collision_point_to_obb(
                            bullet->body->position, corners))
                        {
                            bullet->alive = false;
                            enemy->health -= bullet->damage;
                            enemy->got_hit = true;
                            enemy->hit_direction = v2_flip(v2_normalize(
                                bullet->body->velocity));

                            config.color_start = colors[RED];
                            config.color_end = colors[RED];

                            break;
                        }
                    }
                }
                else
                {
                    struct player* player = &state->player;
                    struct line_segment segments[4] = { 0 };

                    get_body_rectangle(player->body, target_size, target_size,
                        segments);

                    struct v2 corners[] =
                    {
                        segments[0].start,
                        segments[1].start,
                        segments[2].start,
                        segments[3].start
                    };

                    if (player->alive && collision_point_to_obb(
                        bullet->body->position, corners))
                    {
                        bullet->alive = false;
                        player->health -= bullet->damage;

                        config.color_start = colors[RED];
                        config.color_end = colors[RED];
                    }
                }

                particle_emitter_create(&state->particle_system, &config, true);
            }

            particle_line_create(state, bullet->start, bullet->body->position,
                (struct v4){ .xyz = bullet->color.xyz, 0.75f },
                (struct v4){ .xyz = bullet->color.xyz, 0.0f },
                0.45f);

            bullet->start = bullet->body->position;
        }
    }
}

void bullets_render(struct game_state* state)
{
    struct mesh_render_info info = { 0 };
    info.color = colors[WHITE];
    info.mesh = &state->sphere;
    info.texture = state->texture_sphere;
    info.shader = state->shader;

    for (u32 i = 0; i < MAX_BULLETS; i++)
    {
        struct bullet* bullet = &state->bullets[i];

        if (bullet->alive)
        {
            struct m4 transform = m4_translate(bullet->body->position.x,
                bullet->body->position.y, PLAYER_RADIUS);
            struct m4 rotation = m4_rotate_z(bullet->body->angle);
            struct m4 scale = m4_scale_all(PROJECTILE_RADIUS);

            struct m4 model = m4_mul_m4(scale, rotation);
            model = m4_mul_m4(model, transform);

            struct m4 mvp = m4_mul_m4(model, state->camera.view);
            mvp = m4_mul_m4(mvp, state->camera.perspective);

            mesh_render(&info, &mvp);
        }
    }
}

void player_update(struct game_state* state, struct game_input* input, f32 dt)
{
    struct player* player = &state->player;

    player->alive = player->health > 0.0f;

    if (player->alive)
    {
        struct v2 direction = { 0.0f };

        struct v2 dir = { state->mouse.world.x - player->body->position.x,
            (state->mouse.world.y - PLAYER_RADIUS) - player->body->position.y };
        dir = v2_normalize(dir);

        player->body->angle = f32_atan(dir.y, dir.x);

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

        struct rigid_body* body = player->body;

        body->acceleration.x = direction.x * PLAYER_ACCELERATION;
        body->acceleration.y = direction.y * PLAYER_ACCELERATION;

        // Todo: this falls behind here as the position is updated after the
        // eye
        struct v2 eye = { PLAYER_RADIUS + 0.0001f, 0.0f };

        player->eye_position = v2_rotate(eye, player->body->angle);
        player->eye_position.x += player->body->position.x;
        player->eye_position.y += player->body->position.y;

        struct weapon* weapon = &player->weapon;

        if (weapon->last_shot > 0.0f)
        {
            weapon->last_shot -= dt;
        }

        if (player->item_picked)
        {
            switch (player->item_picked)
            {
                case ITEM_HEALTH:
                {
                    player->health += ITEAM_HEALTH_AMOUNT;
                    player->health = MIN(player->health, PLAYER_HEALTH_MAX);
                } break;
                case ITEM_PISTOL:
                case ITEM_MACHINEGUN:
                case ITEM_SHOTGUN:
                {
                    u32 weapon_type = player->item_picked - 1;

                    if (weapon->type != weapon_type)
                    {
                        *weapon = weapon_create(player->item_picked - 1);
                        weapon->ammo = 0;
                        weapon_reload(weapon, true);
                    }

                } break;
                case ITEM_WEAPON_LEVEL_UP:
                {
                    if (weapon_level_up(weapon))
                    {
                        weapon_reload(weapon, true);
                    }
                }
            }

            player->item_picked = ITEM_NONE;
        }

        weapon->direction = dir;
        weapon->position = player->eye_position;
        weapon->velocity = player->body->velocity;

        if (key_times_pressed(&input->reload))
        {
            weapon_reload(weapon, false);
        }

        if (state->level_cleared)
        {
            if (key_times_pressed(&input->weapon_pick) &&
                tile_type_get(&state->level, player->body->position) ==
                TILE_START)
            {
                LOG("Changing level...\n");
                state->level_change = true;
            }
        }

        if (input->shoot.key_down && !state->player_in_start_room)
        {
            weapon_shoot(state, weapon, true);
        }
        else
        {
            weapon->fired = false;
        }

        if (weapon->reloading)
        {
            if (weapon->last_shot <= 0.0f)
            {
                weapon->ammo++;

                if (weapon->ammo == weapon->ammo_max)
                {
                    weapon->reloading = false;
                }
                else
                {
                    weapon->last_shot = weapon->reload_time;
                }
            }
        }

        struct particle_emitter* emitter = &state->particle_system.emitters[0];

        // Todo: testing, remove when enough
        if (v2_length(player->body->velocity) > 1.0f)
        {
            struct particle_emitter_config* config = &emitter->config;
            config->position.xy = player->body->position;

            f32 angle = f32_atan(-player->body->velocity.y,
                -player->body->velocity.x);
            f32 spread = 0.45f;

            config->direction_min = angle - spread;
            config->direction_max = angle + spread;

            emitter->active = true;
        }
        else
        {
            emitter->active = false;
        }
    }
}

void player_render(struct game_state* state)
{
    struct player* player = &state->player;

    if (player->alive)
    {
        struct m4 transform = m4_translate(player->body->position.x,
            player->body->position.y, PLAYER_RADIUS);
        struct m4 rotation = m4_rotate_z(player->body->angle);
        struct m4 scale = m4_scale_xyz(PLAYER_RADIUS, PLAYER_RADIUS, 0.25f);
        struct m4 model = m4_mul_m4(scale, rotation);
        model = m4_mul_m4(model, transform);

        player->cube.model = model;

        cube_renderer_add(&state->cube_renderer, &player->cube);

        if (state->render_debug)
        {
            struct v4 color = colors[LIME];
            color.a = 0.5f;

            // Todo: calculate temporarily here
            struct v2 eye = { PLAYER_RADIUS + 0.0001f, 0.0f };
            eye = v2_rotate(eye, player->body->angle);
            eye = v2_add(eye, player->body->position);

            line_of_sight_render(state, eye, player->body->angle,
                ENEMY_LINE_OF_SIGHT_HALF, color, true);
        }

#if 0
        // Render velocity vector
        if (state->render_debug)
        {
            f32 max_speed = 7.0f;
            f32 length = v2_length(player->body->velocity) / max_speed;
            f32 angle = f32_atan(player->body->velocity.x,
                player->body->velocity.y);

            transform = m4_translate(
                player->body->position.x + player->body->velocity.x / max_speed,
                player->body->position.y + player->body->velocity.y / max_speed,
                0.01f);

            rotation = m4_rotate_z(-angle);
            scale = m4_scale_xyz(0.05f, length, 0.01f);

            model = m4_mul_m4(scale, rotation);
            model = m4_mul_m4(model, transform);

            struct m4 mvp = m4_mul_m4(model, state->camera.view);
            mvp = m4_mul_m4(mvp, state->camera.perspective);

            mesh_render(&state->floor, &mvp, state->texture_tileset,
                state->shader_simple, colors[GREY]);
        }

        // Render aim vector
        if (state->render_debug)
        {
            struct v2 vec =
            {
                state->mouse.world.x - player->body->position.x,
                (state->mouse.world.y - PLAYER_RADIUS) -
                    player->body->position.y
            };

            f32 length = v2_length(vec) * 0.5f;
            f32 angle = f32_atan(vec.x, vec.y);

            struct v2 direction = v2_normalize(vec);

            transform = m4_translate(
                player->body->position.x + direction.x * length,
                player->body->position.y + direction.y * length, PLAYER_RADIUS);
            rotation = m4_rotate_z(-angle);
            scale = m4_scale_xyz(0.01f, length, 0.01f);

            model = m4_mul_m4(scale, rotation);
            model = m4_mul_m4(model, transform);

            struct m4 mvp = m4_mul_m4(model, state->camera.view);
            mvp = m4_mul_m4(mvp, state->camera.perspective);

            mesh_render(&state->floor, &mvp, state->texture_tileset,
                state->shader_simple, colors[RED]);
        }
#endif

        struct weapon* weapon = &player->weapon;
        health_bar_render(&state->render_info_health_bar, &state->camera,
            player->body->position, player->health, PLAYER_HEALTH_MAX);
        ammo_bar_render(&state->render_info_ammo_bar, &state->camera,
            player->body->position, (f32)weapon->ammo, (f32)weapon->ammo_max);
        weapon_level_bar_render(&state->render_info_weapon_bar, &state->camera,
            player->body->position, weapon->level, WEAPON_LEVEL_MAX);
    }
}

void items_update(struct game_state* state, struct game_input* input, f32 dt)
{
    for (u32 i = 0; i < MAX_ITEMS; i++)
    {
        struct item* item = &state->items[i];

        if (item->alive)
        {
            item->body->angle -= (f32)F64_PI * dt;
            item->alive -= dt;

            if (item->alive < 0.0f)
            {
                item->alive = 0.0f;
            }
            else if (item->alive < ITEM_FLASH_TIME)
            {
                if (item->flash_timer <= 0.0f)
                {
                    item->flash_timer = ITEM_FLASH_SPEED;
                    item->flash_hide = !item->flash_hide;
                }
                else
                {
                    item->flash_timer -= dt;
                }
            }

            struct player* player = &state->player;

            if (player->alive && collision_circle_to_circle(
                item->body->position, ITEM_RADIUS, player->body->position,
                PLAYER_RADIUS))
            {
                if (item->type < ITEM_SHOTGUN || item->type > ITEM_PISTOL ||
                    key_times_pressed(&input->weapon_pick))
                {
                    player->item_picked = item->type;
                    item->alive = false;

                    rigid_body_free(item->body);
                }
            }
        }
    }
}

void items_render(struct game_state* state)
{
    for (u32 i = 0; i < MAX_ITEMS; i++)
    {
        struct item* item = &state->items[i];

        if (item->alive && !item->flash_hide)
        {
            f32 t = f32_sin(item->body->angle) * 0.25f;

            struct m4 transform = m4_translate(item->body->position.x,
                item->body->position.y, ITEM_RADIUS);
            struct m4 rotation = m4_rotate_z(item->body->angle);
            struct m4 scale = m4_scale_all(ITEM_RADIUS + t * ITEM_RADIUS);
            struct m4 model = m4_mul_m4(scale, rotation);

            model = m4_mul_m4(model, transform);

            item->cube.model = model;

            cube_renderer_add(&state->cube_renderer, &item->cube);
        }
    }
}

void particle_lines_update(struct game_state* state, struct game_input* input,
    f32 dt)
{
    for (u32 i = 0; i < MAX_PARTICLES; i++)
    {
        struct particle_line* particle = &state->particle_lines[i];

        if (particle->alive)
        {
            if (particle->time_current <= 0.0f)
            {
                particle->alive = false;
            }
            else
            {
                particle->time_current -= dt;
                f32 t = 1.0f - particle->time_current / particle->time_start;

                particle->color_current = particle->color_start;

                particle->color_current.r = particle->color_start.r +
                    (particle->color_end.r - particle->color_start.r) * t;
                particle->color_current.g = particle->color_start.g +
                    (particle->color_end.g - particle->color_start.g) * t;
                particle->color_current.b = particle->color_start.b +
                    (particle->color_end.b - particle->color_start.b) * t;
                particle->color_current.a = particle->color_start.a +
                    (particle->color_end.a - particle->color_start.a) * t;
            }
        }
    }
}

void particle_lines_render(struct game_state* state)
{
    // Todo: hax, but this fixes transparent blending
    f32 depth = 0.0f;

    struct mesh_render_info info = { 0 };
    info.color = colors[WHITE];
    info.mesh = &state->floor;
    info.texture = state->texture_tileset;
    info.shader = state->shader_simple;

    for (u32 i = 0; i < MAX_PARTICLES; i++)
    {
        struct particle_line* particle = &state->particle_lines[i];

        if (particle->alive)
        {
            depth += 0.0001f;

            info.color = particle->color_current;

            line_render(&info, particle->start, particle->end,
                PLAYER_RADIUS + depth, PROJECTILE_RADIUS * 0.5f,
                state->camera.perspective, state->camera.view);
        }
    }
}

void level_mask_init(struct level* mask)
{
    // Inited once per game round e.g. until the player has died

    // Todo: now hard coded, randomize in the future
    mask->width = 8;
    mask->height = 8;
    mask->start_pos = (struct v2) { 1.0f, 1.0f };

    // Level mask mask is used to make each level structurally compatible.
    // The outer walls and start room location will be the same for each
    // level but the inner walls may differ.
    {
        u32 width = mask->width;
        u32 height = mask->height;
        u32 start_x = (u32)mask->start_pos.x;
        u32 start_y = (u32)mask->start_pos.y;

        u64 size = width * height;
        u8* data = mask->tile_types;

        memory_set(data, size, 1);

        {
            data[5 * width + 3] = 0;
            data[5 * width + 4] = 0;
            data[6 * width + 3] = 0;
            data[6 * width + 4] = 0;
        }

        // Reserve space for the starting room
        data[start_y * width + start_x] = 0;
    }
}

void level_init(struct game_state* state)
{
    // Clear everything
    memory_set(state->bullets,
        sizeof(struct bullet) * MAX_BULLETS, 0);
    memory_set(state->enemies,
        sizeof(struct enemy) * MAX_ENEMIES, 0);
    memory_set(state->items,
        sizeof(struct item) * MAX_ITEMS, 0);
    memory_set(state->particle_lines,
        sizeof(struct particle_line) * MAX_PARTICLES, 0);
    memory_set(state->wall_corners,
        sizeof(struct v2) * MAX_WALL_CORNERS, 0);
    memory_set(state->wall_faces,
        sizeof(struct line_segment) * MAX_WALL_FACES, 0);
    memory_set(&state->cols, sizeof(struct collision_map), 0);
    memory_set(state->gun_shots,
        sizeof(struct gun_shot) * MAX_GUN_SHOTS, 0);

    state->num_enemies = 0;
    state->num_wall_corners = 0;
    state->num_wall_faces = 0;
    state->num_gun_shots = 0;
    state->free_bullet = 0;
    state->free_item = 0;
    state->free_particle_line = 0;
    state->free_particle_sphere = 0;

    // Inited once per level
    u32 enemies_min = state->level_current;
    u32 enemies_max =  MIN(enemies_min * 4, MAX_ENEMIES);
    state->num_enemies = u32_random(enemies_min, enemies_max);
    // state->num_enemies = 1;

    LOG("%u enemies\n", state->num_enemies);

    level_generate(&state->stack, &state->level, &state->level_mask);

    state->level.wall_info.mesh = &state->wall;
    state->level.wall_info.texture = state->texture_tileset;
    state->level.wall_info.shader = state->shader;
    state->level.wall_info.color = colors[WHITE];

    u32 color_enemy = cube_renderer_color_add(&state->cube_renderer,
        (struct v4){ 0.7f, 0.90f, 0.1f, 1.0f });
    u32 color_player = cube_renderer_color_add(&state->cube_renderer,
        (struct v4){ 1.0f, 0.4f, 0.9f, 1.0f });

    for (u32 i = 0; i < state->num_enemies; i++)
    {
        struct enemy* enemy = &state->enemies[i];
        enemy->body = rigid_body_get(&state->world);
        enemy->body->position = tile_random_get(&state->level, TILE_FLOOR);
        enemy->body->friction = FRICTION;
        enemy->body->radius = PLAYER_RADIUS;
        enemy->alive = true;
        enemy->health = ENEMY_HEALTH_MAX;
        enemy->vision_cone_size = 0.2f * i;
        enemy->shooting = false;
        enemy->cube.faces[0].texture = 13;
        enemy->state = u32_random(0, 1) ? ENEMY_STATE_SLEEP :
            ENEMY_STATE_WANDER_AROUND;
        // enemy->state = ENEMY_STATE_WANDER_AROUND;

        LOG("Enemy %u is %s\n", i,
            enemy->state == ENEMY_STATE_SLEEP ? "sleeping" : "wandering");

        cube_data_color_update(&enemy->cube, color_enemy);

        enemy->weapon = weapon_create(u32_random(1, 3));
        enemy->weapon.projectile_damage *= 0.2f;
    }

    if (state->level_current == 1)
    {
        state->player.weapon = weapon_create(WEAPON_MACHINEGUN);
    }

    if (!state->player.alive)
    {
        state->player.body = rigid_body_get(&state->world);
        state->player.body->position = state->level.start_pos;
        state->player.body->friction = FRICTION;
        state->player.body->radius = PLAYER_RADIUS;
        state->player.alive = true;
        state->player.health = PLAYER_HEALTH_MAX;
        state->player.cube.faces[0].texture = 11;
    }

    cube_data_color_update(&state->player.cube, color_player);

    state->mouse.world = state->player.body->position;

    struct camera* camera = &state->camera;
    camera->position.xy = state->level.start_pos;
    camera->target.xy = state->level.start_pos;
    camera->target.z = 3.75f;
    camera->view = m4_translate(-camera->position.x, -camera->position.y,
        -camera->position.z);
    camera->view_inverse = m4_inverse(camera->view);

    collision_map_static_calculate(&state->level, state->cols.statics,
        MAX_STATICS, &state->cols.num_statics);

    LOG("Wall faces: %d/%d\n", state->cols.num_statics, MAX_STATICS);

    get_wall_corners_from_faces(state->wall_corners, MAX_WALL_CORNERS,
        &state->num_wall_corners, state->cols.statics,
        state->cols.num_statics);

    LOG("Wall corners: %d/%d\n", state->num_wall_corners, MAX_WALL_CORNERS);

    {
        struct m4 transform = m4_translate(state->level.start_pos.x,
            state->level.start_pos.y + 1.5f, 0.5f);
        struct m4 rotation = m4_rotate_z(0.0f);
        struct m4 scale = m4_scale_xyz(0.25f, 0.125f, 0.125f);
        struct m4 model = m4_mul_m4(scale, rotation);
        model = m4_mul_m4(model, transform);

        state->level.elevator_light.model = model;

        cube_data_color_update(&state->level.elevator_light, RED);
    }
}

#include "tk_state_game.c"
#include "tk_state_physics.c"

b32 PHYSICS_DEBUG = false;

struct state_interface state_physics;
struct state_interface state_game;

void game_init(struct game_memory* memory, struct game_init* init)
{
    struct game_state* state = (struct game_state*)memory->base;

    // Init globals that use the game memory each time the game init is called
    _log = *init->log;
    _rng.seed = &state->random_seed;
    api = init->api;

    if (!memory->initialized)
    {
        // Init OpenGL
        s32 version_major = 0;
        s32 version_minor = 0;
        s32 uniform_blocks_max_vertex = 0;
        s32 uniform_blocks_max_geometry = 0;
        s32 uniform_blocks_max_fragment = 0;
        s32 uniform_blocks_max_combined = 0;
        s32 uniform_buffer_max_bindings = 0;
        s32 uniform_block_max_size = 0;
        s32 vertex_attribs_max = 0;

        api.gl.glGetIntegerv(GL_MAJOR_VERSION, &version_major);
        api.gl.glGetIntegerv(GL_MINOR_VERSION, &version_minor);
        api.gl.glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS,
            &uniform_blocks_max_vertex);
        api.gl.glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS,
            &uniform_blocks_max_geometry);
        api.gl.glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS,
            &uniform_blocks_max_fragment);
        api.gl.glGetIntegerv(GL_MAX_COMBINED_UNIFORM_BLOCKS,
            &uniform_blocks_max_combined);
        api.gl.glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS,
            &uniform_buffer_max_bindings);
        api.gl.glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE,
            &uniform_block_max_size);
        api.gl.glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &vertex_attribs_max);

        LOG("OpenGL %i.%i\n", version_major, version_minor);
        LOG("Uniform blocks max vertex: %d\n", uniform_blocks_max_vertex);
        LOG("Uniform blocks max gemoetry: %d\n", uniform_blocks_max_geometry);
        LOG("Uniform blocks max fragment: %d\n", uniform_blocks_max_fragment);
        LOG("Uniform blocks max combined: %d\n", uniform_blocks_max_combined);
        LOG("Uniform buffer max bindings: %d\n", uniform_buffer_max_bindings);
        LOG("Uniform block max size: %d\n", uniform_block_max_size);
        LOG("Vertex attribs max: %d\n", vertex_attribs_max);

        api.gl.glEnable(GL_DEPTH_TEST);
        api.gl.glEnable(GL_BLEND);
        api.gl.glEnable(GL_CULL_FACE);
        api.gl.glDepthFunc(GL_LESS);
        api.gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        api.gl.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        // Init screen size
        state->camera.screen_width = (f32)init->screen_width;
        state->camera.screen_height = (f32)init->screen_height;

        // Init stack allocator
        state->stack.base = (s8*)state + sizeof(struct game_state);
        state->stack.current = state->stack.base;
        state->stack.size = MEGABYTES(512);

        // Init random seed
        state->random_seed = (u32)init->init_time;

        // Init resources
        state->shader = program_create(&state->stack,
            "assets/shaders/vertex.glsl",
            "assets/shaders/fragment.glsl");

        state->shader_simple = program_create(&state->stack,
            "assets/shaders/vertex.glsl",
            "assets/shaders/fragment_simple.glsl");

        state->shader_cube = program_create(&state->stack,
            "assets/shaders/vertex_cube.glsl",
            "assets/shaders/fragment_cube.glsl");

        state->shader_sprite = program_create(&state->stack,
            "assets/shaders/vertex_sprite.glsl",
            "assets/shaders/fragment_sprite.glsl");

        state->shader_particle = program_create(&state->stack,
            "assets/shaders/vertex_particle.glsl",
            "assets/shaders/fragment_particle.glsl");

        state->texture_tileset = texture_create(&state->stack,
            "assets/textures/tileset.tga");
        state->texture_sphere = texture_create(&state->stack,
            "assets/textures/sphere.tga");
        state->texture_cube = texture_array_create(&state->stack,
            "assets/textures/cube.tga", 4, 4);
        state->texture_sprite = texture_array_create(&state->stack,
            "assets/textures/tileset.tga", 8, 8);
        state->texture_particle = texture_array_create(&state->stack,
            "assets/textures/particles.tga", 8, 8);

        mesh_create(&state->stack, "assets/meshes/sphere.mesh",
            &state->sphere);
        mesh_create(&state->stack, "assets/meshes/wall.mesh",
            &state->wall);
        mesh_create(&state->stack, "assets/meshes/floor.mesh",
            &state->floor);
        mesh_create(&state->stack, "assets/meshes/triangle.mesh",
            &state->triangle);

        state->render_info_health_bar = (struct mesh_render_info)
        {
            colors[RED], &state->floor, state->texture_tileset,
            state->shader_simple
        };
        state->render_info_ammo_bar = (struct mesh_render_info)
        {
            colors[YELLOW], &state->floor, state->texture_tileset,
            state->shader_simple
        };
        state->render_info_weapon_bar = (struct mesh_render_info)
        {
            colors[LIME], &state->floor, state->texture_tileset,
            state->shader_simple
        };

        state_physics = state_physics_create(state);
        state_game = state_game_create(state);

        state->state_current = PHYSICS_DEBUG ? &state_physics : &state_game;
        state->state_current->init(state->state_current->data);

        memory->initialized = true;
    }

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

        f32 step = 1.0f / 120.0f;

        state->accumulator += input->delta_time;

        while (state->accumulator >= step)
        {
            state->accumulator -= step;
            state->state_current->update(state->state_current->data, input,
                step);

            u32 num_keys = sizeof(input->keys)/sizeof(input->keys[0]);

            for (u32 i = 0; i < num_keys; i++)
            {
                input->keys[i].transitions = 0;
            }
        }

        api.gl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        state->state_current->render(state->state_current->data);
    }
    else
    {
        LOG("game_update: memory not initialized!\n");
    }
}
