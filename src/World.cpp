#include "World.h"
#include "Chunk.h"
#include <iostream>
#include <cmath>

long long pack_key(int x, int z) {
    return ((long long)x << 32) | (unsigned int)z;
}

World::World() : m_lastPlayerChunkPos(9999, 9999) {}

World::~World() {
    for (auto const& [key, val] : m_chunks) {
        delete val;
    }
}

void World::update(const glm::vec3& playerPosition) {
    glm::ivec2 currentPlayerChunkPos = {
        floor(playerPosition.x / Chunk::CHUNK_WIDTH),
        floor(playerPosition.z / Chunk::CHUNK_DEPTH)
    };
    if (currentPlayerChunkPos != m_lastPlayerChunkPos) {
        loadChunks(playerPosition);
        unloadChunks(playerPosition);
        m_lastPlayerChunkPos = currentPlayerChunkPos;
    }
}

void World::loadChunks(const glm::vec3& playerPosition) {
    glm::ivec2 centerChunk = {
        floor(playerPosition.x / Chunk::CHUNK_WIDTH),
        floor(playerPosition.z / Chunk::CHUNK_DEPTH)
    };
    for (int x = centerChunk.x - m_renderDistance; x <= centerChunk.x + m_renderDistance; ++x) {
        for (int z = centerChunk.y - m_renderDistance; z <= centerChunk.y + m_renderDistance; ++z) {
            long long key = pack_key(x, z);
            if (m_chunks.find(key) == m_chunks.end()) {
                m_chunks[key] = new Chunk({x, z});
            }
        }
    }
}

void World::unloadChunks(const glm::vec3& playerPosition) {
    glm::ivec2 centerChunk = {
        floor(playerPosition.x / Chunk::CHUNK_WIDTH),
        floor(playerPosition.z / Chunk::CHUNK_DEPTH)
    };
    std::vector<long long> keysToErase;
    for (auto const& [key, chunk] : m_chunks) {
        glm::ivec2 chunkPos = { (int)(key >> 32), (int)key };
        if (abs(chunkPos.x - centerChunk.x) > m_renderDistance || 
            abs(chunkPos.y - centerChunk.y) > m_renderDistance) {
            keysToErase.push_back(key);
        }
    }
    for (long long key : keysToErase) {
        delete m_chunks[key];
        m_chunks.erase(key);
    }
}

void World::draw() const {
    for (auto const& [key, val] : m_chunks) {
        val->draw();
    }
}

BlockType World::getBlock(int x, int y, int z) const {
    if (y < 0 || y >= Chunk::CHUNK_HEIGHT) return BlockType::Air;
    int chunkX = static_cast<int>(floor(static_cast<float>(x) / Chunk::CHUNK_WIDTH));
    int chunkZ = static_cast<int>(floor(static_cast<float>(z) / Chunk::CHUNK_DEPTH));
    long long key = pack_key(chunkX, chunkZ);
    auto it = m_chunks.find(key);
    if (it == m_chunks.end()) return BlockType::Air;
    int localX = x - chunkX * Chunk::CHUNK_WIDTH;
    int localZ = z - chunkZ * Chunk::CHUNK_DEPTH;
    return it->second->getBlock(localX, y, localZ);
}

// --- NY: Implementation för att ändra ett block ---
void World::setBlock(int x, int y, int z, BlockType type) {
    if (y < 0 || y >= Chunk::CHUNK_HEIGHT) return;
    int chunkX = static_cast<int>(floor(static_cast<float>(x) / Chunk::CHUNK_WIDTH));
    int chunkZ = static_cast<int>(floor(static_cast<float>(z) / Chunk::CHUNK_DEPTH));
    long long key = pack_key(chunkX, chunkZ);
    auto it = m_chunks.find(key);
    if (it != m_chunks.end()) {
        int localX = x - chunkX * Chunk::CHUNK_WIDTH;
        int localZ = z - chunkZ * Chunk::CHUNK_DEPTH;
        it->second->setBlock(localX, y, localZ, type);
    }
}

// --- NY: Implementation för Raycasting ---
bool World::raycast(const glm::vec3& start, const glm::vec3& direction, float maxDist, glm::ivec3& out_block, glm::ivec3& out_prev) {
    glm::vec3 dir = normalize(direction);
    glm::vec3 pos = start;
    
    // Current voxel coordinates
    glm::ivec3 voxel = glm::ivec3(floor(pos));
    
    // Direction to increment voxel coordinates
    glm::ivec3 step = glm::ivec3(
        dir.x > 0 ? 1 : -1,
        dir.y > 0 ? 1 : -1,
        dir.z > 0 ? 1 : -1
    );
    
    // Calculate delta distances
    glm::vec3 deltaDist = glm::vec3(
        abs(1.0f / dir.x),
        abs(1.0f / dir.y),
        abs(1.0f / dir.z)
    );
    
    // Calculate initial side distances
    glm::vec3 sideDist;
    if (dir.x < 0) {
        sideDist.x = (pos.x - voxel.x) * deltaDist.x;
    } else {
        sideDist.x = (voxel.x + 1.0f - pos.x) * deltaDist.x;
    }
    if (dir.y < 0) {
        sideDist.y = (pos.y - voxel.y) * deltaDist.y;
    } else {
        sideDist.y = (voxel.y + 1.0f - pos.y) * deltaDist.y;
    }
    if (dir.z < 0) {
        sideDist.z = (pos.z - voxel.z) * deltaDist.z;
    } else {
        sideDist.z = (voxel.z + 1.0f - pos.z) * deltaDist.z;
    }
    
    float currentDist = 0.0f;
    glm::ivec3 prevVoxel = voxel;
    
    // DDA traversal
    while (currentDist < maxDist) {
        // Check current voxel
        if (getBlock(voxel.x, voxel.y, voxel.z) != BlockType::Air) {
            out_block = voxel;
            out_prev = prevVoxel;
            return true;
        }
        
        prevVoxel = voxel;
        
        // Move to next voxel
        if (sideDist.x < sideDist.y && sideDist.x < sideDist.z) {
            sideDist.x += deltaDist.x;
            voxel.x += step.x;
            currentDist = sideDist.x - deltaDist.x;
        } else if (sideDist.y < sideDist.z) {
            sideDist.y += deltaDist.y;
            voxel.y += step.y;
            currentDist = sideDist.y - deltaDist.y;
        } else {
            sideDist.z += deltaDist.z;
            voxel.z += step.z;
            currentDist = sideDist.z - deltaDist.z;
        }
    }
    
    return false;
}