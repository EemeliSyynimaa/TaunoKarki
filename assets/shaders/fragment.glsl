#version 330

in vec2 texcoords;
in vec4 color;
out vec4 fragColor;
uniform sampler2D texture;

void main()
{
	fragColor = texture2D(texture, vec2(texcoords.x, -texcoords.y)) * color;
}
