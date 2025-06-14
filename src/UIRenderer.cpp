#include "UIRenderer.h"
#include "TextureManager.h" // <-- NYTT: Inkludera den nya managern
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

UIRenderer::UIRenderer(unsigned int screenWidth, unsigned int screenHeight)
    : m_shader("assets/shaders/ui.vert", "assets/shaders/ui.frag")
{
    m_projection = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight);
    initRenderData();
}

UIRenderer::~UIRenderer() {
    glDeleteVertexArrays(1, &m_quadVAO);
}

void UIRenderer::initRenderData() {
    unsigned int VBO;
    float vertices[] = { 
        // pos      // tex
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(m_quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// --------------------------------------------------------------------------
// Den gamla "getTextureGridCoords" funktionen har tagits bort från den här filen.
// All logik för texturer finns nu i TextureManager.
// --------------------------------------------------------------------------


void UIRenderer::drawHotbar(const Player& player, Texture& textureAtlas) {
    m_shader.use();
    m_shader.setMat4("projection", m_projection);
    
    const float COLS   = 9.0f;
    const float ROWS   = 10.0f;
    const float STEP_U = 1.0f / COLS;    const float STEP_V = 1.0f / ROWS;
    
    float screenWidth = 1600;
    float startX = (screenWidth - HOTBAR_WIDTH) / 2.0f;
    float startY = 20.0f;
    
    textureAtlas.bind();
    for (size_t i = 0; i < player.hotbar.size(); ++i) {
        BlockType block = player.hotbar[i];
        if (block != BlockType::Air) {
            glm::vec2 itemPos(startX + (i * 20 * HOTBAR_SCALE) + (20 * HOTBAR_SCALE - 16 * HOTBAR_SCALE) / 2, startY + (20 * HOTBAR_SCALE - 16 * HOTBAR_SCALE) / 2);
            glm::vec2 itemSize(16 * HOTBAR_SCALE, 16 * HOTBAR_SCALE);
            
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(itemPos, 0.0f));
            model = glm::scale(model, glm::vec3(itemSize, 1.0f));
            
            m_shader.setMat4("model", model);
            
            // === KORRIGERING: Använd den nya TextureManager ===
            glm::ivec2 gridPos = TextureManager::getBlockIconTexture(block);
            float uo = gridPos.x * STEP_U;
            float vo = (ROWS - 1.0f - gridPos.y) * STEP_V;
            m_shader.setVec3("uv_transform", glm::vec3(uo, vo, STEP_U));

            glBindVertexArray(m_quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
    
    m_shader.setVec3("uv_transform", glm::vec3(-1.0f, 0.0f, 0.0f));
    m_shader.setVec3("color", glm::vec3(1.0f, 1.0f, 1.0f));

    glm::vec2 selectorPos(startX + (player.selectedHotbarSlot * 20 * HOTBAR_SCALE), startY);
    glm::vec2 selectorSize(22 * HOTBAR_SCALE, 22 * HOTBAR_SCALE);

    glLineWidth(4.0f);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(selectorPos, 0.0f));
    model = glm::scale(model, glm::vec3(selectorSize, 1.0f));
    m_shader.setMat4("model", model);
    
    // Använd bara 4 hörn för att rita en fyrkant utan diagonal linje
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    
    glLineWidth(2.5f);
}

void UIRenderer::drawCrosshair() {
    m_shader.use();
    m_shader.setMat4("projection", m_projection);

    float screenWidth = 1600;
    float screenHeight = 900;
    float crosshairSize = 2.0f;
    float crosshairLength = 20.0f;    m_shader.setVec3("uv_transform", glm::vec3(-1.0f, 0.0f, 0.0f));
    m_shader.setVec3("color", glm::vec3(1.0f, 1.0f, 1.0f));

    // Rita horisontell linje
    glm::vec2 h_pos( (screenWidth - crosshairLength) / 2.0f, (screenHeight - crosshairSize) / 2.0f);
    glm::mat4 h_model = glm::mat4(1.0f);
    h_model = glm::translate(h_model, glm::vec3(h_pos, 0.0f));
    h_model = glm::scale(h_model, glm::vec3(crosshairLength, crosshairSize, 1.0f));
    m_shader.setMat4("model", h_model);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Rita vertikal linje
    glm::vec2 v_pos( (screenWidth - crosshairSize) / 2.0f, (screenHeight - crosshairLength) / 2.0f);
    glm::mat4 v_model = glm::mat4(1.0f);
    v_model = glm::translate(v_model, glm::vec3(v_pos, 0.0f));
    v_model = glm::scale(v_model, glm::vec3(crosshairSize, crosshairLength, 1.0f));
    m_shader.setMat4("model", v_model);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}