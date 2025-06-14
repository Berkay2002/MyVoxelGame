#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "Shader.h"
#include "Camera.h"
#include "World.h"
#include "Texture.h"
#include "Player.h"
#include "UIRenderer.h"

// Constants
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

// Global state
Camera camera(glm::vec3(0.0f));
Player player(glm::vec3(8.0f, 15.0f, 8.0f));
World world;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool left_mouse_pressed = false;
bool right_mouse_pressed = false;

// Function prototypes
void framebuffer_size_callback(GLFWwindow*, int width, int height);
void mouse_callback(GLFWwindow*, double xpos, double ypos);
void scroll_callback(GLFWwindow*, double, double yoffset);
void processInput(GLFWwindow* window);
void mouse_button_callback(GLFWwindow*, int button, int action, int);
bool initializeWindow(GLFWwindow** window);
void setupCallbacks(GLFWwindow* window);
unsigned int createLineVAO();
void handleBlockInteraction(bool hit, const glm::ivec3& block_pos, const glm::ivec3& prev_pos);
void renderWorld(Shader& shader, Texture& texture, const glm::mat4& projection, const glm::mat4& view);
void renderSelectionBox(Shader& shader, unsigned int lineVAO, bool hit, const glm::ivec3& block_pos, 
                        const glm::mat4& projection, const glm::mat4& view);

int main() {
    GLFWwindow* window = nullptr;
    if (!initializeWindow(&window)) {
        return -1;
    }
    
    setupCallbacks(window);
    
    // Enable GL features
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); // Bra att ha på för prestanda
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(2.5f);

    // Create resources
    Shader voxelShader("assets/shaders/voxel.vert", "assets/shaders/voxel.frag");
    Shader lineShader("assets/shaders/line.vert", "assets/shaders/line.frag");
    Texture texture_atlas("assets/textures/atlas.png");
    unsigned int lineVAO = createLineVAO();

    // Set up shader uniforms
    UIRenderer uiRenderer(SCR_WIDTH, SCR_HEIGHT);

    voxelShader.use();
    voxelShader.setInt("uTexture", 0);

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        // Timing
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input and updates
        processInput(window);
        player.update(deltaTime, world);
        world.update(player.position);
        
        // Update camera to follow player
        camera.Position = player.position + glm::vec3(0.0f, 1.6f, 0.0f);

        // Raycast for block interaction
        glm::ivec3 block_pos, prev_pos;
        bool hit = world.raycast(camera.Position, camera.Front, 5.0f, block_pos, prev_pos);
        handleBlockInteraction(hit, block_pos, prev_pos);
        
        // Reset mouse button states
        left_mouse_pressed = false;
        right_mouse_pressed = false;

        // Clear the screen
        glClearColor(0.2f, 0.3f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Prepare matrices
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        
        // ---- RENDERINGSSTEG ----

        // 1. Rita 3D-världen
        renderWorld(voxelShader, texture_atlas, projection, view);
        
        // 2. Rita markeringsramen
        renderSelectionBox(lineShader, lineVAO, hit, block_pos, projection, view);

        // 3. --- NYTT: Rita UI ---
        // Avaktivera depth test så att UI:t alltid ritas överst
        glDisable(GL_DEPTH_TEST);
        uiRenderer.drawHotbar(player, texture_atlas);
        // uiRenderer.drawCrosshair(); // Kommentar in om du vill ha ett hårkors
        glEnable(GL_DEPTH_TEST); // Aktivera igen för nästa frame

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &lineVAO);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

bool initializeWindow(GLFWwindow** window) {
    if (!glfwInit()) {
        return false;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Voxel Engine", NULL, NULL);
    if (!*window) {
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(*window);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return false;
    }
    
    return true;
}

void setupCallbacks(GLFWwindow* window) {
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

unsigned int createLineVAO() {
    unsigned int lineVAO, lineVBO;
    float line_vertices[] = {
        // Bottom face (z=-0.5)
        -0.5f,-0.5f,-0.5f, 0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f, 0.5f,0.5f,-0.5f,  0.5f,0.5f,-0.5f, -0.5f,0.5f,-0.5f,  -0.5f,0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
        // Top face (z=0.5)
        -0.5f,-0.5f,0.5f, 0.5f,-0.5f,0.5f,  0.5f,-0.5f,0.5f, 0.5f,0.5f,0.5f,  0.5f,0.5f,0.5f, -0.5f,0.5f,0.5f,  -0.5f,0.5f,0.5f, -0.5f,-0.5f,0.5f,
        // Vertical edges connecting bottom to top
        -0.5f,-0.5f,-0.5f, -0.5f,-0.5f,0.5f,  0.5f,-0.5f,-0.5f, 0.5f,-0.5f,0.5f,  0.5f,0.5f,-0.5f, 0.5f,0.5f,0.5f,  -0.5f,0.5f,-0.5f, -0.5f,0.5f,0.5f
    };
    
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    
    return lineVAO;
}

void handleBlockInteraction(bool hit, const glm::ivec3& block_pos, const glm::ivec3& prev_pos) {
    if (hit) {
        if (left_mouse_pressed) {
            world.setBlock(block_pos.x, block_pos.y, block_pos.z, BlockType::Air);
        }
        if (right_mouse_pressed) {
            // --- ÄNDRING: Använd spelarens valda block ---
            world.setBlock(prev_pos.x, prev_pos.y, prev_pos.z, player.getSelectedBlock());        }
    }
}

void renderWorld(Shader& shader, Texture& texture, const glm::mat4& projection, const glm::mat4& view) {
    shader.use();
    shader.setMat4("uProjection", projection);
    shader.setMat4("uView", view);
    shader.setMat4("uModel", glm::mat4(1.0f));
    texture.bind();
    world.draw();
}

void renderSelectionBox(Shader& shader, unsigned int lineVAO, bool hit, const glm::ivec3& block_pos, 
                       const glm::mat4& projection, const glm::mat4& view) {
    if (hit) {
        shader.use();
        shader.setMat4("uProjection", projection);
        shader.setMat4("uView", view);

        // Transform selection box
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(block_pos));
        model = glm::scale(model, glm::vec3(1.01f));
        
        shader.setMat4("uModel", model);
        
        glBindVertexArray(lineVAO);
        glDrawArrays(GL_LINES, 0, 24);
        glBindVertexArray(0);
    }
}

void mouse_button_callback(GLFWwindow*, int button, int action, int) {
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            left_mouse_pressed = true;
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            right_mouse_pressed = true;
        }
    }
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glm::vec3 direction(0.0f);
    glm::vec3 front = glm::normalize(glm::vec3(camera.Front.x, 0.0f, camera.Front.z));
    glm::vec3 right = glm::normalize(glm::vec3(camera.Right.x, 0.0f, camera.Right.z));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) direction += front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) direction -= front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) direction -= right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) direction += right;
    
    player.move(direction);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) player.jump();
}

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow*, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    
    camera.ProcessMouseMovement(xoffset, yoffset);
}

// --- ÄNDRING: Uppdatera scroll-callback för att byta hotbar-slot ---
void scroll_callback(GLFWwindow*, double, double yoffset) {
    player.changeHotbarSlot(static_cast<int>(yoffset));
}