#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include "component.h"
#include "mesh.h"
#include "texture.h"
#include "transform.h"
#include "shaderprogram.h"
#include "glm/glm.hpp"

class MeshRenderer : public Component
{
public:
    MeshRenderer(GameObject* owner);
    ~MeshRenderer();

    void update();
    void setMesh(Mesh* mesh);
    void setTexture(Texture* texture);
    void setProgram(ShaderProgram* program);
    void setViewMatrix(glm::mat4& matrix);
    void setProjectionMatrix(glm::mat4& matrix);
private:
    Mesh* mesh;
    Texture* texture;
    ShaderProgram* program;
    Transform* transform;

    glm::mat4* projectionMatrix;
    glm::mat4* viewMatrix;

    GLuint VBO;
    GLuint IBO;
    GLint textureIndex;
    GLint MVPIndex;
};

#endif