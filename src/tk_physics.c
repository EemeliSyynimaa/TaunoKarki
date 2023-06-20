#define MAX_CIRCLES 64
#define MAX_CONTACTS 64
#define MAX_COLLIDERS 8

struct rigid_body;

struct collider
{
    u32 type;

    u32 tag;
    u32 collidesWith;

    struct rigid_body* body;

    union
    {
        // Circle collider
        struct
        {
            struct v2 position;
            f32 radius;
            f32 unused;
        } circle;

        // Line collider
        struct
        {
            struct v2 a;
            struct v2 b;
        } line;

        // Rect collider
        struct
        {
            struct v2 position;
            f32 half_width;
            f32 half_height;
        } rect;

        // No collider
        struct
        {
            u32 unused[4];
        } none;
    };
};

struct contact
{
    struct collider* a;
    struct collider* b;
    struct v2 position;
    f32 t;

    b32 active;
};

enum
{
    COLLIDER_NONE,
    COLLIDER_CIRCLE,
    COLLIDER_LINE,
    COLLIDER_RECT,
    COLLIDER_COUNT
};

struct circle
{
    struct contact* contact;
    struct v2 position;
    struct v2 velocity;
    struct v2 move_delta;
    struct v2 acceleration;
    struct v2 target;
    f32 radius;
    f32 mass;
    b32 dynamic;
};

enum
{
    RIGID_BODY_STATIC,
    RIGID_BODY_DYNAMIC
};

struct rigid_body
{
    struct contact* contact;
    struct collider colliders[MAX_COLLIDERS];
    struct v2 position;
    struct v2 velocity;
    struct v2 acceleration;
    struct v2 move_delta;
    u32 type;
    u32 num_colliders;
    f32 friction;
    f32 mass;
    f32 angle;
    b32 alive;
    b32 bullet;
    b32 trigger;
};

void get_body_rectangle(struct rigid_body* body, f32 width_half,
    f32 height_half, struct line_segment* segments)
{
    struct v2 corners[4] =
    {
        { -width_half,  height_half },
        {  width_half,  height_half },
        {  width_half, -height_half },
        { -width_half, -height_half }
    };

    for (u32 i = 0; i < 4; i++)
    {
        corners[i] = v2_rotate(corners[i], body->angle);
        corners[i].x += body->position.x;
        corners[i].y += body->position.y;
    }

    for (u32 i = 0; i < 4; i++)
    {
        segments[i].start = corners[i];
        segments[i].end   = corners[(i+1) % 4];
    }
}

f32 ray_cast_body(struct collision_map* cols, struct v2 start,
    struct rigid_body* body, struct v2* collision, u32 flags)
{
    f32 result = 0.0f;

    struct line_segment segments[4] = { 0 };
    u32 num_segments = 0;

    // Todo: test
    for (u32 j = 0; j < body->num_colliders; j++)
    {
        struct collider* collider = &body->colliders[j];

        if (collider->type == COLLIDER_CIRCLE)
        {
            get_body_rectangle(body, collider->circle.radius,
                collider->circle.radius, segments);
            num_segments = 4;
        }
        else if (collider->type == COLLIDER_LINE)
        {
            segments[0].start = collider->line.a;
            segments[0].end = collider->line.b;
            num_segments = 1;
        }
    }

    for (u32 i = 0; i < num_segments; i++)
    {
        segments[i].type = flags;
    }

    struct v2 direction = v2_direction(start, body->position);

    f32 target = ray_cast_to_direction(start, direction, segments, num_segments,
        NULL, F32_MAX, flags) - 0.1f;

    result = ray_cast_direction(cols, start, direction, collision, flags);

    if (result < target)
    {
        result = 0.0f;
    }

    return result;
}

void collision_map_dynamic_calculate(struct collision_map* cols,
    struct rigid_body* bodies, u32 num_bodies)
{
    cols->num_dynamics = 0;
    struct line_segment segments[4] = { 0 };
    u32 num_segments = 0;

