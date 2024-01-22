#include "tk_math.h"

struct camera
{
    struct m4 projection;
    struct m4 projection_inverse;
    struct m4 view;
    struct m4 view_inverse;
    struct v3 position;
    struct v3 target;
    f32 width;
    f32 height;
};

struct v2 screen_to_world(struct camera* camera, f32 x, f32 y)
{
    // Todo: doesn't work with orthographic projection
    struct v2 result = { 0.0f };

    struct v4 ndc =
    {
        x / (camera->width * 0.5f) - 1.0f,
        (y / (camera->height * 0.5f) - 1.0f) * -1.0f
    };

    struct v4 clip = { ndc.x, ndc.y, -1.0f, 1.0f };
    struct v4 view = m4_mul_v4(camera->projection_inverse, clip);
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

struct v2 world_to_screen(struct camera* camera, f32 x, f32 y, f32 z)
{
    struct v2 result = { 0.0f };
    struct v4 world = { x, y, z, 1.0f };

    struct v4 view = m4_mul_v4(camera->view, world);
    struct v4 clip = m4_mul_v4(camera->projection, view);
    struct v2 ndc = { clip.x / clip.w, clip.y / clip.w };

    result.x = (ndc.x + 1.0f) * camera->width * 0.5f;
    result.y = (ndc.y + 1.0f) * camera->height * 0.5f;

    return result;
}
