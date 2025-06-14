cmake_minimum_required(VERSION 3.10)
project(MyVoxelGame)

# Compiler warning flags
if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Third-party subdirectories
add_subdirectory(third_party/glfw)
add_subdirectory(third_party/glm)

# GLAD static library
add_library(glad STATIC third_party/glad/src/glad.c)
target_include_directories(glad PUBLIC third_party/glad/include)

# --- NYTT: Skapa ett bibliotek för Perlin-bruset ---
add_library(perlin STATIC third_party/perlin/perlin.cpp)
target_include_directories(perlin PUBLIC third_party/perlin)

# Source files
file(GLOB SRC_FILES src/*.cpp)

# Main executable
add_executable(${PROJECT_NAME} ${SRC_FILES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    include
    third_party/glad/include
    third_party/glfw/include
    third_party/glm
    third_party/stb
    third_party/perlin # <-- Inkludera Perlin-mappen
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glad
    glm
    perlin # <-- NYTT: Länka mot Perlin-biblioteket
)

# On Linux/macOS, link system libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE dl pthread X11)
endif()

# Copy the assets folder to the build output directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
)