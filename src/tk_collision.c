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
    struct v2 pq = { q.x - p.x, q.y - p.y };
    f32 r_x_s = v2_cross(r, s);

    if (r_x_s == 0.0f)
    {
        // Todo: implement if necessary
    }
    else
    {
        f32 pq_x_s = v2_cross(pq, s);
        f32 pq_x_r = v2_cross(pq, r);

        f32 t = pq_x_s / r_x_s;
        f32 u = pq_x_r / r_x_s;

        // Note: this is for rounding errors
        f32 e = 0.00001f;

        if (t >= 0.0f && t <= 1.0f && u >= -e && u <= 1.0f + e)
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
    struct v2 pq = { q.x - p.x, q.y - p.y };
    f32 r_x_s = v2_cross(r, s);

    if (r_x_s == 0.0f)
    {
        // Todo: implement if necessary, we only need this if the ray and
        // line segment are actually on the same line
    }
    else
    {
        f32 pq_x_s = v2_cross(pq, s);
        f32 pq_x_r = v2_cross(pq, r);

        f32 t = pq_x_s / r_x_s;
        f32 u = pq_x_r / r_x_s;

        // Note: this is for rounding errors
        f32 e = 0.00001f;

        if (t >= 0.0f && u >= -e && u <= 1.0f + e)
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

    if (flags & COLLISION_WALL)
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


b32 collision_point_to_aabb(f32 x, f32 y, f32 min_x, f32 max_x, f32 min_y,
    f32 max_y)
{
    b32 result = false;

    result = x >= min_x && x <= max_x && y >= min_y && y <= max_y;

    return result;
}

b32 collision_point_to_obb(struct v2 pos, struct v2 corners[4])
{
    b32 result = false;

    b32 result_top = v2_cross(v2_direction(corners[0], corners[1]),
        v2_direction(corners[0], pos)) < 0.0f;
    b32 result_right = v2_cross(v2_direction(corners[1], corners[2]),
        v2_direction(corners[1], pos)) < 0.0f;
    b32 result_bottom = v2_cross(v2_direction(corners[2], corners[3]),
        v2_direction(corners[2], pos)) < 0.0f;
    b32 result_left = v2_cross(v2_direction(corners[3], corners[0]),
        v2_direction(corners[3], pos)) < 0.0f;

    result = result_top && result_right && result_bottom && result_left;

    return result;
}

b32 collision_circle_to_circle(struct v2 position_a, f32 radius_a,
    struct v2 position_b, f32 radius_b)
{
    b32 result = v2_distance(position_a, position_b) < (radius_a + radius_b);

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

    // Todo: this only works if the circle collides with a corner
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

b32 check_tile_collisions(struct level* level, struct v2* pos, struct v2* vel,
    struct v2 move_delta, f32 radius, f32 bounce_factor)
{
    b32 result = false;

    f32 wall_low = WALL_SIZE * 0.5f;
    f32 wall_high = wall_low + radius;

    f32 margin_x = move_delta.x + radius;
    f32 margin_y = move_delta.y + radius;

    f32 min_x = pos->x + wall_low - margin_x - WALL_SIZE;
    f32 min_y = pos->y + wall_low - margin_y - WALL_SIZE;

    u32 start_x = min_x < 0.0f ? 0 : (u32)((u32)min_x / WALL_SIZE);
    u32 start_y = min_y < 0.0f ? 0 : (u32)((u32)min_y / WALL_SIZE);
    u32 end_x = (u32)((MAX(pos->x, 0) + wall_low + margin_x) / WALL_SIZE) + 1;
    u32 end_y = (u32)((MAX(pos->y, 0) + wall_low + margin_y) / WALL_SIZE) + 1;

    f32 time_remaining = 1.0f;

    for (u32 i = 0; i < 4 && time_remaining > 0.0f; i++)
    {
        f32 time = 1.0f;
        struct v2 normal = { 0.0f };

        for (u32 y = start_y; y <= end_y; y++)
        {
            for (u32 x = start_x; x <= end_x; x++)
            {
                if (level->tile_types[y * level->width + x] != 1)
                {
                    continue;
                }

                struct v2 rel =
                {
                    pos->x - x,
                    pos->y - y
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
            face_top.type = COLLISION_WALL;
            face_bottom.type = COLLISION_WALL;

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
            face_left.type = COLLISION_WALL;
            face_right.type = COLLISION_WALL;

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
