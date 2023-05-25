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
        player->body, NULL, COLLISION_STATIC | COLLISION_PLAYER);

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
