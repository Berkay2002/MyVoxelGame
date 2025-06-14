#include "Player.h"
#include <iostream>
#include <algorithm>

// Player constants
const float GRAVITY = 25.0f;
const float PLAYER_SPEED = 5.0f;
const float JUMP_FORCE = 9.0f;
const float PLAYER_WIDTH = 0.6f;
const float PLAYER_HEIGHT = 1.8f;

// --- NYTT: Konstruktor för Player ---
Player::Player(const glm::vec3& position)
    : position(position), velocity(0.0f), isOnGround(false), selectedHotbarSlot(0) { // <-- Initiera vald slot
    
    // --- NYTT: Fyll på hotbar med startblock ---
    hotbar.resize(9, BlockType::Air); // 9 platser
    hotbar[0] = BlockType::Stone;
    hotbar[1] = BlockType::Dirt;
    hotbar[2] = BlockType::Grass;
    hotbar[3] = BlockType::Wood;
    hotbar[4] = BlockType::Leaves;
    hotbar[5] = BlockType::Sand;
}

// --- NYTT: Byt aktiv plats i hotbar ---
void Player::changeHotbarSlot(int offset) {
    selectedHotbarSlot -= offset; // Scrollhjulet är ofta inverterat
    
    // Rulla runt
    if (selectedHotbarSlot >= static_cast<int>(hotbar.size())) selectedHotbarSlot = 0;
    if (selectedHotbarSlot >= hotbar.size()) selectedHotbarSlot = 0;
}

// --- NYTT: Hämta det valda blocket från hotbar ---
BlockType Player::getSelectedBlock() const {
    if (selectedHotbarSlot >= 0 && selectedHotbarSlot < hotbar.size()) {
        return hotbar[selectedHotbarSlot];
    }
    return BlockType::Air;
}

void Player::jump() {
    if (isOnGround) {
        velocity.y = JUMP_FORCE;
        isOnGround = false;
    }
}

void Player::move(const glm::vec3& direction) {
    velocity.x = direction.x * PLAYER_SPEED;
    velocity.z = direction.z * PLAYER_SPEED;
}

void Player::applyGravity(float deltaTime) {
    if (!isOnGround) {
        velocity.y -= GRAVITY * deltaTime;
    }
}

void Player::update(float deltaTime, World& world) {
    applyGravity(deltaTime);

    float dx = velocity.x * deltaTime;
    float dy = velocity.y * deltaTime;
    float dz = velocity.z * deltaTime;

    boundingBox.min = position - glm::vec3(PLAYER_WIDTH / 2, 0, PLAYER_WIDTH / 2);
    boundingBox.max = position + glm::vec3(PLAYER_WIDTH / 2, PLAYER_HEIGHT, PLAYER_WIDTH / 2);
    
    isOnGround = false;

    collideWithWorldY(world, dy);
    collideWithWorldX(world, dx);
    collideWithWorldZ(world, dz);
}

void Player::collideWithWorldY(World& world, float dy) {
    for (int x = static_cast<int>(floor(boundingBox.min.x)); x <= static_cast<int>(floor(boundingBox.max.x)); ++x) {
        for (int z = static_cast<int>(floor(boundingBox.min.z)); z <= static_cast<int>(floor(boundingBox.max.z)); ++z) {
            if (dy < 0 && world.getBlock(x, static_cast<int>(floor(boundingBox.min.y + dy)), z) != BlockType::Air) {
                position.y = (float)floor(boundingBox.min.y + dy) + 1.0f;
                velocity.y = 0;
                isOnGround = true;
                return;
            }
            if (dy > 0 && world.getBlock(x, static_cast<int>(floor(boundingBox.max.y + dy)), z) != BlockType::Air) {
                position.y = (float)floor(boundingBox.max.y + dy) - PLAYER_HEIGHT;
                velocity.y = 0;
                return;
            }
        }
    }
    position.y += dy;
}

void Player::collideWithWorldX(World& world, float dx) {
    for (int y = static_cast<int>(floor(boundingBox.min.y)); y <= static_cast<int>(floor(boundingBox.max.y)); ++y) {
        for (int z = static_cast<int>(floor(boundingBox.min.z)); z <= static_cast<int>(floor(boundingBox.max.z)); ++z) {
            if (dx > 0 && world.getBlock(static_cast<int>(floor(boundingBox.max.x + dx)), y, z) != BlockType::Air) {
                position.x = (float)floor(boundingBox.max.x + dx) - PLAYER_WIDTH / 2.0f;
                velocity.x = 0;
                return;
            }
            if (dx < 0 && world.getBlock(static_cast<int>(floor(boundingBox.min.x + dx)), y, z) != BlockType::Air) {
                position.x = (float)floor(boundingBox.min.x + dx) + 1.0f + PLAYER_WIDTH / 2.0f;
                velocity.x = 0;
                return;
            }
        }
    }
    position.x += dx;
}

void Player::collideWithWorldZ(World& world, float dz) {
    for (int y = static_cast<int>(floor(boundingBox.min.y)); y <= static_cast<int>(floor(boundingBox.max.y)); ++y) {
        for (int x = static_cast<int>(floor(boundingBox.min.x)); x <= static_cast<int>(floor(boundingBox.max.x)); ++x) {
            if (dz > 0 && world.getBlock(x, y, static_cast<int>(floor(boundingBox.max.z + dz))) != BlockType::Air) {
                position.z = (float)floor(boundingBox.max.z + dz) - PLAYER_WIDTH / 2.0f;
                velocity.z = 0;
                return;
            }
            if (dz < 0 && world.getBlock(x, y, static_cast<int>(floor(boundingBox.min.z + dz))) != BlockType::Air) {
                position.z = (float)floor(boundingBox.min.z + dz) + 1.0f + PLAYER_WIDTH / 2.0f;
                velocity.z = 0;
                return;
            }
        }
    }
    position.z += dz;
}