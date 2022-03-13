#include <math.h>
#include <float.h>

#define F64_PI 3.1415926535897932384626433832795028841971693993751058209749445
#define F32_MAX FLT_MAX
#define F32_MIN FLT_MIN
#define U32_MAX 0xFFFFFFFF
#define S32_MAX 0x7FFFFFFF

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

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

struct m2
{
    f32 m[2][2];
};

struct m3
{
    f32 m[3][3];
};

struct m4
{
    f32 m[4][4];
};

f32 f32_radians(f32 degrees)
{
    f32 result = degrees * F64_PI / 180.0;

    return result;
}

f32 f32_degrees(f32 radians)
{
    f32 result = radians * 180.0 / F64_PI;

    return result;
}

f32 f32_atan(f32 y, f32 x)
{
    // Todo: implement own atan(2) function
    return atan2(y, x);
}

f32 f32_acos(f32 value)
{
    // Todo: implement own acos function
    return acos(value);
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

f32 f32_square(f32 value)
{
    f32 result = value * value;

    return result;
}

f32 f32_abs(f32 value)
{
    f32 result = value < 0 ? value * -1 : value;

    return result;
}

f32 f32_distance(f32 ax, f32 ay, f32 bx, f32 by)
{
    f32 result = f32_sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay));

    return result;
}

void f32_swap(f32* a, f32* b)
{
    f32 t = *a;
    *a = *b;
    *b = t;
}

f32 f32_round(f32 value)
{
    f32 result = (s32)(value + (value < 0 ? -0.5f : 0.5f));

    return result;
}

// Todo: clean matrix and vector functions to use more generic approach like
// below, lots of duplicate code now

f32 determinant(f32* values, u32 size)
{
    // Todo: supports only matrices of size from 1 to 4
    f32 result = 0.0f;

    if (size == 2)
    {
        result = values[0] * values[3] - values[1] * values[2];
    }
    else if (size > 2 && size < 5)
    {
        for (u32 i = 0; i < size; i++)
        {
            // Todo: fixed size!
            f32 temp[16] = { 0 };

            u32 x1 = 0;

            for (u32 x = 0; x < size; x++)
            {
                if (x == i)
                {
                    continue;
                }

                for (u32 y = 0; y < size-1; y++)
                {
                    temp[y * (size-1) + x1] = values[(y+1) * size + x];
                }

                x1++;
            }

            f32 sign = i % 2 ? -1.0f : 1.0f;
            f32 det = determinant(temp, size-1);
            f32 multiplier = values[i];

            result += sign * det * multiplier;
        }
    }
    else if (size == 1)
    {
        result = values[0];
    }

    return result;
}

f32 m2_determinant(struct m2 a)
{
    f32 result = determinant(&a.m[0][0], 2);

    return result;
}

f32 m3_determinant(struct m3 a)
{
    f32 result = determinant(&a.m[0][0], 3);

    return result;
}

f32 m4_determinant(struct m4 a)
{
    f32 result = determinant(&a.m[0][0], 4);

    return result;
}

struct m3 m3_matrix_of_minors(struct m3 a)
{
    struct m3 result = { 0 };

    for (u32 j = 0; j < 3; j++)
    {
        for (u32 k = 0; k < 3; k++)
        {
            u32 y1 = 0;

            struct m2 t;

            for (u32 y = 0; y < 3; y++)
            {
                if (y == j)
                {
                    continue;
                }

                u32 x1 = 0;

                for (u32 x = 0; x < 3; x++)
                {
                    if (x == k)
                    {
                        continue;
                    }

                    t.m[y1][x1] = a.m[y][x];

                    x1++;
                }

                y1++;
            }

            f32 determinant = m2_determinant(t);

            result.m[j][k] = determinant;
        }
    }

    return result;
}

struct m3 m3_matrix_of_cofactors(struct m3 a)
{
    struct m3 result = { 0 };

    for (u32 y = 0; y < 3; y++)
    {
        f32 sign = y % 2 ? -1.0f : 1.0f;

        for (u32 x = 0; x < 3; x++)
        {
            result.m[y][x] = a.m[y][x] * sign;

            sign *= -1.0f;
        }
    }

