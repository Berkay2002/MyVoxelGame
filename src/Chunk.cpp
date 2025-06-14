#include "Chunk.h"
#include "perlin.h"
#include "TextureManager.h" // <-- NYTT: Inkludera den nya managern
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <cmath>

// --------------------------------------------------------------------------
// Den gamla "getTextureGridCoords" funktionen har tagits bort från den här filen.
// All logik för texturer finns nu i TextureManager.
// --------------------------------------------------------------------------

// Denna array är korrekt och behöver inte ändras.
static const float face_vertices[] = {
    -0.5f,-0.5f,-0.5f, 0.0f,0.0f, 0.5f,-0.5f,-0.5f, 1.0f,0.0f, 0.5f, 0.5f,-0.5f, 1.0f,1.0f,
     0.5f, 0.5f,-0.5f, 1.0f,1.0f,-0.5f, 0.5f,-0.5f, 0.0f,1.0f,-0.5f,-0.5f,-0.5f, 0.0f,0.0f,
    -0.5f,-0.5f, 0.5f, 0.0f,0.0f, 0.5f, 0.5f, 0.5f, 1.0f,1.0f, 0.5f,-0.5f, 0.5f, 1.0f,0.0f,
     0.5f, 0.5f, 0.5f, 1.0f,1.0f,-0.5f,-0.5f, 0.5f, 0.0f,0.0f,-0.5f, 0.5f, 0.5f, 0.0f,1.0f,
    -0.5f, 0.5f, 0.5f, 1.0f,1.0f,-0.5f, 0.5f,-0.5f, 0.0f,1.0f,-0.5f,-0.5f,-0.5f, 0.0f,0.0f,
    -0.5f,-0.5f,-0.5f, 0.0f,0.0f,-0.5f,-0.5f, 0.5f, 1.0f,0.0f,-0.5f, 0.5f, 0.5f, 1.0f,1.0f,
     0.5f, 0.5f, 0.5f, 1.0f,1.0f, 0.5f,-0.5f,-0.5f, 0.0f,0.0f, 0.5f, 0.5f,-0.5f, 0.0f,1.0f,
     0.5f,-0.5f,-0.5f, 0.0f,0.0f, 0.5f, 0.5f, 0.5f, 1.0f,1.0f, 0.5f,-0.5f, 0.5f, 1.0f,0.0f,
    -0.5f,-0.5f,-0.5f, 0.0f,1.0f, 0.5f,-0.5f,-0.5f, 1.0f,1.0f, 0.5f,-0.5f, 0.5f, 1.0f,0.0f,
     0.5f,-0.5f, 0.5f, 1.0f,0.0f,-0.5f,-0.5f, 0.5f, 0.0f,0.0f,-0.5f,-0.5f,-0.5f, 0.0f,1.0f,
    -0.5f, 0.5f,-0.5f, 0.0f,1.0f, 0.5f, 0.5f, 0.5f, 1.0f,0.0f, 0.5f, 0.5f,-0.5f, 1.0f,1.0f,
     0.5f, 0.5f, 0.5f, 1.0f,0.0f,-0.5f, 0.5f,-0.5f, 0.0f,1.0f,-0.5f, 0.5f, 0.5f, 0.0f,0.0f
};

BlockType Chunk::getBlock(int x, int y, int z) const {
    if (x >= 0 && x < CHUNK_WIDTH && y >= 0 && y < CHUNK_HEIGHT && z >= 0 && z < CHUNK_DEPTH) {
        return m_blocks[x][y][z];
    }
    return BlockType::Air;
}

double FBM(Perlin& perlin, double x, double z) {
    double total = 0.0;
    double frequency = 0.02;
    double amplitude = 1.0;
    double persistence = 0.5;
    int octaves = 4;
    for(int i = 0; i < octaves; i++) {
        total += perlin.noise(x * frequency, z * frequency, 0.0) * amplitude;
        amplitude *= persistence;
        frequency *= 2.0;
    }
    return total;
}

