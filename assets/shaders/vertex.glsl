#version 330

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoords;

uniform mat4 MVP;
uniform vec4 uniform_color;
out vec2 texcoords;
out vec4 color;

void main()
{
    gl_Position = MVP * vec4(inPosition, 1.0);
    texcoords = inTexcoords;
    color = uniform_color;
}
