#version 330

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoords;
layout(location = 3) in vec4 inColor;

uniform mat4 MVP;
out vec2 texcoords;
out vec4 color;

void main()
{
	gl_Position = MVP * vec4(inPosition, 1.0);
	texcoords = inTexcoords;
	color = inColor;
}
