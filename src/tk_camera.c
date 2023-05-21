struct camera
{
    struct m4 perspective;
    struct m4 perspective_inverse;
    struct m4 ortho;
    struct m4 ortho_inverse;
    struct m4 view;
    struct m4 view_inverse;
    struct v3 position;
    struct v3 target;
    f32 screen_width;
    f32 screen_height;
};

struct v2 calculate_world_pos(f32 pos_x, f32 pos_y, struct camera* camera)
{
    // Todo: doesn't work with orthographic projection
    struct v2 result = { 0.0f };

    struct v4 ndc =
    {
        pos_x / (camera->screen_width * 0.5f) - 1.0f,
        (pos_y / (camera->screen_height * 0.5f) - 1.0f) * -1.0f
    };

    struct v4 clip = { ndc.x, ndc.y, -1.0f, 1.0f };
    struct v4 view = m4_mul_v4(camera->perspective_inverse, clip);
    view.z = -1.0f;
    view.w = 0.0f;

    struct v4 world = m4_mul_v4(camera->view_inverse, view);

    struct v3 temp = { world.x, world.y, world.z };
    temp = v3_normalize(temp);

    f32 c = f32_abs(camera->position.z / temp.z);

    result.x = temp.x * c + camera->position.x;
    result.y = temp.y * c + camera->position.y;

    return result;
}

struct v2 calculate_screen_pos(f32 pos_x, f32 pos_y, f32 pos_z,
    struct camera* camera)
{
    struct v2 result = { 0.0f };
    struct v4 world = { pos_x, pos_y, pos_z, 1.0f };

    struct v4 view = m4_mul_v4(camera->view, world);
    struct v4 clip = m4_mul_v4(camera->perspective, view);
    struct v2 ndc = { clip.x / clip.w, clip.y / clip.w };

    result.x = (ndc.x + 1.0f) * camera->screen_width * 0.5f;
    result.y = (ndc.y + 1.0f) * camera->screen_height * 0.5f;

    return result;
}