    return result;
}

struct m3 m3_transpose(struct m3 a)
{
    struct m3 result = { 0 };

    for (u32 y = 0; y < 3; y++)
    {
        for (u32 x = 0; x < 3; x++)
        {
            result.m[x][y] = a.m[y][x];
        }
    }

    return result;
}

struct m3 m3_mul_f32(struct m3 a, f32 b)
{
    struct m3 result = { 0 };

    for (u32 i = 0; i < 3; i++)
    {
        for (u32 j = 0; j < 3; j++)
        {
            result.m[i][j] = a.m[i][j] * b;
        }
    }

    return result;
}

struct m4 m4_matrix_of_cofactors(struct m4 a)
{
    struct m4 result = { 0 };

    for (u32 y = 0; y < 4; y++)
    {
        f32 sign = y % 2 ? -1.0f : 1.0f;

        for (u32 x = 0; x < 4; x++)
        {
            result.m[y][x] = a.m[y][x] * sign;

            sign *= -1.0f;
        }
    }

    return result;
}

struct m4 m4_transpose(struct m4 a)
{
    struct m4 result = { 0 };

    for (u32 y = 0; y < 4; y++)
    {
        for (u32 x = 0; x < 4; x++)
        {
            result.m[x][y] = a.m[y][x];
        }
    }

    return result;
}

struct m4 m4_matrix_of_minors(struct m4 a)
{
    struct m4 result = { 0 };

    for (u32 j = 0; j < 4; j++)
    {
        for (u32 k = 0; k < 4; k++)
        {
            u32 y1 = 0;

            struct m3 t;

            for (u32 y = 0; y < 4; y++)
            {
                if (y == j)
                {
                    continue;
                }

                u32 x1 = 0;

                for (u32 x = 0; x < 4; x++)
                {
                    if (x == k)
                    {
                        continue;
                    }

                    t.m[y1][x1] = a.m[y][x];

                    x1++;
                }

                y1++;
            }

            f32 determinant = m3_determinant(t);

            result.m[j][k] = determinant;
        }
    }

    return result;
}

struct m4 m4_translate(f32 x, f32 y, f32 z)
{
    struct m4 result =
    {{
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        {    x,    y,    z, 1.0f }
    }};

    return result;
}

struct m4 m4_rotate_x(f32 angle)
{
    f32 c = f32_cos(angle);
    f32 s = f32_sin(angle);

    struct m4 result =
    {{
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f,    c,    s, 0.0f },
        { 0.0f,   -s,    c, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    }};

    return result;
}

struct m4 m4_rotate_y(f32 angle)
{
    f32 c = f32_cos(angle);
    f32 s = f32_sin(angle);

    struct m4 result =
    {{
        { c,    0.0f,   -s, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { s,    0.0f,    c, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    }};

    return result;
}

struct m4 m4_rotate_z(f32 angle)
{
    f32 c = f32_cos(angle);
    f32 s = f32_sin(angle);

    struct m4 result =
    {{
        {    c,    s, 0.0f, 0.0f },
        {   -s,    c, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    }};

    return result;
}

struct m4 m4_scale_xyz(f32 x, f32 y, f32 z)
{
    struct m4 result =
    {{
        {    x, 0.0f, 0.0f, 0.0f },
        { 0.0f,    y, 0.0f, 0.0f },
        { 0.0f, 0.0f,    z, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    }};

    return result;
}

struct m4 m4_scale_all(f32 s)
{
    return m4_scale_xyz(s, s, s);
}

struct m4 m4_mul_m4(struct m4 a, struct m4 b)
{
    struct m4 result = { 0 };

    for (u32 i = 0; i < 4; i++)
    {
        for (u32 j = 0; j < 4; j++)
        {
            result.m[i][j] =
                a.m[i][0] * b.m[0][j] +
                a.m[i][1] * b.m[1][j] +
                a.m[i][2] * b.m[2][j] +
                a.m[i][3] * b.m[3][j];
        }
    }

    return result;
}

struct m4 m4_mul_f32(struct m4 a, f32 b)
{
    struct m4 result = { 0 };

