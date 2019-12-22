#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>
#include <map>
#include <sstream>

#include "glm\glm.hpp"

struct Vertex
{
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
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

    void indexVBO(std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals);
};

#endif