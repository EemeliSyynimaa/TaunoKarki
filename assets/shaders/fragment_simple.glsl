#version 330

in vec2 texcoords;
in vec3 normal;
out vec4 fragColor;
uniform vec4 color;

void main()
{
	fragColor = color;
}
