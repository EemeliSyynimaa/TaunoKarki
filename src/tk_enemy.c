enum
{
    ENEMY_STATE_SHOOT,
    ENEMY_STATE_WANDER_AROUND,
    ENEMY_STATE_RUSH_TO_TARGET,
    ENEMY_STATE_SLEEP,
    ENEMY_STATE_REACT_TO_PLAYER_SEEN,
    ENEMY_STATE_REACT_TO_GUN_SHOT,
    ENEMY_STATE_REACT_TO_BEING_SHOT_AT,
    ENEMY_STATE_LOOK_AROUND,
    ENEMY_STATE_LOOK_FOR_PLAYER
};

char enemy_state_str[][256] =
{
    "SHOOT",
    "WANDER AROUND",
    "RUSH TO TARGET",
    "SLEEP",
    "REACT_TO_PLAYER_SEEN",
    "REACT_TO_GUN_SHOT",
    "REACT_TO_BEING_SHOT_AT",
    "LOOK_AROUND",
    "LOOK_FOR_PLAYER"
};

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
        player->body, NULL, COLLISION_WALL | COLLISION_PLAYER);

    result = enemy->state != ENEMY_STATE_SLEEP && player->alive &&
        angle_player < ENEMY_LINE_OF_SIGHT_HALF && player_ray_cast > 0.0f &&
        player_ray_cast < ENEMY_LINE_OF_SIGHT_DISTANCE;

    return result;
}

b32 enemy_hears_gun_shot(struct gun_shot* shots, u32 num_gun_shots,
    struct enemy* enemy)
{
    b32 result = false;
    f32 closest = ENEMY_GUN_FIRE_HEAR_DISTANCE;

