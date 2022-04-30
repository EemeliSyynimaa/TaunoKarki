#version 330

in vec2 texcoords;
in vec4 color;
in float texture_index;
out vec4 fragColor;
uniform sampler2DArray uni_texture;

void main()
{
    vec4 texture_color = texture(uni_texture, vec3(texcoords.x, -texcoords.y,
        texture_index));

    if (texture_color.a == 0.0)
        fragColor = color;
    else
        fragColor = texture_color;
}
