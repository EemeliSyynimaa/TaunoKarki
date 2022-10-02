#version 330

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormals;
layout(location = 2) in vec2 inTexcoords;
layout(location = 3) in vec4 inColor;
layout(location = 4) in float inTextureIndex;

uniform mat4 uniform_mvp;

out vec2 texcoords;
out vec4 color;
out float texture_index;

void main()
{
    gl_Position = uniform_vp * inModel * vec4(inPosition, 1.0);
    texture_index = inTextureIndex;
    texcoords = inTexcoords;
    color = inColor;
}
