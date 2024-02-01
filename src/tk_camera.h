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
