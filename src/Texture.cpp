#include "Texture.h"
#include <glad/glad.h>

// This tells the stb_image header to include the implementation code
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

Texture::Texture(const std::string& path)
    : m_renderer_id(0), m_file_path(path), m_width(0), m_height(0), m_bpp(0) {

    // Flip the image vertically because OpenGL expects the 0.0 y-coordinate to be at the bottom
    stbi_set_flip_vertically_on_load(1);
    
    // Load the image data
    unsigned char* local_buffer = stbi_load(path.c_str(), &m_width, &m_height, &m_bpp, 4); // 4 = RGBA

    if (!local_buffer) {
        std::cerr << "Error: Failed to load texture: " << path << std::endl;
        return;
    }

    glGenTextures(1, &m_renderer_id);
    glBindTexture(GL_TEXTURE_2D, m_renderer_id);

    // Set texture parameters
    // Use GL_NEAREST for the pixelated, Minecraft-like look
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create the OpenGL texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, local_buffer);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Free the CPU memory now that the texture is on the GPU
    if (local_buffer) {
        stbi_image_free(local_buffer);
    }
}

Texture::~Texture() {
    glDeleteTextures(1, &m_renderer_id);
}

void Texture::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_renderer_id);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}