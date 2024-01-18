#include "glm/ext/matrix_transform.hpp"
#include "shader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

inline static constexpr int32_t WIDTH = 1024;
inline static constexpr int32_t HEIGHT = 768;

static void framebuffer_size_callback(GLFWwindow* window, int32_t width, int32_t height)
{
    (void)window;
    glViewport(0, 0, width, height);
}

static void process_input(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    } else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

static std::vector<char> read_file(const std::filesystem::path& file_path)
{
    std::ifstream file{file_path, std::ios::binary | std::ios::ate};
    if (!file) {
        throw std::runtime_error(file_path.u8string() + ": " + std::strerror(errno));
    }
    auto end = file.tellg();
    file.seekg(0, std::ios::beg);
    auto size = static_cast<size_t>(end - file.tellg());

    if (size == 0) {
        return {};
    }
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), buffer.size())) {
        throw std::runtime_error(file_path.u8string() + ": " + std::strerror(errno));
    }
    return buffer;
}

static bool check_shader_compilation_status(uint32_t shader_id)
{
    int32_t result = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
    if (result == 0) {
        int32_t length = 0;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> message(length);
        glGetShaderInfoLog(shader_id, length, &length, message.data());
        std::cout << "Failed to compile " << " shader" << std::endl;
        std::cout << message.data() << std::endl;
        glDeleteShader(shader_id);
        return false;
    }
    return true;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Learn OpenGL", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    float vertices[] = {
        // positions                        // colors                   // texture coords
        0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left     // top
    };
    uint32_t indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    }; 

    uint32_t vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    uint32_t vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    

    uint32_t ebo = 0;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);


    uint32_t texture1_id = 0;
    glGenTextures(1, &texture1_id);
    glBindTexture(GL_TEXTURE_2D, texture1_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int32_t img_width = 0;
    int32_t img_height = 0;
    int32_t nr_channels = 0;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* img_data = stbi_load("assets/textures/container.jpg", &img_width, &img_height, &nr_channels, 0);
    if (!img_data) {
        std::cout << "Failed to load image" << std::endl;
        return -1;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(img_data);

    uint32_t texture2_id = 0;
    glGenTextures(1, &texture2_id);
    glBindTexture(GL_TEXTURE_2D, texture2_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    img_data = stbi_load("assets/textures/awesomeface.png", &img_width, &img_height, &nr_channels, 0);
    if (!img_data) {
        std::cout << "Failed to load image" << std::endl;
        return -1;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(img_data);

    glEnable(GL_BLEND);// you enable blending function
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glm::mat4 trans{1.0f};
    // trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f});
    // trans = glm::scale(trans, glm::vec3{0.5f, 0.5f, 0.5f});

    Shader shader("assets/shaders/triangle.vert", "assets/shaders/triangle.frag");
    shader.Use();
    shader.setInteger("texture1", 0);
    shader.setInteger("texture2", 1);
    // shader.setMatrix4("transform", trans);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        process_input(window);

        glm::mat4 trans{1.0f};
        trans = glm::translate(trans, glm::vec3{0.5f, -0.5f, 0.0f});
        trans = glm::rotate(trans, static_cast<float>(glfwGetTime()), glm::vec3{0.0f, 0.0f, 1.0f});
        trans = glm::scale(trans, glm::vec3{0.5f, 0.5f, 0.5f});
        shader.setMatrix4("transform", trans);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1_id);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2_id);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    glfwTerminate();
    return 0;
}