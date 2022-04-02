#version 330

in vec2 texcoords;
in vec4 color;
out vec4 fragColor;
uniform sampler2DArray uni_texture;

void main()
{
	fragColor = texture(uni_texture, vec3(texcoords.x, -texcoords.y, 13)) * color;
}
