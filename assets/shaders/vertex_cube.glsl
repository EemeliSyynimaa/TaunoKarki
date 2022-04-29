#version 330

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormals;
layout(location = 2) in vec2 inTexcoords;
layout(location = 3) in mat4 inModel;
layout(location = 7) in uvec3 inFace1;
layout(location = 8) in uvec3 inFace2;
layout(location = 9) in uvec3 inFace3;
layout(location = 10) in uvec3 inFace4;
layout(location = 11) in uvec3 inFace5;
layout(location = 12) in uvec3 inFace6;

uniform mat4 uniform_vp;

out vec2 texcoords;
out vec4 color;
out float texture_index;

layout(std140) uniform uniform_colors
{
    vec4 colors[4096];
};

void main()
{
    gl_Position = uniform_vp * inModel * vec4(inPosition, 1.0);

    // Choose texture based on cube normal (different texture for each side)
    uvec3 face;

    if (inNormals.z == 1)
        face = inFace1;
    else if (inNormals.z == -1)
        face = inFace2;
    else if (inNormals.x == -1)
        face = inFace3;
    else if (inNormals.x == 1)
        face = inFace4;
    else if (inNormals.y == 1)
        face = inFace5;
    else if (inNormals.y == -1)
        face = inFace6;

    texture_index = face.x;
    float angle = radians(face.y * 90.0f);
    float c = cos(angle);
    float s = sin(angle);

    texcoords.x = c * inTexcoords.x - s * inTexcoords.y;
    texcoords.y = s * inTexcoords.x + c * inTexcoords.y;

    color = colors[face.z];
}
