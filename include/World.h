#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "Chunk.h"

class World {
public:
    World();
    ~World();

    void update(const glm::vec3& playerPosition);
    void draw() const;
    BlockType getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, BlockType type); // <-- NY: För att ändra block

    // NY: Raycasting-funktion
    bool raycast(const glm::vec3& start, const glm::vec3& direction, float maxDist, glm::ivec3& out_block, glm::ivec3& out_prev);

private:
    void loadChunks(const glm::vec3& playerPosition);
    void unloadChunks(const glm::vec3& playerPosition);

    int m_renderDistance = 4;
    glm::ivec2 m_lastPlayerChunkPos;

    std::map<long long, Chunk*> m_chunks; 
};

#endif // WORLD_H