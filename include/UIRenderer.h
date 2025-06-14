#ifndef UI_RENDERER_H
#define UI_RENDERER_H

#include "Shader.h"
#include "Player.h"
#include "Texture.h"
#include <glm/glm.hpp>

class UIRenderer {
public:
    UIRenderer(unsigned int screenWidth, unsigned int screenHeight);
    ~UIRenderer();

    void drawHotbar(const Player& player, Texture& textureAtlas);
    void drawCrosshair();

private:
    void initRenderData();
    // En hjälpfunktion för att få texturkoordinater, liknande den i Chunk.cpp
    glm::ivec2 getTextureGridCoords(BlockType type);

    Shader m_shader;
    unsigned int m_quadVAO;
    glm::mat4 m_projection;

    // Konstanter för UI-layout
    const float HOTBAR_ASPECT = 91.0f / 11.0f;
    const float HOTBAR_SCALE = 4.0f;
    const float HOTBAR_WIDTH = 22 * HOTBAR_SCALE * 9;
    const float HOTBAR_HEIGHT = 22 * HOTBAR_SCALE;
};

#endif // UI_RENDERER_H