    for (u32 i = 0; i < num_bodies; i++)
    {
        struct rigid_body* body = &bodies[i];

        if (body->alive && body->type == RIGID_BODY_DYNAMIC)
        {
            for (u32 j = 0; j < body->num_colliders; j++)
            {
                struct collider* collider = &body->colliders[j];

                if (collider->type == COLLIDER_CIRCLE)
                {
                    get_body_rectangle(body, collider->circle.radius,
                        collider->circle.radius, segments);
                    num_segments = 4;
                }
                else if (collider->type == COLLIDER_LINE)
                {
                    segments[0].start = collider->line.a;
                    segments[0].end = collider->line.b;
                    num_segments = 1;
                }
            }

            for (u32 i = 0; i < num_segments &&
                cols->num_dynamics < MAX_STATICS; i++)
            {
                segments[i].type = COLLISION_ENEMY; // Todo: where to get this?
                cols->dynamics[cols->num_dynamics++] = segments[i];
            }
        }
    }
}

struct physics_world
{
    struct rigid_body bodies[MAX_BODIES];
    struct line_segment* walls;
    struct contact contacts[MAX_CONTACTS];
    u32 num_walls;
    u32 num_contacts;
};

struct rigid_body* rigid_body_get(struct physics_world* world)
{
    struct rigid_body* result = NULL;

    for (u32 i = 0; i < MAX_BODIES; i++)
    {
        if (!world->bodies[i].alive)
        {
            result = &world->bodies[i];
            *result = (struct rigid_body){ 0 };
            result->alive = true;
            result->mass = 1.0f;
            result->num_colliders = 0;

            break;
        }
    }

    return result;
}

void rigid_body_free(struct rigid_body* body)
{
    body->alive = false;
}

b32 collision_detect_circle_circle(struct collider* a, struct collider* b,
    struct contact* contact)
{
    b32 result = false;

    // Todo: add early escapes
    // - amount of movement is less distance between circles minus radii

    // Reduce the velocity of b from the velocity of a
    struct v2 a_vel = v2_sub(a->body->move_delta, b->body->move_delta);

    // Calculate velocity direction
    struct v2 a_dir = v2_normalize(a_vel);

    // Calculate vector from a to b
    struct v2 ab = v2_direction(a->body->position, b->body->position);

    // There cannot be collision if the circles are moving to different
    // directions
    if (v2_dot(ab, a_dir) > 0.0f)
    {
        // Calculate closest point to b on line in velocity direction
        struct v2 d_pos = v2_add(a->body->position, v2_mul_f32(a_dir,
            v2_dot(ab, a_dir)));

        // Calculate velocity
        f32 a_len = v2_length(a_vel);

        // Calculate distance from circle a to d
        f32 ad_len = v2_distance(a->body->position, d_pos);

        // Calculate squared distance from b to d
        f32 bd_len = v2_distance_squared(b->body->position, d_pos);

        // Calculate total squared radii of a and b
        f32 rad_total = f32_square(a->circle.radius + b->circle.radius);

        // There cannot be collision if the length between b and d is equal or
        // greater than the total radii
        if (bd_len < rad_total)
        {
            // Calculate length from c to d
            f32 cd_len = f32_sqrt(rad_total - bd_len);

            // Calculate length from a to c
            f32 ac_len = ad_len - cd_len;

            // Collision occurs if the length from a to c is less than velocity
            if (ac_len < a_len)
            {
                result = true;

                // Store contact position
                if (contact)
                {
                    contact->position = v2_add(a->body->position,
                        v2_mul_f32(a_dir, ac_len));
                    contact->t = ac_len / a_len;
                    contact->a = a;
                    contact->b = b;
                }
            }
        }
    }

    return result;
}

struct v2 get_closest_point_on_line_segment(struct v2 point, struct v2 start,
    struct v2 end)
{
    struct v2 result = { 0.0f };
    struct v2 d = v2_normalize(v2_direction(start, end));
    struct v2 s_to_p = v2_sub(point, start);

    result = v2_add(start, v2_mul_f32(d, v2_dot(s_to_p, d)));

    f32 length_segment = v2_distance(start, end);
    f32 distance_start_closest = v2_distance(start, result);
    f32 distance_end_closest = v2_distance(end, result);

    if (distance_end_closest > length_segment ||
        distance_start_closest > length_segment)
    {
        if (distance_end_closest > distance_start_closest)
        {
            result = start;
        }
        else
        {
            result = end;
        }
    }

    return result;
}

