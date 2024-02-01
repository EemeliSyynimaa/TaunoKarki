// Todo: separate ortho and perspective cameras
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
