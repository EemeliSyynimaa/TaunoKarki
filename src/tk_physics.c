#define MAX_CIRCLES 64
#define MAX_CONTACTS 64

struct contact
{
    struct rigid_body* a;
    struct rigid_body* b;
    struct line_segment* line;
    struct v2 position;
    f32 t;

    b32 active;
};

struct collider_circle
{
    struct v2 position;
    f32 radius;
};

struct collider_line
{
    struct v2 a;
    struct v2 b;
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

struct rigid_body
{
    struct contact* contact;
    struct v2 position;
    struct v2 velocity;
    struct v2 acceleration;
    struct v2 move_delta;
    f32 friction;
    f32 radius;
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

    get_body_rectangle(body, body->radius, body->radius, segments);

    for (u32 i = 0; i < 4; i++)
    {
        segments[i].type = flags;
    }

    struct v2 direction = v2_direction(start, body->position);

    f32 target = ray_cast_to_direction(start, direction, segments, 4, NULL,
        F32_MAX, flags) - 0.1f;

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

    for (u32 i = 0; i < num_bodies; i++)
    {
        struct rigid_body* body = &bodies[i];

        if (body->alive)
        {
            get_body_rectangle(body, body->radius, body->radius, segments);

            for (u32 i = 0; i < 4 && cols->num_dynamics < MAX_STATICS; i++)
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

            break;
        }
    }

    return result;
}

void rigid_body_free(struct rigid_body* body)
{
    body->alive = false;
}

b32 collision_detect_circle_circle(struct rigid_body* a, struct rigid_body* b,
    struct contact* contact)
{
    b32 result = false;

    // Todo: add early escapes
    // - amount of movement is less distance between circles minus radii

    // Reduce the velocity of b from the velocity of a
    struct v2 a_vel = v2_sub(a->move_delta, b->move_delta);

    // Calculate velocity direction
    struct v2 a_dir = v2_normalize(a_vel);

    // Calculate vector from a to b
    struct v2 ab = v2_direction(a->position, b->position);

