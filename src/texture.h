#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <vector>
#include "GL/glew.h"

class Texture
{
public:
    Texture(GLenum target, const std::string& path);
    ~Texture();

    void bind(GLenum target);

    GLuint getID() { return ID; }
private:
    GLuint ID;
    GLuint width;
    GLuint height;
    GLenum target;
};

#endif