struct v2 get_closest_point_on_line(struct v2 point, struct v2 start,
    struct v2 end)
{
    struct v2 result = { 0.0f };
    struct v2 d = v2_normalize(v2_direction(start, end));
    struct v2 s_to_p = v2_sub(point, start);

    result = v2_add(start, v2_mul_f32(d, v2_dot(s_to_p, d)));

    return result;
}

f32 get_distance_to_closest_point_on_line_segment(struct v2 point,
    struct line_segment line)
{
    f32 result = 0.0f;

    result = v2_distance(get_closest_point_on_line_segment(point, line.start,
        line.end), point);

    return result;
}

b32 collision_detect_circle_line(struct collider* a, struct collider* b,
    struct contact* contact)
{
    b32 result = false;

    struct line_segment line = {
        a->body->position,
        v2_add(a->body->position, a->body->move_delta)
    };

    struct line_segment line_b = { b->line.a, b->line.b };

    f32 r = a->circle.radius;

    if (intersect_line_segment_to_line_segment(line, line_b, NULL) ||
        get_distance_to_closest_point_on_line_segment(line.end, line_b) < r ||
        get_distance_to_closest_point_on_line_segment(line_b.start, line) < r ||
        get_distance_to_closest_point_on_line_segment(line_b.end, line) < r )
    {
        struct v2 line_intersection = { 0 };

        intersect_line_to_line(line, line_b, &line_intersection);

        struct v2 closest = get_closest_point_on_line(a->body->position,
            line_b.start, line_b.end);

        f32 distance_to_closest = v2_distance(a->body->position, closest);
        f32 distance_to_intersection = v2_distance(a->body->position,
            line_intersection);
        f32 distance_to_contact =
            r * (distance_to_intersection / distance_to_closest);

        struct v2 i_to_a = v2_normalize(v2_direction(line_intersection,
            a->body->position));

        struct v2 c = v2_add(line_intersection, v2_mul_f32(i_to_a,
            distance_to_contact));

        contact->t = v2_distance(c, a->body->position) /
            v2_length(a->body->move_delta);
        contact->position = get_closest_point_on_line_segment(c, line_b.start,
            line_b.end);
        contact->a = a;
        contact->b = b;

        // Todo: collisions with corners are not perfect, the circle may get
        // stuck

        result = true;
    }

    return result;
}

u32 body_collisions_check(struct rigid_body bodies[], u32 num_bodies,
    struct contact contacts[], struct line_segment lines[], u32 num_lines)
{
    u32 result = 0;

    // Todo: this looks horrible, refactor and clean
    for (u32 i = 0; i < num_bodies - 1; i++)
    {
        struct rigid_body* body = &bodies[i];

        if (!body->alive || body->bullet)
        {
            continue;
        }

        if (result >= MAX_CONTACTS)
        {
            LOG("Maximum number of contacts reached!\n");
            break;
        }

        // Check collisions against other bodies
        for (u32 j = i + 1; j < num_bodies; j++)
        {
            // Todo: we don't need to check the collisions if neither
            // body is moving
            struct rigid_body* other = &bodies[j];
            struct contact contact = { 0 };

            if (!other->alive || other->bullet)
            {
                continue;
            }

            for (u32 k = 0; k < body->num_colliders; k++)
            {
                struct collider* a = &body->colliders[k];

                if (a->collidesWith == COLLISION_NONE)
                {
                    continue;
                }

                for (u32 l = 0; l < other->num_colliders; l++)
                {
                    struct collider* b = &other->colliders[l];

                    if (!(a->collidesWith & b->tag))
                    {
                        continue;
                    }

                    b32 collision = false;

                    if (a->type == COLLIDER_CIRCLE &&
                        b->type == COLLIDER_CIRCLE )
                    {
                        collision = collision_detect_circle_circle(a, b,
                            &contact);
                    }
                    else if (a->type == COLLIDER_LINE &&
                        b->type == COLLIDER_CIRCLE )
                    {
                        collision = collision_detect_circle_line(a, b,
                            &contact);
                    }
                    else if (a->type == COLLIDER_CIRCLE &&
                        b->type == COLLIDER_LINE )
                    {
                        collision = collision_detect_circle_line(a, b,
                            &contact);
                    }

                    if (collision)
                    {
                        LOG("COLLISION: body %d and body %d\n", i, j);

                        if (body->trigger || other->trigger)
                        {
                            // Todo: implement collision callbacks
                            continue;
                        }

                        if ((!body->contact || contact.t < body->contact->t) &&
                            (!other->contact || contact.t < other->contact->t))
                        {
                            if (!body->contact && !other->contact)
                            {
                                body->contact = &contacts[result++];
                                other->contact = body->contact;
                            }
                            else
                            {
                                if (body->contact)
                                {
                                    if (body->contact->a &&
                                        body->contact->a != a)
                                    {
                                        body->contact->a->body->contact = NULL;
                                    }

                                    if (body->contact->b &&
                                        body->contact->b != a)
                                    {
                                        body->contact->b->body->contact = NULL;
                                    }

                                    other->contact = body->contact;
                                }
                                else
                                {
                                    if (other->contact->a &&
                                        other->contact->a != b)
                                    {
                                        other->contact->a->body->contact = NULL;
                                    }

                                    if (other->contact->b &&
                                        other->contact->b != b)
                                    {
                                        other->contact->b->body->contact = NULL;
                                    }

                                    body->contact = other->contact;
                                }
                            }

                            *body->contact = contact;
                        }
                    }
                }
            }
        }
    }

