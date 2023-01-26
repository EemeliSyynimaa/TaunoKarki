#version 330

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexcoords;
layout(location = 2) in mat4 inModel;
layout(location = 6) in vec4 inColor;
layout(location = 7) in uint inTextureIndex;

uniform mat4 uniform_vp;

out vec2 texcoords;
out vec4 color;
out float texture_index;

void main()
{
    gl_Position = uniform_vp * inModel * vec4(inPosition, 0.0, 1.0);
    texture_index = inTextureIndex;
    texcoords = inTexcoords;
    color = inColor;
}
