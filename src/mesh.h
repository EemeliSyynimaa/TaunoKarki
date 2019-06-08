#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>
#include <map>
#include <sstream>

#include "GL\glew.h"
#include "glm\glm.hpp"

struct Vertex
{
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;

    bool operator<(const Vertex& other) const
    {
        return memcmp((void*)this, (void*)&other, sizeof(Vertex)) > 0;
    }
};

class Mesh
{
public:
    Mesh(const std::string& path);
    ~Mesh();

    std::vector<Vertex>& getVertices() { return vertices; }
    std::vector<GLuint>& getIndices() { return indices; }
private:
    std::vector<GLuint> indices;
    std::vector<Vertex> vertices;

    bool getSimilarVertexIndex(Vertex& vertex, std::map<Vertex, GLuint>& vertexToOutIndex, GLuint& result);
    void indexVBO(std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals);
};

#endif