Chunk::Chunk(const glm::ivec2& position)
  : m_vao(0), m_vbo(0), m_vertex_count(0), m_position(position)
{
    static Perlin perlin_terrain(12345);
    static Perlin perlin_trees(54321);

    const int seaLevel = 9;    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_DEPTH; ++z) {
            float real_x = static_cast<float>(x + m_position.x * CHUNK_WIDTH);
            float real_z = static_cast<float>(z + m_position.y * CHUNK_DEPTH);
            
            double noiseVal = FBM(perlin_terrain, real_x, real_z);
            int ground_level = 10 + static_cast<int>(noiseVal * 6.0f);

            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                if (ground_level < seaLevel) { 
                    if (y < ground_level) m_blocks[x][y][z] = BlockType::Stone;
                    else if (y == ground_level) m_blocks[x][y][z] = BlockType::Sand;
                    else if (y <= seaLevel) m_blocks[x][y][z] = BlockType::Water;
                    else m_blocks[x][y][z] = BlockType::Air;
                } else {
                    if (y < ground_level - 3) m_blocks[x][y][z] = BlockType::Stone;
                    else if (y < ground_level) m_blocks[x][y][z] = BlockType::Dirt;
                    else if (y == ground_level) m_blocks[x][y][z] = BlockType::Grass;
                    else m_blocks[x][y][z] = BlockType::Air;
                }
            }
        }
    }    for (int x = 2; x < CHUNK_WIDTH - 2; ++x) {
        for (int z = 2; z < CHUNK_DEPTH - 2; ++z) {
            float real_x = static_cast<float>(x + m_position.x * CHUNK_WIDTH);
            float real_z = static_cast<float>(z + m_position.y * CHUNK_DEPTH);
            double treeVal = perlin_trees.noise(real_x * 0.1, real_z * 0.1, 0.0);

            if (treeVal > 0.7) {
                int ground_level = 0;
                for(int y = CHUNK_HEIGHT - 1; y >= 0; --y) {
                    if(m_blocks[x][y][z] == BlockType::Grass) {
                        ground_level = y;
                        break;
                    }
                }
                if (ground_level > 0 && ground_level + 6 < CHUNK_HEIGHT) {
                    for (int i = 1; i <= 4; ++i) m_blocks[x][ground_level + i][z] = BlockType::Wood;
                    for (int y_off = 3; y_off <= 6; ++y_off)
                        for (int x_off = -2; x_off <= 2; ++x_off)
                        for (int z_off = -2; z_off <= 2; ++z_off)
                            if (x + x_off >= 0 && x + x_off < CHUNK_WIDTH && z + z_off >= 0 && z + z_off < CHUNK_DEPTH)
                                if (!(x_off == 0 && z_off == 0 && y_off <= 4))
                                    m_blocks[x + x_off][ground_level + y_off][z + z_off] = BlockType::Leaves;
                }
            }
        }
    }

    generateMesh();
}

Chunk::~Chunk() {
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
}

void Chunk::setBlock(int x, int y, int z, BlockType type) {
    if (x >= 0 && x < CHUNK_WIDTH && y >= 0 && y < CHUNK_HEIGHT && z >= 0 && z < CHUNK_DEPTH) {
        m_blocks[x][y][z] = type;
        generateMesh();
    }
}

void Chunk::generateMesh() {    std::vector<float> verts;
    float ox = static_cast<float>(m_position.x * CHUNK_WIDTH);
    float oz = static_cast<float>(m_position.y * CHUNK_DEPTH);

    const float COLS   = 9.0f;
    const float ROWS   = 10.0f;
    const float STEP_U = 1.0f / COLS;
    const float STEP_V = 1.0f / ROWS;

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
    for (int z = 0; z < CHUNK_DEPTH; ++z) {
        BlockType bt = m_blocks[x][y][z];
        if (bt == BlockType::Air) continue;

        // === KORRIGERING: Använd den nya TextureManager ===
        glm::ivec2 topGrid    = TextureManager::getBlockFaceTexture(bt, 0);
        glm::ivec2 sideGrid   = TextureManager::getBlockFaceTexture(bt, 1);
        glm::ivec2 bottomGrid = TextureManager::getBlockFaceTexture(bt, 2);

        auto pushFace = [&](int startIdx, const glm::ivec2& G){
            float uo = G.x * STEP_U;
            float vo = (ROWS - 1.0f - G.y) * STEP_V;
            for (int i = startIdx; i < startIdx + 6; ++i) {
                verts.push_back(face_vertices[i*5 + 0] + x + ox);
                verts.push_back(face_vertices[i*5 + 1] +     y);
                verts.push_back(face_vertices[i*5 + 2] + z + oz);
                verts.push_back(face_vertices[i*5 + 3] * STEP_U + uo);
                verts.push_back(face_vertices[i*5 + 4] * STEP_V + vo);
            }
        };

        // Den här logiken är korrekt
        if (z==0              || m_blocks[x][y][z-1]==BlockType::Air) pushFace( 0, sideGrid);
        if (z==CHUNK_DEPTH-1  || m_blocks[x][y][z+1]==BlockType::Air) pushFace( 6, sideGrid);
        if (x==0              || m_blocks[x-1][y][z]==BlockType::Air) pushFace(12, sideGrid);
        if (x==CHUNK_WIDTH-1  || m_blocks[x+1][y][z]==BlockType::Air) pushFace(18, sideGrid);
        if (y==0              || m_blocks[x][y-1][z]==BlockType::Air) pushFace(24, bottomGrid);
        if (y==CHUNK_HEIGHT-1 || m_blocks[x][y+1][z]==BlockType::Air) pushFace(30, topGrid);
    }}}

    m_vertex_count = static_cast<int>(verts.size()) / 5;

    // Om vi regenererar en mesh som redan finns, ta bort den gamla först.
    if(m_vao) glDeleteVertexArrays(1, &m_vao);
    if(m_vbo) glDeleteBuffers(1, &m_vbo);

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1,      &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 verts.size() * sizeof(float),
                 verts.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void Chunk::draw() const {
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_vertex_count);
    glBindVertexArray(0);
}