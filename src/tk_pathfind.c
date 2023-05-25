struct node
{
    // Todo: use integers instead?
    struct v2 position;
    struct node* parent;
    f32 g;
    f32 h;
    f32 f;
    b32 in_use;
};

struct node* node_insert(struct node* node, struct node nodes[], u32 num_nodes)
{
    struct node* result = NULL;

    for (u32 i = 0; i < num_nodes; i++)
    {
        if (!nodes[i].in_use)
        {
            nodes[i] = *node;
            result = &nodes[i];
            break;
        }
    }

    return result;
}

struct node* lowest_rank_find(struct node nodes[], u32 num_nodes)
{
    struct node* result = NULL;

    for (u32 i = 0; i < num_nodes; i++)
    {
        if (nodes[i].in_use && (!result || nodes[i].f < result->f))
        {
            result = &nodes[i];
        }
    }

    return result;
}

b32 nodes_equal(struct node* a, struct node* b)
{
    return v2_equals(a->position, b->position);
}

struct node* node_find(struct v2* node, struct node nodes[], u32 num_nodes)
{
    struct node* result = NULL;

    for (u32 i = 0; i < num_nodes; i++)
    {
        if (nodes[i].in_use && v2_equals(*node, nodes[i].position))
        {
            result = &nodes[i];
            break;
        }
    }

    return result;
}

b32 neighbor_check(struct level* level, f32 x, f32 y, struct v2 neighbors[],
    u32* index)
{
    b32 result = false;

    struct v2 neighbor = { x, y };

    if (tile_is_free(level, neighbor))
    {
        neighbors[(*index)++] = neighbor;
        result = true;
    }

    return result;
}

u32 neighbors_get(struct level* level, struct node* node, struct v2 neighbors[])
{
    u32 result = 0;

    f32 x = node->position.x;
    f32 y = node->position.y;

    b32 left  = neighbor_check(level, x - 1.0f, y, neighbors, &result);
    b32 right = neighbor_check(level, x + 1.0f, y, neighbors, &result);
    b32 up    = neighbor_check(level, x, y + 1.0f, neighbors, &result);
    b32 down  = neighbor_check(level, x, y - 1.0f, neighbors, &result);

    if (left && up)
    {
        neighbor_check(level, x - 1.0f, y + 1.0f, neighbors, &result);
    }

    if (left && down)
    {
        neighbor_check(level, x - 1.0f, y - 1.0f, neighbors, &result);
    }

    if (right && up)
    {
        neighbor_check(level, x + 1.0f, y + 1.0f, neighbors, &result);
    }

    if (right && down)
    {
        neighbor_check(level, x + 1.0f, y - 1.0f, neighbors, &result);
    }

    return result;
}

f32 cost_calculate(struct v2 a, struct v2 b)
{
    f32 result = 0.0f;

    if (a.x != b.x && a.y != b.y)
    {
        result = f32_sqrt(2.0f);
    }
    else
    {
        result = 1.0f;
    }

    return result;
}

f32 heuristic_calculate(struct v2 a, struct v2 b)
{
    f32 result = 0.0f;

    f32 dx = f32_abs(a.x - b.x);
    f32 dy = f32_abs(a.y - b.y);
    f32 cost = 1.0f;
    f32 cost_diagonal = f32_sqrt(2.0f);

    result = cost * (dx + dy) + (cost_diagonal - 2.0f * cost) * MIN(dx, dy);

    return result;
}

void node_add_to_path(struct node* node, struct v2 path[], u32* index)
{
    if (!node->parent)
    {
        return;
    }

    // Todo: this overflows stack sometimes
    node_add_to_path(node->parent, path, index);

    (*index)++;

    if (path)
    {
        path[*index] = node->position;
    }
}

u32 path_find(struct level* level, struct v2 start, struct v2 goal,
    struct v2 path[], u32 path_size)
{
    u32 result = 0;

    if (!tile_is_free(level, start) || !tile_is_free(level, goal))
    {
        return result;
    }

    struct node open[MAX_NODES] = { 0 };
    struct node closed[MAX_NODES] = { 0 };

    start.x = f32_round(start.x);
    start.y = f32_round(start.y);
    goal.x  = f32_round(goal.x);
    goal.y  = f32_round(goal.y);

    struct node node_start = { start, NULL, 0.0f, 0.0f, 0.0f, true };
    struct node node_goal =  { goal, NULL, 0.0f, 0.0f, 0.0f, true };

    node_insert(&node_start, open, MAX_NODES);

    struct node* lowest = NULL;

    while ((lowest = lowest_rank_find(open, MAX_NODES)) &&
        !nodes_equal(lowest, &node_goal))
    {
        struct node* current = node_insert(lowest, closed, MAX_NODES);

        if (!current)
        {
            // Todo: assert here?
            LOG("CLOSED SET FULL!\n");
            break;
        }

        lowest->in_use = false;

        struct v2 neighbors[8];

        u32 num_neighbors = neighbors_get(level, current, neighbors);

        for (u32 i = 0; i < num_neighbors; i++)
        {
            struct v2* neighbor = &neighbors[i];
            struct node* neighbor_node = NULL;
            f32 cost = current->g + cost_calculate(current->position,
                *neighbor);
            f32 heuristic = heuristic_calculate(*neighbor, goal);

            if ((neighbor_node = node_find(neighbor, open, MAX_NODES)) &&
                cost < neighbor_node->g)
            {
                neighbor_node->in_use = false;
            }

            if ((neighbor_node = node_find(neighbor, closed, MAX_NODES)) &&
                cost < neighbor_node->g)
            {
                neighbor_node->in_use = false;
            }

            if (!node_find(neighbor, open, MAX_NODES) &&
                !node_find(neighbor, closed, MAX_NODES))
            {
                struct node new =
                {
                    *neighbor, current, cost, heuristic, cost + heuristic, true
                };

                // Todo: assert here?
                if (!node_insert(&new, open, MAX_NODES))
                {
                    LOG("OPEN SET FULL!\n");
                    break;
                }
            }
        }
    }

    if (lowest)
    {
        node_add_to_path(lowest, path, &result);
        result++;
    }

    return result;
}

void path_trim(struct collision_map* cols, struct v2 obj_start,
    struct v2 path[], u32* path_size)
{
    if (!(*path_size))
    {
        return;
    }

    struct v2 result[MAX_PATH] = { 0 };
    struct v2 start = obj_start;

    u32 result_index = 0;
    u32 path_index = 0;

    while (path_index < (*path_size - 1))
    {
        struct v2 end = path[path_index + 1];

        // Ray cast to each tile corner
        f32 wall_half = WALL_SIZE * 0.5f;

        struct v2 tl = { end.x - wall_half, end.y + wall_half };
        struct v2 bl = { end.x - wall_half, end.y - wall_half };
        struct v2 br = { end.x + wall_half, end.y - wall_half };
        struct v2 tr = { end.x + wall_half, end.y + wall_half };

        if (!ray_cast_position(cols, start, tl, NULL, COLLISION_STATIC) ||
            !ray_cast_position(cols, start, bl, NULL, COLLISION_STATIC) ||
            !ray_cast_position(cols, start, br, NULL, COLLISION_STATIC) ||
            !ray_cast_position(cols, start, tr, NULL, COLLISION_STATIC))
        {
            start = result[result_index++] = path[path_index];
        }
        else
        {
            path_index++;
        }
    }

    // Always insert the end node
    result[result_index++] = path[*path_size - 1];

    *path_size = result_index;
    memory_copy(&result, path, *path_size * sizeof(struct v2));
}