    for (u32 i = 0; i < num_gun_shots; i++)
    {
        struct gun_shot* shot = &shots[i];

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

void enemy_state_transition(struct enemy* enemy, u32 state_new,
    struct level* level, struct collision_map* cols)
{
    if (state_new == enemy->state)
    {
        return;
    }

    u32 state_old = enemy->state;

    enemy->state_timer = 0.0f;
    enemy->path_length = 0;
    enemy->state = state_new;

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
            enemy_calculate_path_to_target(cols, level, enemy);
        } break;
        case ENEMY_STATE_WANDER_AROUND:
        {
            enemy->acceleration = ENEMY_ACCELERATION * 0.5f;
            enemy->target = tile_random_get(level, TILE_FLOOR);
            enemy_calculate_path_to_target(cols, level, enemy);
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

void enemy_state_shoot_update(struct enemy* enemy, struct level* level,
    struct collision_map* cols, struct object_pool* bullet_pool,
    struct physics_world* world)
{
    enemy->acceleration = 0.0f;

    if (enemy->player_in_view)
    {
        struct v2 target_forward = v2_direction( enemy->body->position,
            enemy->player_last_seen_position);

        if (v2_length(enemy->body->velocity))
        {
            // Todo: calculate right with cross product
            f32 angle = F64_PI*1.5f;
            f32 tsin = f32_sin(angle);
            f32 tcos = f32_cos(angle);

            struct v2 target_right =
            {
                target_forward.x * tcos - target_forward.y * tsin,
                target_forward.x * tsin + target_forward.y * tcos
            };

            f32 initial_velocity = v2_dot(enemy->body->velocity, target_right);

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
                v2_dot(desired_velocity, target_right_rotate_back),
                v2_dot(desired_velocity, target_forward_rotate_back)
            };

            enemy->direction_aim = v2_normalize(final_direction);
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

        if (weapon_shoot(bullet_pool, world, weapon, false))
        {
            // Todo: implement this
            // gun_shot_register(state, weapon->position, 10.0f);
        }

        if (weapon->fired && enemy->trigger_release <= 0.0f)
        {
            enemy->trigger_release = 0.5f;
        }
    }
    else
    {
        enemy_state_transition(enemy, ENEMY_STATE_REACT_TO_PLAYER_SEEN, level,
            cols);
    }
}

void enemy_state_wander_around_update(struct enemy* enemy, struct level* level,
    struct collision_map* cols)
{
    if (!enemy->path_length)
    {
        enemy_state_transition(enemy, ENEMY_STATE_LOOK_AROUND, level, cols);
    }
}

void enemy_state_rush_to_target_update(struct enemy* enemy, struct level* level,
    struct collision_map* cols)
{
    if (!enemy->path_length)
    {
        enemy_look_towards_position(enemy, enemy->target);
        enemy_state_transition(enemy, ENEMY_STATE_LOOK_FOR_PLAYER, level, cols);
    }
}

void enemy_state_sleep_update(struct enemy* enemy, struct level* level,
    struct collision_map* cols)
{
}

void enemy_state_react_to_player_seen_update(struct enemy* enemy,
    struct level* level, struct collision_map* cols)
{
    if (enemy->state_timer_finished)
    {
        if (enemy->player_in_view)
        {
            enemy_state_transition(enemy, ENEMY_STATE_SHOOT, level, cols);
        }
        else
        {
            enemy->target = enemy->player_last_seen_position;
            enemy_state_transition(enemy, ENEMY_STATE_RUSH_TO_TARGET, level,
                cols);
        }
    }
    else if (enemy->player_in_view)
    {
        enemy_look_towards_position(enemy, enemy->player_last_seen_position);
    }
}

void enemy_state_react_to_gun_shot_update(struct enemy* enemy,
    struct level* level, struct collision_map* cols)
{
    if (enemy->state_timer_finished)
    {
        if (ray_cast_position(cols, enemy->eye_position,
            enemy->gun_shot_position, NULL,
            COLLISION_WALL | COLLISION_PLAYER))
        {
            enemy_state_transition(enemy, ENEMY_STATE_LOOK_FOR_PLAYER, level,
                cols);
        }
        else
        {
            enemy->target = enemy->gun_shot_position;
            enemy_state_transition(enemy, ENEMY_STATE_RUSH_TO_TARGET, level,
                cols);

            // Here we can skip the last node since we only need
            // to see the location where gun shot occured
            if (enemy->path_length > 0)
            {
                enemy->path_length--;
            }
        }
    }
}

void enemy_state_react_to_being_shot_at_update(struct enemy* enemy,
    struct level* level, struct collision_map* cols)
{
    if (enemy->state_timer_finished)
    {
        f32 length = ray_cast_direction(cols, enemy->eye_position,
            enemy->hit_direction, NULL, COLLISION_WALL);

        // Skip one tile so we don't find a path into a wall
        length = MAX(0, length - 1.0f);

        enemy->target = v2_add(enemy->eye_position,
            v2_mul_f32(enemy->hit_direction, length));

        enemy_state_transition(enemy, ENEMY_STATE_RUSH_TO_TARGET, level, cols);

        enemy->hit_direction = v2_zero;
    }
}

void enemy_state_look_around_update(struct enemy* enemy, struct level* level,
    struct collision_map* cols)
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
            enemy_state_transition(enemy, ENEMY_STATE_WANDER_AROUND, level,
                cols);
        }
    }
    else if (enemy->state_timer <= 0.0f && !enemy->turn_amount)
    {
        enemy->state_timer = enemy_look_around_delay_get(enemy);
    }
}

void enemy_state_look_for_player_update(struct enemy* enemy,
    struct level* level, struct collision_map* cols)
{
    if (!v2_is_zero(enemy->player_last_seen_direction))
    {
        enemy_look_towards_direction(enemy, enemy->player_last_seen_direction);
        enemy->player_last_seen_direction = v2_zero;
        enemy->state_timer = enemy_look_around_delay_get(enemy);
    }
    else
    {
        if (enemy->state_timer_finished)
        {
            if (enemy->turns_left)
            {
                f32 diff = f32_random( -F64_PI, F64_PI);
                f32 angle_new = enemy->body->angle + diff;

                enemy_look_towards_angle(enemy, angle_new);

                enemy->turns_left--;
            }
            else
            {
                enemy_state_transition(enemy, ENEMY_STATE_WANDER_AROUND, level,
                    cols);
            }
        }
        else if (enemy->state_timer <= 0.0f && !enemy->turn_amount)
        {
            enemy->state_timer = enemy_look_around_delay_get(enemy);
        }
    }
}