    // There cannot be collision if the circles are moving to different
    // directions
    if (v2_dot(ab, a_dir) > 0.0f)
    {
        // Calculate closest point to b on line in velocity direction
        struct v2 d_pos = v2_add(a->position, v2_mul_f32(a_dir,
            v2_dot(ab, a_dir)));

        // Calculate velocity
        f32 a_len = v2_length(a_vel);

        // Calculate distance from circle a to d
        f32 ad_len = v2_distance(a->position, d_pos);

        // Calculate squared distance from b to d
        f32 bd_len = v2_distance_squared(b->position, d_pos);

        // Calculate total squared radii of a and b
        f32 rad_total = f32_square(a->radius + b->radius);

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
                    contact->position = v2_add(a->position,
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

b32 collision_detect_circle_line(struct rigid_body* a, struct line_segment* b,
    struct contact* contact)
{
    b32 result = false;

    struct line_segment line = {
        a->position,
        v2_add(a->position, a->move_delta)
    };

    f32 r = a->radius;

    if (intersect_line_segment_to_line_segment(line, *b, NULL) ||
        get_distance_to_closest_point_on_line_segment(line.end, *b) < r ||
        get_distance_to_closest_point_on_line_segment(b->start, line) < r ||
        get_distance_to_closest_point_on_line_segment(b->end, line) < r )
    {
        struct v2 line_intersection = { 0 };

        intersect_line_to_line(line, *b, &line_intersection);

        struct v2 closest = get_closest_point_on_line(a->position, b->start,
            b->end);

        f32 distance_to_closest = v2_distance(a->position, closest);
        f32 distance_to_intersection = v2_distance(a->position,
            line_intersection);
        f32 distance_to_contact =
            a->radius * (distance_to_intersection / distance_to_closest);

        struct v2 i_to_a = v2_normalize(v2_direction(line_intersection,
            a->position));

        struct v2 c = v2_add(line_intersection, v2_mul_f32(i_to_a,
            distance_to_contact));

        contact->t = v2_distance(c, a->position) / v2_length(a->move_delta);
        contact->position = get_closest_point_on_line_segment(c, b->start,
            b->end);

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
    for (u32 i = 0; i < num_bodies; i++)
    {
        struct rigid_body* body = &bodies[i];

        if (!body->alive || body->bullet)
        {
            continue;
        }

        if (result < MAX_CONTACTS)
        {
            if (i < num_bodies - 1)
            {
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

                    if (collision_detect_circle_circle(body, other, &contact))
                    {
                        LOG("COLLISION: body %d and body %d\n", i, j);

                        if (body->trigger || other->trigger)
                        {
                            // Todo: implement collision callbacks
                            continue;
                        }

                        if ((!body->contact ||
                            contact.t < body->contact->t) &&
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
                                        body->contact->a != body)
                                    {
                                        body->contact->a->contact = NULL;
                                    }

                                    if (body->contact->b &&
                                        body->contact->b != body)
                                    {
                                        body->contact->b->contact = NULL;
                                    }

                                    other->contact = body->contact;
                                }
                                else
                                {
                                    if (other->contact->a &&
                                        other->contact->a != other)
                                    {
                                        other->contact->a->contact = NULL;
                                    }

                                    if (other->contact->b &&
                                        other->contact->b != other)
                                    {
                                        other->contact->b->contact = NULL;
                                    }

                                    body->contact = other->contact;
                                }
                            }

                            *body->contact = contact;
                        }
                    }
                }
            }

            // Todo: we don't need to check these collisions if the circle
            // is not moving

            // Check collisions against static walls
            for (u32 j = 0; j < num_lines; j++)
            {
                struct line_segment* other = &lines[j];
                struct contact contact = { 0 };

                if (collision_detect_circle_line(body, other, &contact))
                {
                    contact.a = body;
                    contact.b = NULL;
                    contact.line = other;

                    // Todo: HERE RECURSIVELY CHCECK THAT EACH CONTACT IS
                    // NULLIFIED FOR ALL PARTIES
                    LOG("COLLISION: body %d and line %d\n", i, j);
                    if (!body->contact || contact.t < body->contact->t)
                    {
                        if (body->contact)
                        {
                            if (body->contact->a &&
                                body->contact->a != body)
                            {
                                body->contact->a->contact = NULL;
                            }

                            if (body->contact->b &&
                                body->contact->b != body)
                            {
                                body->contact->b->contact = NULL;
                            }
                        }
                        else
                        {
                            body->contact = &contacts[result++];
                        }

                        *body->contact = contact;
                    }
                }
            }
        }
        else
        {
            LOG("Maximum number of contacts reached!\n");
            break;
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

        struct rigid_body* a = contact->a;
        struct rigid_body* b = contact->b;
        struct line_segment* line = contact->line;

        // Circle - circle
        if (a && b)
        {
            // Todo: when circle cannot move, if it's in a corner for example,
            // it should be regarded as static!

            // Elastic collisions
            // Calculate new velocities
            struct v2 n = v2_normalize(v2_direction(b->position, a->position));

            f32 a1 = v2_dot(a->velocity, n);
            f32 a2 = v2_dot(b->velocity, n);

            f32 p = (2.0f * (a1 - a2)) / (a->mass + b->mass);

            a->velocity.x = a->velocity.x - p * b->mass * n.x;
            a->velocity.y = a->velocity.y - p * b->mass * n.y;

            b->velocity.x = b->velocity.x + p * a->mass * n.x;
            b->velocity.y = b->velocity.y + p * a->mass * n.y;

            a->move_delta = v2_mul_f32(a->move_delta, contact->t);
            a->position = v2_add(a->position, a->move_delta);
            a->move_delta = v2_mul_f32(a->velocity, t_remaining * dt);

            b->move_delta = v2_mul_f32(b->move_delta, contact->t);
            b->position = v2_add(b->position, b->move_delta);
            b->move_delta = v2_mul_f32(b->velocity, t_remaining * dt);
        }
        // Circle - line
        else if (a && line)
        {
            a->position = v2_add(a->position,
                v2_mul_f32(a->move_delta, contact->t));

            struct v2 n = v2_normalize(v2_direction(a->position,
                contact->position));

            f32 vdot_a = v2_dot(a->velocity, n);

            a->velocity = v2_sub(a->velocity, v2_mul_f32(n, vdot_a));

            f32 mvdot_a = v2_dot(a->move_delta, n);

            a->move_delta = v2_mul_f32(
                v2_sub(a->move_delta, v2_mul_f32(n, mvdot_a)), t_remaining);

        }
        // No collision??
        else
        {
            LOG("Error: no collidees in collision?\n");
        }

        if (contact->a)
        {
            contact->a->contact = NULL;
        }

        if (contact->b)
        {
            contact->b->contact = NULL;
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
