#include "TextureManager.h"

// The single, definitive function for getting face textures.
glm::ivec2 TextureManager::getBlockFaceTexture(BlockType type, int face) {
    // face: 0=top, 1=side, 2=bottom
    switch (type) {
        case BlockType::Grass:
            if (face == 0) return { 8, 0 };  // grass_top
            if (face == 2) return { 6, 0 };  // dirt
            return { 7, 4 };                 // grass_side
        case BlockType::Dirt:
            return { 6, 0 };
        case BlockType::Wood:
            if (face == 0 || face == 2) return { 1, 1 }; // wood_top
            return { 1, 0 };                 // wood_side
        case BlockType::Leaves:
            return { 4, 8 };
        case BlockType::Stone:
            return { 3, 4 };
        case BlockType::Water:
            return { 7, 9 };
        case BlockType::Sand:
            return { 7, 7 };
        default:
            return { 3, 4 }; // Fallback to stone
    }
}

// The single, definitive function for getting icon textures for the UI.
glm::ivec2 TextureManager::getBlockIconTexture(BlockType type) {
    switch (type) {
        // For most blocks, the icon is the side texture.
        case BlockType::Grass: return { 7, 4 }; // Use grass_side for the icon
        case BlockType::Dirt:  return { 6, 0 };
        case BlockType::Wood:  return { 1, 0 };
        case BlockType::Leaves:return { 4, 8 };
        case BlockType::Stone: return { 3, 4 };
        case BlockType::Water: return { 7, 9 };
        case BlockType::Sand:  return { 7, 7 };
        default:
            return { 0, 0 }; // Default for Air, etc.
    }
}