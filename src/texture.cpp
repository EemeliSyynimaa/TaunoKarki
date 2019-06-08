#include "texture.h"
#include "lodepng.h"

Texture::Texture(GLenum target, const std::string& path) : target(target)
{
    std::vector<unsigned char> file;
    std::vector<unsigned char> pixels;

    lodepng::load_file(file, path);
    lodepng::decode(pixels, width, height, file.data(), file.size());

    glGenTextures(1, &ID);
    glBindTexture(target, ID);

    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(target, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

Texture::~Texture()
{
    glDeleteTextures(1, &ID);
}

void Texture::bind(GLenum textureUnit)
{
    glActiveTexture(textureUnit);
    glBindTexture(target, ID);
}