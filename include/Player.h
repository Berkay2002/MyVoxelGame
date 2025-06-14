#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include <vector> // Lade till vector
#include "World.h"
#include "Chunk.h" 

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

class Player {
public:
    Player(const glm::vec3& position);

    void update(float deltaTime, World& world);
    void jump();
    void move(const glm::vec3& direction);

    // --- NYTT: Inventariehantering ---
    void changeHotbarSlot(int offset);
    BlockType getSelectedBlock() const;

    glm::vec3 position;
    glm::vec3 velocity;
    AABB boundingBox;
    bool isOnGround;
    
    // --- NYTT: Inventariedata ---
    std::vector<BlockType> hotbar;
    int selectedHotbarSlot;


private:
    void applyGravity(float deltaTime);
    void collideWithWorldY(World& world, float dy);
    void collideWithWorldX(World& world, float dx);
    void collideWithWorldZ(World& world, float dz);
};

#endif // PLAYER_H