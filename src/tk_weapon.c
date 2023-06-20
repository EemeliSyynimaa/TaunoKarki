
void bullet_create(struct object_pool* pool, struct physics_world* world,
    struct v2 position, struct v2 start_velocity, struct v2 direction,
    f32 speed, f32 damage, b32 player_owned, f32 size)
{
    struct bullet* bullet = object_pool_get_next(pool);

    bullet->header.type = ENTITY_BULLET;

    f32 color = f32_random(0.75f, 1.0f);

    struct rigid_body* body = bullet->header.body;

    if (!bullet->alive)
    {
        body = entity_add_body(&bullet->header, world);
    }
    else
    {
        *body = (struct rigid_body){ 0 };
    }

    body->type = RIGID_BODY_DYNAMIC;
    // bullet->body->bullet = true;
    body->position = position;
    body->velocity = start_velocity;
    body->velocity.x += direction.x * speed;
    body->velocity.y += direction.y * speed;
    bullet->alive = true;
    bullet->damage = damage;
    bullet->player_owned = player_owned;
    bullet->start = body->position;
    bullet->color = (struct v4){ color, color, color, 1.0f };

    u32 tag = COLLISION_BULLET_PLAYER;
    u32 collidesWith = (COLLISION_ENEMY_HITBOX | COLLISION_WALL);

    if (!player_owned)
    {
        tag = COLLISION_BULLET_ENEMY;
        collidesWith = (COLLISION_PLAYER_HITBOX | COLLISION_WALL);
    }

    body_add_circle_collider(body, v2_zero, PROJECTILE_RADIUS, tag,
        collidesWith);

    // Todo: add a fancy particle effect here
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

b32 weapon_shoot(struct object_pool* bullet_pool, struct physics_world* world,
    struct weapon* weapon, bool player)
{
    b32 result = false;

    if (weapon->type == WEAPON_PISTOL)
    {
        if (weapon->last_shot <= 0.0f && weapon->ammo && !weapon->fired &&
            !weapon->reloading)
        {
            struct v2 randomized = v2_rotate(weapon->direction,
                f32_random(-weapon->spread, weapon->spread));

            bullet_create(bullet_pool, world, weapon->position,
                weapon->velocity, randomized, weapon->projectile_speed,
                weapon->projectile_damage, player, weapon->projectile_size);

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

            result = true;
        }
    }
    else if (weapon->type == WEAPON_MACHINEGUN)
    {
        if (weapon->last_shot <= 0.0f && weapon->ammo && !weapon->fired &&
            !weapon->reloading)
        {
            struct v2 randomized = v2_rotate(weapon->direction,
                f32_random(-weapon->spread, weapon->spread));

            bullet_create(bullet_pool, world, weapon->position,
                weapon->velocity, randomized, weapon->projectile_speed,
                weapon->projectile_damage, player, weapon->projectile_size);

            if (--weapon->ammo == 0)
            {
                weapon->reloading = true;
                weapon->last_shot = weapon->reload_time;
            }
            else
            {
                weapon->last_shot = weapon->fire_rate;
            }

            result = true;
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

                bullet_create(bullet_pool, world, weapon->position,
                    weapon->velocity, randomized, speed,
                    weapon->projectile_damage, player, weapon->projectile_size);
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

            result = true;
        }
    }

    return result;
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
