#version 330

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in mat4 inModel;
layout(location = 6) in vec4 inColor;

uniform mat4 uniform_vp;

out vec4 color;

void main()
{
	gl_Position = uniform_vp * inModel * vec4(inPosition, 1.0);
	color = inColor;
}
