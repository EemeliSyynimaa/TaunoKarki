// Todo: this is not really a particle thing but a primitive
// Todo: create a primitive renderer
struct particle_line
{
    struct v2 start;
    struct v2 end;
    struct v4 color_start;
    struct v4 color_end;
    struct v4 color_current;
    f32 time_start;
    f32 time_current;
    b32 alive;
};