    for (u32 i = 0; i < 4; i++)
    {
        for (u32 j = 0; j < 4; j++)
        {
            result.m[i][j] = a.m[i][j] * b;
        }
    }

    return result;
}

struct v4 m4_mul_v4(struct m4 a, struct v4 b)
{
    struct v4 result = { 0 };

    result.x = a.m[0][0] * b.x + a.m[1][0] * b.y + a.m[2][0] * b.z + a.m[3][0]
        * b.w;
    result.y = a.m[0][1] * b.x + a.m[1][1] * b.y + a.m[2][1] * b.z + a.m[3][1]
        * b.w;
    result.z = a.m[0][2] * b.x + a.m[1][2] * b.y + a.m[2][2] * b.z + a.m[3][2]
        * b.w;
    result.w = a.m[0][3] * b.x + a.m[1][3] * b.y + a.m[2][3] * b.z + a.m[3][3]
        * b.w;

    return result;
}

struct m4 m4_inverse(struct m4 a)
{
    struct m4 result = { 0 };

    // Calculate matrix of minors
    result = m4_matrix_of_minors(a);

    // Calculate matrix of cofactors
    result = m4_matrix_of_cofactors(result);

    // Calculate adjugate
    result = m4_transpose(result);

    // Multiply by 1 / determinant
    f32 det = m4_determinant(a);

    if (det)
    {
        result = m4_mul_f32(result, 1.0f / det);
    }
    else
    {
        // Todo: error case
    }

    return result;
}

struct m4 m4_identity()
{
    struct m4 result = { 0.0f };

    for (s32 i = 0; i < 4; i++)
    {
        result.m[i][i] = 1.0f;
    }

    return result;
}

f32 v3_length(struct v3 v)
{
    f32 result = f32_sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

    return result;
}

f32 v3_dot(struct v3 a, struct v3 b)
{
    f32 result = a.x * b.x + a.y * b.y + a.z * b.z;

    return result;
}

struct v3 v3_cross(struct v3 a, struct v3 b)
{
    struct v3 result;

    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;

    return result;
}

struct v3 v3_normalize(struct v3 v)
{
    struct v3 result = { 0 };

    f32 length = v3_length(v);

    if (length)
    {
        result.x = v.x / length;
        result.y = v.y / length;
        result.z = v.z / length;
    }

    return result;
}

struct v3 v3_from_v2(struct v2 v, f32 z)
{
    struct v3 result = { 0 };

    result.x = v.x;
    result.y = v.y;
    result.z = z;

