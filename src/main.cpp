#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>

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

    std::vector<char> vertex_shader_source = read_file("assets/shaders/triangle.vert");
    vertex_shader_source.push_back('\0');
    uint32_t vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const char* vss = vertex_shader_source.data();
    glShaderSource(vertex_shader, 1, &vss, nullptr);
    glCompileShader(vertex_shader);
    if (!check_shader_compilation_status(vertex_shader)) {
        return -1;
    }


    std::vector<char> fragment_shader_source = read_file("assets/shaders/triangle.frag");
    fragment_shader_source.push_back('\0');
    uint32_t fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fss = fragment_shader_source.data();
    glShaderSource(fragment_shader, 1, &fss, nullptr);
    glCompileShader(fragment_shader);
    if (!check_shader_compilation_status(fragment_shader)) {
        return -1;
    }

    uint32_t shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    int32_t success = 0;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        return -1;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    float vertices[] = {
        0.5f,  0.5f, 0.0f,  // top right
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    uint32_t ebo = 0;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        process_input(window);

        glUseProgram(shader_program);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shader_program);

    glfwTerminate();
    return 0;
}