#ifndef TILEMAP_H
#define TILEMAP_H

#include <vector>
#include "assetmanager.h"

class Tilemap
{
public:
    Tilemap(glm::vec3 position, AssetManager& assetManager);
    ~Tilemap();

    void draw(glm::mat4& view, glm::mat4& perspective);
    void generate(unsigned int width, unsigned int height);
    glm::vec3 getStartingPosition();
    size_t getNumberOfStartingPositions() { return startingPositions.size(); }
    unsigned short** getData() const { return data; }
    unsigned int getWidth() const { return width; }
    unsigned int getHeight() const { return height; }
    glm::vec3 getRandomFreePosition();
    bool isPositionFree(unsigned int x, unsigned int y);
private:
    unsigned short** data;
    unsigned int width;
    unsigned int height;
    unsigned short currentRegion;
    int windingPercent;

    AssetManager& assetManager;
    ShaderProgram& program;
    Texture& texture;
    glm::vec3 position;

    std::vector<GLuint> indices;
    std::vector<Vertex> vertices;
    std::vector<glm::vec3> startingPositions;

    GLuint VAO;
    GLuint VBO;
    GLuint IBO;
    GLint textureIndex;
    GLint MVPIndex;
    glm::mat4 matrix;

    void startRegion() { currentRegion++; };
    void addRooms();
    bool roomConflictsWithOthers(unsigned int roomX, unsigned int roomY, unsigned int roomW, unsigned int roomH);
    void carveRoom(unsigned int roomX, unsigned int roomY, unsigned int roomW, unsigned int roomH);
    void growMaze(glm::uvec2 pos);
    void connectRegions();
    void removeDeadEnds();
    void openClosedAreas();
    void carve(unsigned short x, unsigned short y) { data[y][x] = currentRegion; }
    void carve(glm::uvec2 pos) { carve(pos.x, pos.y); }
    bool canCarve(glm::uvec2 pos, glm::uvec2 dir);
    bool checkTile(size_t x, size_t y, size_t tile) { return data[y][x] == tile; };
    void init(unsigned int width, unsigned int height);
    void createMeshes();
};

#endif