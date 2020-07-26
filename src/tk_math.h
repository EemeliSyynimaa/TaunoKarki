#include <math.h>

#define F64_PI 3.1415926535897932384626433832795028841971693993751058209749445

struct v2
{
    f32 x;
    f32 y;
};

struct v3
{
    union
    {
        struct
        {
            f32 x;
            f32 y;
            f32 z;
        };

        struct
        {
            f32 r;
            f32 g;
            f32 b;
        };
    };
};

struct v4
{
    union
    {
        struct
        {
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };

        struct
        {
            f32 r;
            f32 g;
            f32 b;
            f32 a;
        };
    };
};

struct m4
{
    f32 m[4][4];
};

f32 f32_radians(f32 degrees)
{
    f32 result;

    result = degrees * F64_PI / 180.0;

    return result;
}

f32 f32_degrees(f32 radians)
{
    f32 result;

    result = radians * 180.0 / F64_PI;

    return result;
}

f32 f32_atan(f32 y, f32 x)
{
    // Todo: implement own atan(2) function
    return atan2(y, x);
}

f32 f32_sin(f32 angle)
{
    // Todo: implement own sin function
    return sin(angle);
}

f32 f32_cos(f32 angle)
{
    // Todo: implement own cos function
    return cos(angle);
}

f32 f32_tan(f32 angle)
{
    // Todo: implement own tan function
    return tan(angle);
}

f32 f32_sqrt(f32 value)
{
    // Todo: implement own sqrt function
    return sqrt(value);
}

f32 f32_distance(f32 ax, f32 ay, f32 bx, f32 by)
{
    f32 result;

    result = f32_sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay));

    return result;
}

struct m4 m4_translate(f32 x, f32 y, f32 z)
{
    struct m4 m = 
    {{
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        {    x,    y,    z, 1.0f }
    }};

    return m;
}

struct m4 m4_rotate_x(f32 angle)
{
    f32 c = f32_cos(angle);
    f32 s = f32_sin(angle);

    struct m4 m = 
    {{
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f,    c,    s, 0.0f },
        { 0.0f,   -s,    c, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    }};

    return m;
}

struct m4 m4_rotate_y(f32 angle)
{
    f32 c = f32_cos(angle);
    f32 s = f32_sin(angle);

    struct m4 m = 
    {{
        { c,    0.0f,   -s, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { s,    0.0f,    c, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    }};

    return m;
}

struct m4 m4_rotate_z(f32 angle)
{
    f32 c = f32_cos(angle);
    f32 s = f32_sin(angle);

    struct m4 m = 
    {{
        {    c,    s, 0.0f, 0.0f },
        {   -s,    c, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    }};

    return m;
}

struct m4 m4_scale(f32 x, f32 y, f32 z)
{
    struct m4 m = 
    {{
        {    x, 0.0f, 0.0f, 0.0f },
        { 0.0f,    y, 0.0f, 0.0f },
        { 0.0f, 0.0f,    z, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    }};

    return m;
}

struct m4 m4_mul(struct m4 a, struct m4 b)
{
    struct m4 m;

    for (u32 i = 0; i < 4; i++)
    {
        for (u32 j = 0; j < 4; j++)
        {
            m.m[i][j] =
                a.m[i][0] * b.m[0][j] +
                a.m[i][1] * b.m[1][j] + 
                a.m[i][2] * b.m[2][j] + 
                a.m[i][3] * b.m[3][j];
        }
    }

    return m;
}

f32 v3_length(struct v3 v)
{
    f32 result;

    result = f32_sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

    return result;
}

f32 v3_dot(struct v3 a, struct v3 b)
{
    f32 result;

    result = a.x * b.x + a.y * b.y + a.z * b.z;

    return result;
}

struct v3 v3_cross(struct v3 a, struct v3 b)
{
    struct v3 result;

    result.x = a.y * b.z - a.z * b.y;
    result.y = a.x * b.z - a.z * b.x;
    result.z = a.x * b.y - a.y * b.x;

    return result;
}

struct v3 v3_normalize(struct v3 v)
{
    struct v3 result;

    f32 length = v3_length(v);

    result.x = v.x / length;
    result.y = v.y / length;
    result.z = v.z / length;

    return result;
}

struct m4 m4_perspective(f32 fov, f32 aspect, f32 n, f32 f)
{
    f32 t = f32_tan(f32_radians(fov) / 2.0f);

    struct m4 m = 
    {{
        { 1.0f / (t*aspect), 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f/t, 0.0f, 0.0f },
        { 0.0f, 0.0f, (f+n)/(n-f), -1.0f },
        { 0.0f, 0.0f, (2.0f*f*n)/(n-f), 0.0f }
    }};

    return m;
}