#version 330

in vec2 texcoords;
in vec4 color;
in float texture_index;
out vec4 fragColor;
uniform sampler2DArray uniform_texture;

void main()
{
    vec4 result = texture(uniform_texture, vec3(texcoords.x, -texcoords.y,
        texture_index)) * color;
    if (result.a == 0)
    {
        discard;
    }
    else
    {
        fragColor = result;
    }
}
