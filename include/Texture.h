#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

class Texture {
public:
    Texture(const std::string& path);
    ~Texture();

    void bind(unsigned int slot = 0) const;
    void unbind() const;

private:
    unsigned int m_renderer_id;
    std::string m_file_path;
    int m_width, m_height, m_bpp; // Bits per pixel
};

#endif // TEXTURE_H