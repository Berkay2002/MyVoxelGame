#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class Shader {
public:
    // Constructor reads and builds the shader program from given file paths
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    // Activate/use this shader program
    void use() const;

    // Uniform helpers
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    unsigned int ID;  // OpenGL program ID
    mutable std::unordered_map<std::string, int> uniformLocationCache;

    // Load a text file into a string
    std::string loadFile(const std::string& path) const;
    // Compile one shader stage
    unsigned int compile(unsigned int type, const std::string& source) const;
    // Get (and cache) uniform location
    int getUniformLocation(const std::string& name) const;
};

#endif // SHADER_H
