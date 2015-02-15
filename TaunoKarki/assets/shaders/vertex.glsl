
#version 330

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec2 inTexcoords;

uniform mat4 MVP;
out vec2 texcoords;

void main()
{
	gl_Position = inPosition;
	texcoords = inTexcoords;
}