    return result;
}

void body_collisions_resolve(struct contact contacts[], u32 num_contacts,
    f32 dt)
{
    for (u32 i = 0; i < num_contacts; i++)
    {
        struct contact* contact = &contacts[i];
        // Todo: this is a bit of haxy way to move the remaining (1 - t)
        // with a new velocity
        f32 t_remaining = 1.0f - contact->t;

        struct collider* a = contact->a;
        struct collider* b = contact->b;

        // Circle - circle
        if (a->type == COLLIDER_CIRCLE && b->type == COLLIDER_CIRCLE)
        {
            // Todo: when circle cannot move, if it's in a corner for example,
            // it should be regarded as static!

            // Elastic collisions
            // Calculate new velocities
            struct v2 n = v2_normalize(v2_direction(b->body->position,
                a->body->position));

            f32 a1 = v2_dot(a->body->velocity, n);
            f32 a2 = v2_dot(b->body->velocity, n);

            f32 p = (2.0f * (a1 - a2)) / (a->body->mass + b->body->mass);

            a->body->velocity.x = a->body->velocity.x - p * b->body->mass * n.x;
            a->body->velocity.y = a->body->velocity.y - p * b->body->mass * n.y;

            b->body->velocity.x = b->body->velocity.x + p * a->body->mass * n.x;
            b->body->velocity.y = b->body->velocity.y + p * a->body->mass * n.y;

            a->body->move_delta = v2_mul_f32(a->body->move_delta, contact->t);
            a->body->position = v2_add(a->body->position, a->body->move_delta);
            a->body->move_delta = v2_mul_f32(a->body->velocity,
                t_remaining * dt);

            b->body->move_delta = v2_mul_f32(b->body->move_delta, contact->t);
            b->body->position = v2_add(b->body->position, b->body->move_delta);
            b->body->move_delta = v2_mul_f32(b->body->velocity,
                t_remaining * dt);
        }
        // Circle - line
        else if (a->type == COLLIDER_CIRCLE && b->type == COLLIDER_LINE)
        {
            a->body->position = v2_add(a->body->position,
                v2_mul_f32(a->body->move_delta, contact->t));

            struct v2 n = v2_normalize(v2_direction(a->body->position,
                contact->position));

            f32 vdot_a = v2_dot(a->body->velocity, n);

            a->body->velocity = v2_sub(a->body->velocity,
                v2_mul_f32(n, vdot_a));

            f32 mvdot_a = v2_dot(a->body->move_delta, n);

            a->body->move_delta = v2_mul_f32(v2_sub(a->body->move_delta,
                v2_mul_f32(n, mvdot_a)), t_remaining);
        }
        // No collision??
        else
        {
            LOG("Error: no collidees in collision?\n");
        }

        if (contact->a)
        {
            contact->a->body->contact = NULL;
        }

        if (contact->b)
        {
            contact->b->body->contact = NULL;
        }
    }
}

