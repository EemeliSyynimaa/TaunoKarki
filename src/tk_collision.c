struct ray_cast_collision
{
    struct v2 position;
    struct v2 wall_start;
    struct v2 wall_end;
    // Todo: add wall normal? Could be nice
    f32 ray_length;
};

struct collision_map
{
    struct line_segment statics[MAX_STATICS];
    struct line_segment dynamics[MAX_STATICS];
    u32 num_statics;
    u32 num_dynamics;
};

u32 COLLISION_STATIC  = 1;
u32 COLLISION_PLAYER  = 2;
u32 COLLISION_ENEMY   = 4;
u32 COLLISION_BULLET  = 8;
u32 COLLISION_DYNAMIC = 14;
u32 COLLISION_ALL     = 255;

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

b32 intersect_line_to_line(struct line_segment line_a,
    struct line_segment line_b, struct v2* collision)
{
    b32 result = false;

    struct v2 p = line_a.start;
    struct v2 r = v2_direction(line_a.start, line_a.end);
    struct v2 q = line_b.start;
    struct v2 s = v2_direction(line_b.start, line_b.end);
    struct v2 qp = { q.x - p.x, q.y - p.y };
    f32 r_x_s = v2_cross(r, s);

    if (r_x_s)
    {
        f32 qp_x_s = v2_cross(qp, s);
        f32 t = qp_x_s / r_x_s;

        if (collision)
        {
            collision->x = p.x + t * r.x;
            collision->y = p.y + t * r.y;
        }

        result = true;
    }

    return result;
}

b32 intersect_line_to_line_segment(struct line_segment line_a,
    struct line_segment line_b, struct v2* collision)
{
    b32 result = false;

    struct v2 p = line_a.start;
    struct v2 r = v2_direction(line_a.start, line_a.end);
    struct v2 q = line_b.start;
    struct v2 s = v2_direction(line_b.start, line_b.end);
    struct v2 qp = { q.x - p.x, q.y - p.y };
    f32 r_x_s = v2_cross(r, s);

    if (r_x_s)
    {
        f32 qp_x_s = v2_cross(qp, s);
        f32 t = qp_x_s / r_x_s;

        if (t > 0.0f)
        {
            collision->x = p.x + t * r.x;
            collision->y = p.y + t * r.y;

            result = true;
        }
    }

    return result;
}

b32 intersect_line_segment_to_line_segment(struct line_segment line_a,
    struct line_segment line_b, struct v2* collision)
{
    b32 result = false;

    struct v2 p = line_a.start;
    struct v2 r = v2_direction(line_a.start, line_a.end);
    struct v2 q = line_b.start;
    struct v2 s = v2_direction(line_b.start, line_b.end);
    struct v2 qp = { q.x - p.x, q.y - p.y };
    f32 r_x_s = v2_cross(r, s);

    if (r_x_s == 0.0f)
    {
        // Todo: implement if necessary
    }
    else
    {
        f32 qp_x_s = v2_cross(qp, s);
        f32 qp_x_r = v2_cross(qp, r);

        f32 t = qp_x_s / r_x_s;
        f32 u = qp_x_r / r_x_s;

        if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f)
        {
            if (collision)
            {
                collision->x = p.x + t * r.x;
                collision->y = p.y + t * r.y;
            }

            result = true;
        }
    }

    return result;
}

b32 intersect_ray_to_line_segment(struct v2 start, struct v2 direction,
    struct line_segment line_segment, struct v2* collision)
{
    b32 result = false;
    struct v2 p = start;
    struct v2 r = direction;
    struct v2 q = line_segment.start;
    struct v2 s = v2_direction(line_segment.start, line_segment.end);
    struct v2 qp = { q.x - p.x, q.y - p.y };
    f32 r_x_s = v2_cross(r, s);

    if (r_x_s == 0.0f)
    {
        // Todo: implement if necessary
    }
    else
    {
        f32 qp_x_s = v2_cross(qp, s);
        f32 qp_x_r = v2_cross(qp, r);

        f32 t = qp_x_s / r_x_s;
        f32 u = qp_x_r / r_x_s;

        if (t > 0.0f && u > 0.0f && u < 1.0f)
        {
            collision->x = p.x + t * r.x;
            collision->y = p.y + t * r.y;

            result = true;
        }
    }

    return result;
}

f32 ray_cast_to_direction(struct v2 start, struct v2 direction,
    struct line_segment cols[], u32 num_cols, struct v2* collision,
    f32 max_length, u32 flags)
{
    f32 result = max_length;

    for (u32 i = 0; i < num_cols; i++)
    {
        struct v2 position = { 0 };

        if (cols[i].type & flags &&
            intersect_ray_to_line_segment(start, direction, cols[i], &position)
            && v2_distance(start, position) < result)
        {
            if (collision)
            {
                *collision = position;
            }

            result = v2_distance(start, position);
        }
    }

    return result;
}

f32 ray_cast_direction(struct collision_map* cols, struct v2 position,
    struct v2 direction, struct v2* collision, u32 flags)
{
    f32 result = F32_MAX;

    if (flags & COLLISION_STATIC)
    {
        result = ray_cast_to_direction(position, direction, cols->statics,
            cols->num_statics, collision, result, flags);
    }

    if (flags & COLLISION_DYNAMIC)
    {
        result = ray_cast_to_direction(position, direction, cols->dynamics,
            cols->num_dynamics, collision, result, flags);
    }

    return result;
}

f32 ray_cast_position(struct collision_map* cols, struct v2 start,
    struct v2 end, struct v2* collision, u32 flags)
{
    f32 result = ray_cast_direction(cols, start, v2_direction(start, end),
        collision, flags);
    f32 distance = v2_distance(start, end) - 0.1f;

    if (result < distance)
    {
        result = 0.0f;
    }

    return result;
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
