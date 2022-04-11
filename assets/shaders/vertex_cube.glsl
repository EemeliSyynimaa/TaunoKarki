#version 330

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormals;
layout(location = 2) in vec2 inTexcoords;
layout(location = 3) in mat4 inModel;
layout(location = 7) in vec4 inColor;
layout(location = 8) in uint inTexture;

uniform mat4 uniform_vp;

out vec2 texcoords;
out vec4 color;
out float texture_index;

void main()
{
	gl_Position = uniform_vp * inModel * vec4(inPosition, 1.0);
	texcoords = inTexcoords;
	color = inColor;

	// Choose texture based on cube normal (different texture for each side)
	if (inNormals.z == 1)
		texture_index = inTexture;
	else if (inNormals.z == -1)
		texture_index = 0;
	else if (inNormals.x == -1)
		texture_index = 1;
	else if (inNormals.x == 1)
		texture_index = 1;
	else if (inNormals.y == 1)
		texture_index = 0;
	else if (inNormals.y == -1)
		texture_index = 0;
}