    return result;
}

b32 v3_equals(struct v3 a, struct v3 b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

f32 v2_length(struct v2 v)
{
    f32 result = f32_sqrt(v.x * v.x + v.y * v.y);

    return result;
}

f32 v2_length_squared(struct v2 v)
{
    f32 result = v.x * v.x + v.y * v.y;

    return result;
}

f32 v2_dot(struct v2 a, struct v2 b)
{
    f32 result = a.x * b.x + a.y * b.y;

    return result;
}

f32 v2_cross(struct v2 a, struct v2 b)
{
    f32 result = a.x * b.y - a.y * b.x;

    return result;
}

b32 v2_is_zero(struct v2 v)
{
    f32 result = v.x == 0.0f && v.y == 0.0f;

    return result;
}

f32 v2_distance(struct v2 a, struct v2 b)
{
    f32 result = f32_distance(a.x, a.y, b.x, b.y);

    return result;
}

f32 v2_angle(struct v2 a, struct v2 b)
{
    f32 result = f32_acos(v2_dot(a, b) / (v2_length(a) * v2_length(b)));

    return result;
}

f32 f32_angle(f32 ax, f32 ay, f32 bx, f32 by)
{
    f32 result;

    struct v2 a = { ax, ay };
    struct v2 b = { bx, by };

    result = v2_angle(a, b);

    return result;
}

struct v2 v2_normalize(struct v2 v)
{
    struct v3 temp;
    struct v2 result;

    temp.x = v.x;
    temp.y = v.y;
    temp.z = 0.0f;

    temp = v3_normalize(temp);

    result.x = temp.x;
    result.y = temp.y;

    return result;
}

struct v2 v2_direction(struct v2 start, struct v2 end)
{
    struct v2 result;

    result.x = end.x - start.x;
    result.y = end.y - start.y;

    return result;
}

struct v2 v2_direction_from_angle(f32 angle)
{
    struct v2 result;

    result.x = f32_cos(angle);
    result.y = f32_sin(angle);

    return result;
}

b32 v2_equals(struct v2 a, struct v2 b)
{
    return a.x == b.x && a.y == b.y;
}

struct v2 v2_mul_f32(struct v2 a, f32 b)
{
    struct v2 result;

    result.x = a.x * b;
    result.y = a.y * b;

    return result;
}

void v2_swap(struct v2* a, struct v2* b)
{
    struct v2 t = *a;
    *a = *b;
    *b = t;
}

struct v2 v2_sub(struct v2 a, struct v2 b)
{
    struct v2 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

struct v2 v2_add(struct v2 a, struct v2 b)
{
    struct v2 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

struct v2 v2_mul(struct v2 a, struct v2 b)
{
    struct v2 result;

    result.x = a.x * b.x;
    result.y = a.y * b.y;

    return result;
}

struct v2 v2_rotate(struct v2 a, f32 angle)
{
    struct v2 result;

    struct m4 rotate = m4_rotate_z(angle);
    struct v4 temp = { a.x, a.y, 0.0f, 1.0f };

    temp = m4_mul_v4(rotate, temp);

    result.x = temp.x;
    result.y = temp.y;

    return result;
}

struct m4 m4_perspective(f32 fov, f32 aspect, f32 near, f32 far)
{
    f32 t = f32_tan(f32_radians(fov) / 2.0f);

    // Todo: check if n-f is not zero
    // Todo: check if t*aspect is not zero
    struct m4 result =
    {{
        { 1.0f / (t * aspect), 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f / t, 0.0f, 0.0f },
        { 0.0f, 0.0f, (far + near) / (near - far), -1.0f },
        { 0.0f, 0.0f, (2.0f * far * near) / (near - far), 0.0f }
    }};

    return result;
}

struct m4 m4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near,
    f32 far)
{
    f32 tx = -(right + left) / (right - left);
    f32 ty = -(top + bottom) / (top - bottom);
    f32 tz = -(far + near) / (far - near);

    struct m4 result =
    {{
        { 2.0f / (right - left), 0.0f, 0.0f, 0 },
        { 0.0f, 2.0f / (top - bottom), 0.0f, 0 },
        { 0.0f, 0.0f, -2.0f / (far - near), 0 },
        { tx, ty, tz, 1.0f }
    }};

    return result;
}

struct m4 m4_look_at(struct v3 position, struct v3 target, struct v3 up)
{
    struct v3 forward =
    {
        position.x - target.x,
        position.y - target.y,
        position.z - target.z
    };

    forward = v3_normalize(forward);
    up = v3_normalize(up);

    struct v3 right = v3_cross(up, forward);
    up = v3_cross(forward, right);

    struct m4 result = m4_identity();
    result.m[0][0] = right.x;
    result.m[1][0] = right.y;
    result.m[2][0] = right.z;
    result.m[0][1] = up.x;
    result.m[1][1] = up.y;
    result.m[2][1] = up.z;
    result.m[0][2] = forward.x;
    result.m[1][2] = forward.y;
    result.m[2][2] = forward.z;
    result.m[3][0] = -v3_dot(right, position);
    result.m[3][1] = -v3_dot(up, position);
    result.m[3][2] = -v3_dot(forward, position);

    return result;
}

struct v4 v4_mul(struct v4 a, struct v4 b)
{
    struct v4 result;

    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;
    result.w = a.w * b.w;

    return result;
}

f32 f32_triangle_area_signed(struct v2 a, struct v2 b, struct v2 c)
{
    f32 result;

    result = v2_cross(
        (struct v2){ b.x - a.x, b.y - a.y },
        (struct v2){ c.x - a.x, c.y - a.y });

    return result;
}