void body_velocities_update(struct rigid_body bodies[], u32 num_bodies, f32 dt)
{
    for (u32 i = 0; i < num_bodies; i++)
    {
        struct rigid_body* body = &bodies[i];

        if (body->alive)
        {
            body->acceleration.x += -body->velocity.x * body->friction;
            body->acceleration.y += -body->velocity.y * body->friction;

            body->move_delta.x = 0.5f * body->acceleration.x * f32_square(dt) +
                body->velocity.x * dt;
            body->move_delta.y = 0.5f * body->acceleration.y * f32_square(dt) +
                body->velocity.y * dt;

            body->velocity.x = body->velocity.x + body->acceleration.x * dt;
            body->velocity.y = body->velocity.y + body->acceleration.y * dt;
        }
    }
}

void body_positions_update(struct rigid_body bodies[], u32 num_bodies)
{
    for (u32 i = 0; i < num_bodies; i++)
    {
        struct rigid_body* body = &bodies[i];

        if (body->alive)
        {
            body->position.x += body->move_delta.x;
            body->position.y += body->move_delta.y;
        }
    }
}

struct collider* body_add_circle_collider(struct rigid_body* body,
    struct v2 position, f32 radius, u32 tag, u32 collidesWith)
{
    struct collider* result = NULL;

    if (body->num_colliders < MAX_COLLIDERS)
    {
        result = &body->colliders[body->num_colliders++];
        result->type = COLLIDER_CIRCLE;
        result->tag = tag;
        result->collidesWith = collidesWith;
        result->circle.position = position;
        result->circle.radius = radius;
        result->body = body;
    }

    return result;
}

struct collider* body_add_line_collider(struct rigid_body* body,
    struct v2 start, struct v2 end, u32 tag, u32 collidesWith)
{
    struct collider* result = NULL;

    if (body->num_colliders < MAX_COLLIDERS)
    {
        result = &body->colliders[body->num_colliders++];
        result->type = COLLIDER_LINE;
        result->tag = tag;
        result->collidesWith = collidesWith;
        result->line.a = start;
        result->line.b = end;
        result->body = body;
    }

    return result;
}

struct collider* body_add_rect_collider(struct rigid_body* body,
    struct v2 position, f32 half_width, f32 half_height, u32 tag,
    u32 collidesWith)
{
    struct collider* result = NULL;

    if (body->num_colliders < MAX_COLLIDERS)
    {
        result = &body->colliders[body->num_colliders++];
        result->type = COLLIDER_RECT;
        result->tag = tag;
        result->collidesWith = collidesWith;
        result->rect.position = position;
        result->rect.half_width = half_width;
        result->rect.half_height = half_height;
        result->body = body;
    }

    return result;
}

void world_update(struct physics_world* world, f32 step)
{
    body_velocities_update(world->bodies, MAX_BODIES, step);

    u32 max_iterations = 10;

    for (u32 i = 0; i < max_iterations; i++)
    {
        // LOG("Checking collisions, iteration %d\n", i + 1);

        // Todo: sometimes, for some reasons, a collision
        // between two objects happens again in the following
        // iteration. This shouldn't occur as the collision
        // is already resolved and the velocities and positions
        // are updated! Investigate

        // Todo: sometimes collisions are handled poorly
        // when multiple bodies are touching...?

        u32 num_contacts = body_collisions_check(world->bodies, MAX_BODIES,
            world->contacts, world->walls, world->num_walls);

        if (num_contacts)
        {
            body_collisions_resolve(world->contacts, num_contacts, step);
        }
        else
        {
            // LOG("No contacts!\n");
            break;
        }
    }

    body_positions_update(world->bodies, MAX_BODIES);
}

void world_wall_bodies_create(struct physics_world* world,
    struct line_segment* walls, u32 num_walls)
{
    for (u32 i = 0; i < num_walls; i++)
    {
        // Todo: should we store the wall bodies...?
        struct rigid_body* body = rigid_body_get(world);
        body->type = RIGID_BODY_STATIC;
        body_add_line_collider(body, walls[i].start, walls[i].end,
            COLLISION_WALL,
            (COLLISION_PLAYER | COLLISION_ENEMY | COLLISION_BULLET_ENEMY |
                COLLISION_BULLET_PLAYER));
    }
}

void world_init(struct physics_world* world)
{
    memory_set(world->bodies, sizeof(struct rigid_body) * MAX_BODIES, 0);
}
