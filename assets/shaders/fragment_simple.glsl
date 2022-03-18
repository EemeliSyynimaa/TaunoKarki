#version 330

in vec2 texcoords;
in vec3 normal;
in vec4 color;
out vec4 fragColor;
uniform vec4 uniform_color;

void main()
{
	fragColor = color * uniform_color;
}
