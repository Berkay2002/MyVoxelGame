#ifndef CHUNK_H
#define CHUNK_H

#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>

enum class BlockType : uint8_t {
    Air = 0, Stone, Dirt, Grass, Wood, Leaves, Water, Sand
};

class Chunk {
public:
    static const int CHUNK_WIDTH = 16, CHUNK_HEIGHT = 16, CHUNK_DEPTH = 16;

    Chunk(const glm::ivec2& position);
    ~Chunk();

    void generateMesh();
    void draw() const;
    BlockType getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, BlockType type); // <-- NY: För att ändra ett block

private:
    unsigned int m_vao;
    unsigned int m_vbo;
    int m_vertex_count;
    glm::ivec2 m_position; 

    BlockType m_blocks[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_DEPTH];
};

#endif // CHUNK_H