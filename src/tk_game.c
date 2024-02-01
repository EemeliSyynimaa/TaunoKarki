#include "tk_game.h"

#include "tk_resources.c"
#include "tk_camera.c"

// Todo: create mesh_renderer that can be configured to work with particles,
// cubes, sprites and more
#include "tk_primitive_renderer.c"
#include "tk_sprite_renderer.c"
#include "tk_cube_renderer.c"
#include "tk_particle.c"

#include "tk_level.c"
#include "tk_collision.c"
#include "tk_physics.c"
#include "tk_input.c"

#include "tk_pathfind.c"
#include "tk_weapon.c"
#include "tk_world.c"

#include "tk_enemy.c"

void collision_map_render(struct game_state* state, struct line_segment* cols,
    u32 num_cols, struct camera* camera)
{
    struct mesh_render_info info = { 0 };
    info.color = colors[RED];
    info.mesh = &state->floor;
    info.texture = state->texture_tileset;
    info.shader = state->shader_simple;

    struct m4 vp = m4_mul_m4(camera->view, camera->projection);

    for (u32 i = 0; i < num_cols; i++)
    {
        line_render(&info, cols[i].start, cols[i].end, WALL_SIZE + 0.01f,
            0.025f, vp);
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

struct item* item_create(struct physics_world* world,
    struct object_pool* item_pool, struct v2 position, u32 type)
{
    struct item* result = NULL;

    if (type > ITEM_NONE && type < ITEM_COUNT)
    {
        result = object_pool_get_next(item_pool);

        if (!result->alive)
        {
            entity_add_body(&result->header, world);
        }

        struct rigid_body* body = result->header.body;
        body->type = RIGID_BODY_DYNAMIC;
        body->position = position;
        body->trigger = true;
        result->alive = ITEM_ALIVE_TIME;
        result->type = type;
        body_add_circle_collider(body, v2_zero, ITEM_RADIUS, COLLISION_ITEM,
            COLLISION_PLAYER);

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

void enemies_update(struct game_state* state, struct game_input* input, f32 dt)
{
    // Todo: clean this function...
    for (u32 i = 0; i < MAX_ENEMIES; i++)
    {
        struct enemy* enemy = &state->enemies[i];

        if (enemy->alive)
        {
            struct rigid_body* body = enemy->header.body;

            if (enemy->health < 0.0f)
            {
                enemy->alive = false;

                rigid_body_free(body);

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

                item_create(&state->world, &state->item_pool,
                    body->position, ITEM_HEALTH + enemy->weapon.type);

                u32 random_item_count = u32_random(0, 3);

                for (u32 j = 0; j < random_item_count; j++)
                {
                    struct v2 position = body->position;

                    f32 offset_max = 0.25f;
                    f32 offset_x = f32_random(-offset_max, offset_max);
                    f32 offset_y = f32_random(-offset_max, offset_max);

                    position.x += offset_x;
                    position.y += offset_y;

                    u32 random_item_type = u32_random(0, 2);
                    random_item_type =
                        random_item_type ? ITEM_HEALTH : ITEM_WEAPON_LEVEL_UP;

                    item_create(&state->world, &state->item_pool, position,
                        random_item_type);
                }

                continue;
            }

            enemy->player_in_view = enemy_sees_player(&state->world, enemy,
                &state->player);

            enemy->gun_shot_heard = enemy_hears_gun_shot(state->gun_shots,
                state->num_gun_shots, enemy);

            if (enemy->player_in_view)
            {
                enemy->player_last_seen_position =
                    state->player.header.body->position;
                enemy->player_last_seen_direction = v2_normalize(
                    body->velocity);

                if (enemy->state != ENEMY_STATE_SHOOT &&
                    enemy->state != ENEMY_STATE_SLEEP)
                {
                    enemy_state_transition(enemy,
                        ENEMY_STATE_REACT_TO_PLAYER_SEEN, &state->level,
                        &state->cols);
                }
            }
            else if (enemy->gun_shot_heard)
            {
                // Todo: enemy should react only to the closest source of noise
                if (enemy->state == ENEMY_STATE_SLEEP)
                {
                    enemy_state_transition(enemy,
                        ENEMY_STATE_LOOK_FOR_PLAYER, &state->level,
                        &state->cols);
                }
                else if (enemy->state == ENEMY_STATE_RUSH_TO_TARGET)
                {
                    enemy->target = enemy->gun_shot_position;
                    enemy_calculate_path_to_target(&state->cols, &state->level,
                        enemy);
                }
                else if (enemy->state != ENEMY_STATE_SHOOT)
                {
                    enemy_state_transition(enemy,
                        ENEMY_STATE_REACT_TO_GUN_SHOT, &state->level,
                        &state->cols);
                }
            }
            else if (enemy->got_hit)
            {
                // Todo: react only if the source of bullet is not visible?
                if (enemy->state == ENEMY_STATE_SLEEP)
                {
                    enemy_state_transition(enemy,
                        ENEMY_STATE_LOOK_FOR_PLAYER, &state->level,
                        &state->cols);
                }
                else if (enemy->state != ENEMY_STATE_SHOOT)
                {
                    enemy_state_transition(enemy,
                        ENEMY_STATE_REACT_TO_BEING_SHOT_AT, &state->level,
                        &state->cols);
                }

                enemy->got_hit = false;
            }

            struct level* level = &state->level;
            struct collision_map* cols = &state->cols;

            switch (enemy->state)
            {
                case ENEMY_STATE_SLEEP:
                    break;
                case ENEMY_STATE_REACT_TO_PLAYER_SEEN:
                    enemy_state_react_to_player_seen_update(enemy, level, cols);
                    break;
                case ENEMY_STATE_REACT_TO_GUN_SHOT:
                    enemy_state_react_to_gun_shot_update(enemy, level, cols);
                    break;
                case ENEMY_STATE_SHOOT:
                    enemy_state_shoot_update(enemy, level, cols,
                        &state->bullet_pool, &state->world);
                    break;
                case ENEMY_STATE_RUSH_TO_TARGET:
                    enemy_state_rush_to_target_update(enemy, level, cols);
                    break;
                case ENEMY_STATE_LOOK_AROUND:
                    enemy_state_look_around_update(enemy, level, cols);
                    break;
                case ENEMY_STATE_WANDER_AROUND:
                    enemy_state_wander_around_update(enemy, level, cols);
                    break;
                case ENEMY_STATE_REACT_TO_BEING_SHOT_AT:
                    enemy_state_react_to_being_shot_at_update(enemy, level,
                        cols);
                    break;
                case ENEMY_STATE_LOOK_FOR_PLAYER:
                    enemy_state_look_for_player_update(enemy, level,
                        cols);
                    break;
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

                f32 distance_to_current = v2_distance(body->position,
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
                        body->position, current));

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

            body->acceleration = v2_mul_f32(enemy->direction_move,
                enemy->acceleration);

            if (enemy->turn_amount)
            {
                f32 speed = enemy_turn_speed_get(enemy) * dt;
                f32 remaining = MIN(f32_abs(enemy->turn_amount), speed);

                if (enemy->turn_amount > 0)
                {
                    body->angle -= remaining;
                    enemy->turn_amount = MAX(enemy->turn_amount - remaining, 0);
                }
                else
                {
                    body->angle += remaining;
                    enemy->turn_amount = MIN(enemy->turn_amount + remaining, 0);
                }
            }

            struct v2 eye = { PLAYER_RADIUS + 0.0001f, 0.0f };

            enemy->eye_position = v2_rotate(eye, body->angle);
            enemy->eye_position.x += body->position.x;
            enemy->eye_position.y += body->position.y;

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

void line_of_sight_render(struct game_state* state, struct camera* camera,
    struct v2 position, f32 angle_start, f32 angle_max, struct v4 color,
    b32 render_lines)
{
    struct v2 corners[MAX_WALL_CORNERS] = { 0 };
    u32 num_corners = 0;

    struct ray_cast_collision collision = { 0 };
    struct v2 dir_left = v2_direction_from_angle(angle_start + angle_max);
    struct v2 dir_right = v2_direction_from_angle(angle_start - angle_max);

    collision = world_raycast(&state->world, position, dir_right,
        COLLISION_ALL);
    corners[num_corners++] = collision.position;

    collision = world_raycast(&state->world, position, dir_left,
        COLLISION_ALL);
    corners[num_corners++] = collision.position;

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

    struct m4 vp = m4_mul_m4(camera->view, camera->projection);

    for (u32 i = 0; i < num_corners; i++)
    {
        struct v2 direction = v2_direction(position, corners[i]);
        f32 angle = f32_atan(direction.y, direction.x);
        f32 t = 0.001f;

        collision = world_raycast(&state->world, position, direction,
            COLLISION_ALL);

        if (!collision.body)
        {
            LOG("Raycast returned zero! Pos: %f %f Dir: %f %f Len: %f\n",
                position.x, position.y, direction.x, direction.y,
                v2_length(direction));
        }

        struct ray_cast_collision collision_temp = { 0 };
        finals[num_finals++] = collision.position;

        if (render_lines)
        {
            line_render(&info, position, collision.position, 0.005f, 0.005f,
                vp);
        }

        collision_temp = world_raycast(&state->world, position,
            v2_direction_from_angle(angle + t), COLLISION_ALL);


        if (!collision_temp.body)
        {
            LOG("Raycast returned zero!\n");
        }

        if (v2_distance(collision_temp.position, collision.position) > 0.001f)
        {
            if (render_lines)
            {
                line_render(&info, position, collision_temp.position, 0.005f,
                    0.005f, vp);
            }

            finals[num_finals++] = collision_temp.position;
        }

        collision_temp = world_raycast(&state->world, position,
            v2_direction_from_angle(angle - t), COLLISION_ALL);

        if (!collision_temp.body)
        {
            LOG("Raycast returned zero!\n");
        }

        if (v2_distance(collision_temp.position, collision.position) > 0.001f)
        {
            if (render_lines)
            {
                line_render(&info, position, collision_temp.position, 0.005f,
                    0.005f, vp);
            }

            finals[num_finals++] = collision_temp.position;
        }
    }

    reorder_corners_ccw(finals, num_finals, position);

    for (u32 i = 0; i < num_finals-1; i++)
    {
        triangle_render(&info_triangle, position, finals[i], finals[i+1],
            0.0025f, vp);
    }
}

void enemies_render(struct game_state* state, struct camera* camera_game,
    struct camera* camera_gui)
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

            struct rigid_body* body = enemy->header.body;

            struct m4 transform = m4_translate(body->position.x,
                body->position.y, PLAYER_RADIUS);
            struct m4 rotation = m4_rotate_z(body->angle);
            struct m4 scale = m4_scale_xyz(PLAYER_RADIUS, PLAYER_RADIUS, 0.25f);
            struct m4 model = m4_mul_m4(scale, rotation);
            model = m4_mul_m4(model, transform);

            enemy->cube.model = model;

            cube_renderer_add(&state->cube_renderer, &enemy->cube);

            // if (state->render_debug)
            // {
            //     // Todo: calculate temporarily here
            //     struct v2 eye = { PLAYER_RADIUS + 0.0001f, 0.0f };
            //     eye = v2_rotate(eye, body->angle);
            //     eye = v2_add(eye, body->position);

            //     line_of_sight_render(state, eye, body->angle,
            //         enemy->vision_cone_size * ENEMY_LINE_OF_SIGHT_HALF,
            //         enemy->player_in_view ? colors[PURPLE] : colors[TEAL],
            //         true);
            // }

            struct v2 screen_pos = calculate_screen_pos(body->position.x,
                body->position.y + 0.5f, 0.5f, camera_game);

            health_bar_render(&state->render_info_health_bar, camera_gui,
                screen_pos, enemy->health, ENEMY_HEALTH_MAX);
            ammo_bar_render(&state->render_info_ammo_bar, camera_gui,
                screen_pos, (f32)enemy->weapon.ammo,
                (f32)enemy->weapon.ammo_max);

#if 0
            // Render velocity vector
            if (state->render_debug)
            {
                f32 max_speed = 7.0f;
                f32 length = v2_length(body->velocity) / max_speed;
                f32 angle = f32_atan(body->velocity.x,
                    body->velocity.y);

                transform = m4_translate(
                    body->position.x + body->velocity.x /
                        max_speed,
                    body->position.y + body->velocity.y /
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
                    body->position.x + enemy->direction_aim.x / length,
                    body->position.y + enemy->direction_aim.y / length,
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
                    body->position.x + enemy->direction_look.x / length,
                    body->position.y + enemy->direction_look.y / length,
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
                    struct v2 current = body->position;

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
    struct bullet* bullets = state->bullet_pool.data;

    for (u32 i = 0; i < state->bullet_pool.count; i++)
    {
        struct bullet* bullet = &bullets[i];

        if (bullet->alive)
        {
            struct rigid_body* body = bullet->header.body;
            // struct v2 move_delta =
            // {
            //     body->velocity.x * dt,
            //     body->velocity.y * dt
            // };

            // f32 distance_target = v2_length(move_delta);
            // u32 flags = COLLISION_WALL;

            // if (bullet->player_owned)
            // {
            //     flags |= COLLISION_ENEMY;
            // }
            // else
            // {
            //     flags |= COLLISION_PLAYER;
            // }

            // f32 distance = ray_cast_direction(&state->cols,
            //     body->position, v2_normalize(move_delta), NULL, flags);

            // f32 fraction = 1.0f;

            // if (distance < distance_target)
            // {
            //     fraction = distance / distance_target;
            // }

            // body->position.x += move_delta.x * fraction;
            // body->position.y += move_delta.y * fraction;

            if (false/*distance < distance_target*/)
            {
                rigid_body_free(body);

                bullet->alive = false;

                f32 angle = f32_atan(-body->velocity.y,
                    -body->velocity.x);
                f32 spread = 0.75f;
                f32 color = f32_random(0.25f, 0.75f);

                struct particle_emitter_config config;
                config.position.xy = body->position;
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

                        get_body_rectangle(body, target_size,
                            target_size, segments);

                        struct v2 corners[] =
                        {
                            segments[0].start,
                            segments[1].start,
                            segments[2].start,
                            segments[3].start
                        };

                        if (enemy->alive && collision_point_to_obb(
                            body->position, corners))
                        {
                            bullet->alive = false;
                            enemy->health -= bullet->damage;
                            enemy->got_hit = true;
                            enemy->hit_direction = v2_flip(v2_normalize(
                                body->velocity));

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

                    get_body_rectangle(player->header.body, target_size,
                        target_size, segments);

                    struct v2 corners[] =
                    {
                        segments[0].start,
                        segments[1].start,
                        segments[2].start,
                        segments[3].start
                    };

                    if (player->alive && collision_point_to_obb(
                        body->position, corners))
                    {
                        bullet->alive = false;
                        player->health -= bullet->damage;

                        config.color_start = colors[RED];
                        config.color_end = colors[RED];
                    }
                }

                particle_emitter_create(&state->particle_system, &config, true);
            }

            particle_line_create(state, bullet->start, body->position,
                (struct v4){ .xyz = bullet->color.xyz, 0.75f },
                (struct v4){ .xyz = bullet->color.xyz, 0.0f },
                0.45f);

            bullet->start = body->position;
        }
    }
}

void bullets_render(struct game_state* state, struct camera* camera)
{
    struct mesh_render_info info = { 0 };
    info.color = colors[WHITE];
    info.mesh = &state->sphere;
    info.texture = state->texture_sphere;
    info.shader = state->shader;

    struct bullet* bullets = state->bullet_pool.data;

    for (u32 i = 0; i < state->bullet_pool.count; i++)
    {
        struct bullet* bullet = &bullets[i];

        if (bullet->alive)
        {
            struct rigid_body* body = bullet->header.body;

            struct m4 transform = m4_translate(body->position.x,
                body->position.y, PLAYER_RADIUS);
            struct m4 rotation = m4_rotate_z(body->angle);
            struct m4 scale = m4_scale_all(PROJECTILE_RADIUS);

            struct m4 model = m4_mul_m4(scale, rotation);
            model = m4_mul_m4(model, transform);

            struct m4 mvp = m4_mul_m4(model, camera->view);
            mvp = m4_mul_m4(mvp, camera->projection);

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
        struct rigid_body* body = player->header.body;
        struct v2 direction = { 0.0f };

        struct v2 dir = { state->mouse.world.x - body->position.x,
            (state->mouse.world.y - PLAYER_RADIUS) - body->position.y };
        dir = v2_normalize(dir);

        body->angle = f32_atan(dir.y, dir.x);

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

        body->acceleration.x = direction.x * PLAYER_ACCELERATION;
        body->acceleration.y = direction.y * PLAYER_ACCELERATION;

        // Todo: this falls behind here as the position is updated after the
        // eye
        struct v2 eye = { PLAYER_RADIUS + 0.0001f, 0.0f };

        player->eye_position = v2_rotate(eye, body->angle);
        player->eye_position.x += body->position.x;
        player->eye_position.y += body->position.y;

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
        weapon->position = body->position;
        weapon->velocity = body->velocity;

        if (key_times_pressed(&input->reload))
        {
            weapon_reload(weapon, false);
        }

        if (state->level_cleared)
        {
            if (key_times_pressed(&input->weapon_pick) &&
                tile_type_get(&state->level, body->position) ==
                TILE_START)
            {
                LOG("Changing level...\n");
                state->level_change = true;
            }
        }

        if (input->shoot.key_down && !state->player_in_start_room)
        {
            if (weapon_shoot(&state->bullet_pool, &state->world, weapon, true))
            {
                gun_shot_register(state, weapon->position, 10.0f);
            }
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
        if (v2_length(body->velocity) > 1.0f)
        {
            struct particle_emitter_config* config = &emitter->config;
            config->position.xy = body->position;

            f32 angle = f32_atan(-body->velocity.y, -body->velocity.x);
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

void player_render(struct game_state* state, struct camera* camera_game,
    struct camera* camera_gui)
{
    struct player* player = &state->player;

    if (player->alive)
    {
        struct rigid_body* body = player->header.body;

        struct m4 transform = m4_translate(body->position.x,
            body->position.y, PLAYER_RADIUS);
        struct m4 rotation = m4_rotate_z(body->angle);
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
            eye = v2_rotate(eye, body->angle);
            eye = v2_add(eye, body->position);

            line_of_sight_render(state, camera_game, eye, body->angle,
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
        struct v2 screen_pos = calculate_screen_pos(body->position.x,
            body->position.y + 0.5f, 0.5f, camera_game);

        struct weapon* weapon = &player->weapon;
        health_bar_render(&state->render_info_health_bar, camera_gui,
            screen_pos, player->health, PLAYER_HEALTH_MAX);
        ammo_bar_render(&state->render_info_ammo_bar, camera_gui,
            screen_pos, (f32)weapon->ammo, (f32)weapon->ammo_max);
        weapon_level_bar_render(&state->render_info_weapon_bar, camera_gui,
            screen_pos, weapon->level, WEAPON_LEVEL_MAX);
    }
}

void items_update(struct object_pool* item_pool, struct player* player,
    struct game_input* input, f32 dt)
{
    struct item* items = item_pool->data;

    for (u32 i = 0; i < item_pool->count; i++)
    {
        struct item* item = &items[i];

        if (item->alive)
        {
            struct rigid_body* body = item->header.body;

            body->angle -= (f32)F64_PI * dt;
            item->alive -= dt;

            if (item->alive < 0.0f)
            {
                item->alive = 0.0f;

                rigid_body_free(body);
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

            if (player->alive && collision_circle_to_circle(
                body->position, ITEM_RADIUS, player->header.body->position,
                PLAYER_RADIUS))
            {
                if (item->type < ITEM_SHOTGUN || item->type > ITEM_PISTOL ||
                    key_times_pressed(&input->weapon_pick))
                {
                    player->item_picked = item->type;
                    item->alive = 0;

                    rigid_body_free(body);
                }
            }
        }
    }
}

void items_render(struct object_pool* item_pool, struct cube_renderer* renderer)
{
    struct item* items = item_pool->data;

    for (u32 i = 0; i < item_pool->count; i++)
    {
        struct item* item = &items[i];

        if (item->alive && !item->flash_hide)
        {
            struct rigid_body* body = item->header.body;
            f32 t = f32_sin(body->angle) * 0.25f;

            struct m4 transform = m4_translate(body->position.x,
                body->position.y, ITEM_RADIUS);
            struct m4 rotation = m4_rotate_z(body->angle);
            struct m4 scale = m4_scale_all(ITEM_RADIUS + t * ITEM_RADIUS);
            struct m4 model = m4_mul_m4(scale, rotation);

            model = m4_mul_m4(model, transform);

            item->cube.model = model;

            cube_renderer_add(renderer, &item->cube);
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

void particle_lines_render(struct game_state* state, struct camera* camera)
{
    // Todo: hax, but this fixes transparent blending
    f32 depth = 0.0f;

    struct mesh_render_info info = { 0 };
    info.color = colors[WHITE];
    info.mesh = &state->floor;
    info.texture = state->texture_tileset;
    info.shader = state->shader_simple;

    struct m4 vp = m4_mul_m4(camera->view, camera->projection);

    for (u32 i = 0; i < MAX_PARTICLES; i++)
    {
        struct particle_line* particle = &state->particle_lines[i];

        if (particle->alive)
        {
            depth += 0.0001f;

            info.color = particle->color_current;

            line_render(&info, particle->start, particle->end,
                PLAYER_RADIUS + depth, PROJECTILE_RADIUS * 0.5f, vp);
        }
    }
}

void level_init(struct game_state* state)
{
    // Clear everything
    object_pool_reset(&state->bullet_pool);
    object_pool_reset(&state->item_pool);

    memory_set(state->enemies,
        sizeof(struct enemy) * MAX_ENEMIES, 0);
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
    state->free_particle_line = 0;

    // Inited once per level
    u32 enemies_min = state->level_current;
    u32 enemies_max =  MIN(enemies_min * 4, MAX_ENEMIES);
    state->num_enemies = u32_random(enemies_min, enemies_max);
    // state->num_enemies = 1;

    LOG("%u enemies\n", state->num_enemies);

    level_generate(&state->stack_temporary, &state->level, &state->level_mask);

    state->level.wall_info.mesh = &state->wall;
    state->level.wall_info.texture = state->texture_tileset;
    state->level.wall_info.shader = state->shader;
    state->level.wall_info.color = colors[WHITE];

    u32 color_enemy = cube_renderer_color_add(&state->cube_renderer,
        (struct v4){ 0.7f, 0.90f, 0.1f, 1.0f });
    u32 color_player = cube_renderer_color_add(&state->cube_renderer,
        (struct v4){ 1.0f, 0.4f, 0.9f, 1.0f });

    world_init(&state->world);

    for (u32 i = 0; i < state->num_enemies; i++)
    {
        struct enemy* enemy = &state->enemies[i];
        struct rigid_body* body = entity_add_body(&enemy->header,
            &state->world);
        body->type = RIGID_BODY_DYNAMIC;
        body->position = tile_random_get(&state->level, TILE_FLOOR);
        body->friction = FRICTION;
        enemy->header.type = ENTITY_ENEMY;
        enemy->alive = true;
        enemy->health = ENEMY_HEALTH_MAX;
        enemy->vision_cone_size = 0.2f * i;
        enemy->shooting = false;
        enemy->cube.faces[0].texture = 13;
        enemy->state = u32_random(0, 1) ? ENEMY_STATE_SLEEP :
            ENEMY_STATE_WANDER_AROUND;
        body_add_circle_collider(body, v2_zero, PLAYER_RADIUS, COLLISION_ENEMY,
            (COLLISION_ENEMY | COLLISION_PLAYER | COLLISION_WALL));
        body_add_rect_collider(body, v2_zero, PLAYER_RADIUS, PLAYER_RADIUS,
            COLLISION_ENEMY_HITBOX, COLLISION_NONE);

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
        struct rigid_body* body = entity_add_body(&state->player.header,
            &state->world);
        body->type = RIGID_BODY_DYNAMIC;
        body->position = state->level.start_pos;
        body->friction = FRICTION;
        state->player.header.type = ENTITY_PLAYER;
        state->player.alive = true;
        state->player.health = PLAYER_HEALTH_MAX;
        state->player.cube.faces[0].texture = 11;
        body_add_circle_collider(body, v2_zero, PLAYER_RADIUS, COLLISION_PLAYER,
            (COLLISION_ENEMY | COLLISION_ITEM | COLLISION_WALL));
        body_add_rect_collider(body, v2_zero, PLAYER_RADIUS, PLAYER_RADIUS,
            COLLISION_PLAYER_HITBOX, COLLISION_NONE);
    }

    cube_data_color_update(&state->player.cube, color_player);

    state->mouse.world = state->player.header.body->position;

    struct camera* camera = &state->camera_game;
    camera->position.xy = state->level.start_pos;
    camera->target.xy = state->level.start_pos;
    camera->target.z = 3.75f;
    camera->view = m4_translate(-camera->position.x, -camera->position.y,
        -camera->position.z);
    camera->view_inverse = m4_inverse(camera->view);

    collision_map_static_calculate(&state->level, state->cols.statics,
        MAX_STATICS, &state->cols.num_statics);

    // state->world.walls = state->cols.statics;
    // state->world.num_walls = state->cols.num_statics;

    world_wall_bodies_create(&state->world, state->cols.statics,
        state->cols.num_statics);

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
// #include "tk_state_physics.c"

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
        state->camera_game.width = (f32)init->screen_width;
        state->camera_game.height = (f32)init->screen_height;
        state->camera_gui.width = (f32)init->screen_width;
        state->camera_gui.height = (f32)init->screen_height;

        // Init permanent stack allocator
        state->stack_permanent.base = (s8*)state + sizeof(struct game_state);
        state->stack_permanent.current = state->stack_permanent.base;
        state->stack_permanent.size = MEGABYTES(256);

        // Init temporary stack allocator
        state->stack_temporary.base = (s8*)state + sizeof(struct game_state) +
            state->stack_permanent.size;
        state->stack_temporary.current = state->stack_temporary.base;
        state->stack_temporary.size = MEGABYTES(256);

        // Init random seed
        state->random_seed = (u32)init->init_time;

        // Init resources
        state->shader = program_create(&state->stack_temporary,
            "assets/shaders/vertex.glsl",
            "assets/shaders/fragment.glsl");

        state->shader_simple = program_create(&state->stack_temporary,
            "assets/shaders/vertex.glsl",
            "assets/shaders/fragment_simple.glsl");

        state->shader_cube = program_create(&state->stack_temporary,
            "assets/shaders/vertex_cube.glsl",
            "assets/shaders/fragment_cube.glsl");

        state->shader_sprite = program_create(&state->stack_temporary,
            "assets/shaders/vertex_sprite.glsl",
            "assets/shaders/fragment_sprite.glsl");

        state->shader_particle = program_create(&state->stack_temporary,
            "assets/shaders/vertex_particle.glsl",
            "assets/shaders/fragment_particle.glsl");

        state->texture_tileset = texture_create(&state->stack_temporary,
            "assets/textures/tileset.tga");
        state->texture_sphere = texture_create(&state->stack_temporary,
            "assets/textures/sphere.tga");
        state->texture_cube = texture_array_create(&state->stack_temporary,
            "assets/textures/cube.tga", 4, 4);
        state->texture_sprite = texture_array_create(&state->stack_temporary,
            "assets/textures/tileset.tga", 8, 8);
        state->texture_particle = texture_array_create(&state->stack_temporary,
            "assets/textures/particles.tga", 8, 8);

        mesh_create(&state->stack_temporary, "assets/meshes/sphere.mesh",
            &state->sphere);
        mesh_create(&state->stack_temporary, "assets/meshes/wall.mesh",
            &state->wall);
        mesh_create(&state->stack_temporary, "assets/meshes/floor.mesh",
            &state->floor);
        mesh_create(&state->stack_temporary, "assets/meshes/triangle.mesh",
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

        object_pool_init(&state->bullet_pool, sizeof(struct bullet),
            MAX_BULLETS, &state->stack_permanent);
        object_pool_init(&state->item_pool, sizeof(struct item),
            MAX_ITEMS, &state->stack_permanent);

        // state_physics = state_physics_create(state);
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
