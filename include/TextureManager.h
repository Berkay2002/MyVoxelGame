#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <glm/glm.hpp>
#include "Chunk.h" // For BlockType

// A central place to manage block texture coordinates.
class TextureManager {
public:
    // Gets coordinates for a specific face of a block.
    // face: 0=top, 1=side, 2=bottom
    static glm::ivec2 getBlockFaceTexture(BlockType type, int face);

    // Gets the single "icon" texture for UI elements.
    static glm::ivec2 getBlockIconTexture(BlockType type);
};

#endif // TEXTURE_MANAGER_H