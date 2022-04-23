#version 330

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormals;
layout(location = 2) in vec2 inTexcoords;
layout(location = 3) in mat4 inModel;
layout(location = 7) in vec4 inColor;
layout(location = 8) in uvec2 inTexture1;
layout(location = 9) in uvec2 inTexture2;
layout(location = 10) in uvec2 inTexture3;
layout(location = 11) in uvec2 inTexture4;
layout(location = 12) in uvec2 inTexture5;
layout(location = 13) in uvec2 inTexture6;

uniform mat4 uniform_vp;

out vec2 texcoords;
out vec4 color;
out float texture_index;

void main()
{
	gl_Position = uniform_vp * inModel * vec4(inPosition, 1.0);
	color = inColor;

	// Choose texture based on cube normal (different texture for each side)
	uvec2 texture_used;

	if (inNormals.z == 1)
		texture_used = inTexture1;
	else if (inNormals.z == -1)
		texture_used = inTexture2;
	else if (inNormals.x == -1)
		texture_used = inTexture3;
	else if (inNormals.x == 1)
		texture_used = inTexture4;
	else if (inNormals.y == 1)
		texture_used = inTexture5;
	else if (inNormals.y == -1)
		texture_used = inTexture6;

	texture_index = texture_used.x;

	float angle = radians(texture_used.y * 90.0f);

	float c = cos(angle);
	float s = sin(angle);

	texcoords.x = c * inTexcoords.x - s * inTexcoords.y;
	texcoords.y = s * inTexcoords.x + c * inTexcoords.y;
}
