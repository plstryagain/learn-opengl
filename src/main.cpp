#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
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

glm::vec3 camera_pos{0.0f, 0.0f, 3.0f};
glm::vec3 camera_front{0.0f, 0.0f, -1.0f};
glm::vec3 camera_up{0.0f, 1.0f, 0.0f};

float delta_time = 0.0f;
float last_frame = 0.0f;

float last_x = 400;
float last_y = 300;

float yaw = 0;
float pitch = 0;

bool first_mouse = true;

float fov = 45.0f;

static void framebuffer_size_callback(GLFWwindow* window, int32_t width, int32_t height)
{
    (void)window;
    glViewport(0, 0, width, height);
}

static void process_input(GLFWwindow* window)
{
    const float camera_speed = 2.5f * delta_time;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    } else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera_pos += camera_speed * camera_front;
    } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera_pos -= camera_speed * camera_front;
    } else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera_pos -= glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
    } 
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera_pos += glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
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

static void mouse_callback(GLFWwindow* window, double x_pos, double y_pos)
{
    if (first_mouse) {
        last_x = x_pos;
        last_y = y_pos;
        first_mouse = false;
    }
    float x_offset = x_pos - last_x;
    float y_offset = last_y - y_pos;
    last_x = x_pos;
    last_y = y_pos;

    const float sensitivity = 0.1f;
    x_offset *= sensitivity;
    y_offset *= sensitivity;

    yaw += x_offset;
    pitch += y_offset;

    if (pitch > 89.0f) {
        pitch = 89.0f;
    }
    if (pitch < -89.0f) {
        pitch = -89.0f;
    }

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camera_front = glm::normalize(direction);
}

void scroll_calback(GLFWwindow* window, double x_offset, double y_offset)
{
    fov -= static_cast<float>(y_offset);
    if (fov < 1.0f) {
        fov = 1.0f;
    }
    if (fov > 45.0f) {
        fov = 45.0f;
    }

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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_calback);

    // float vertices[] = {
    //     // positions                        // colors                   // texture coords
    //     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
    //     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    //     -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    //     -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left     // top
    // };


float vertices[] = {
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

     0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
     0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
     0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
     0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
     0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
     0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
     0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
     0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f
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
    

    // uint32_t ebo = 0;
    // glGenBuffers(1, &ebo);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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
    glEnable(GL_DEPTH_TEST);

    // glm::mat4 trans{1.0f};
    // trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f});
    // trans = glm::scale(trans, glm::vec3{0.5f, 0.5f, 0.5f});

    Shader shader("assets/shaders/triangle.vert", "assets/shaders/triangle.frag");
    shader.Use();
    shader.setInteger("texture1", 0);
    shader.setInteger("texture2", 1);
    // shader.setMatrix4("transform", trans);
    std::vector<glm::vec3> cube_positions = {
        glm::vec3{ 0.0f,  0.0f,  0.0f}, 
        glm::vec3{ 2.0f,  5.0f, -15.0f}, 
        glm::vec3{-1.5f, -2.2f, -2.5f},  
        glm::vec3{-3.8f, -2.0f, -12.3f},  
        glm::vec3{ 2.4f, -0.4f, -3.5f},  
        glm::vec3{-1.7f,  3.0f, -7.5f},  
        glm::vec3{ 1.3f, -2.0f, -2.5f},  
        glm::vec3{ 1.5f,  2.0f, -2.5f}, 
        glm::vec3{ 1.5f,  0.2f, -1.5f}, 
        glm::vec3{-1.3f,  1.0f, -1.5f}  
    };
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float current_frame = glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        process_input(window);

        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1_id);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2_id);

        shader.Use();
        // glm::vec3 camera_pos{0.0f, 0.0f, 3.0f};
        // glm::vec3 camera_target{0.0f, 0.0f, 0.0f};
        // glm::vec3 camera_direction = glm::normalize(camera_pos - camera_target);
        // glm::vec3 up{0.0f, 1.0f, 0.0f};
        // glm::vec3 camera_right = glm::normalize(glm::cross(up, camera_direction));
        // glm::vec3 camera_up = glm::cross(camera_direction, camera_right);

        // glm::mat4 view = glm::mat4{1.0f};
        float radius = 20.0f;
        float cam_x = sin(glfwGetTime()) * radius;
        float cam_z = cos(glfwGetTime()) * radius;
        glm::mat4 view = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(fov), static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f, 100.0f);
        
        shader.setMatrix4("view", view);
        shader.setMatrix4("projection", projection);
        
        glBindVertexArray(vao);
        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        for (int i = 0; i < cube_positions.size(); ++i) {
            glm::mat4 model = glm::mat4{1.0f};
            model = glm::translate(model, cube_positions[i]);
            float angle = (i == 0 ? 20.0f : 20.0f * i);
            model = glm::rotate(model, static_cast<float>(glfwGetTime()) * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            shader.setMatrix4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    glfwTerminate();
    return 0;
}