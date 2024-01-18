#include "shader.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>
#include <stdexcept>

#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const std::filesystem::path& vert_path, const std::filesystem::path& frag_path)
{
    std::string vert_code = ReadShaderFile(vert_path);
    std::string frag_code = ReadShaderFile(frag_path);
    uint32_t vertex = CompileShader(vert_code, GL_VERTEX_SHADER);
    uint32_t fragment = CompileShader(frag_code, GL_FRAGMENT_SHADER);
    program_id_ = glCreateProgram();
    glAttachShader(program_id_, vertex);
    glAttachShader(program_id_, fragment);
    glLinkProgram(program_id_);

    int32_t success = 0;
    char info_log[512];
    glGetProgramiv(program_id_, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program_id_, 512, nullptr, info_log);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
        throw std::runtime_error("error");
    }
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader()
{
    glDeleteProgram(program_id_);
}


void Shader::Use()
{
    glUseProgram(program_id_);
}

void Shader::setBool(std::string_view name, bool value) const
{
    glUniform1i(GetUniformLocation(name), static_cast<int32_t>(value));
}

void Shader::setInteger(std::string_view name, int32_t value) const
{
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::setFloat(std::string_view name, float value) const
{
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::setMatrix4(std::string_view name, const glm::mat4& value) const
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

std::string Shader::ReadShaderFile(const std::filesystem::path& file_path) const
{
    std::ifstream shader_file;
    std::stringstream shader_stream;
    // ensure ifstream objects can throw exceptions
    shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit); 
    try {
        shader_file.open(file_path);
        shader_stream << shader_file.rdbuf();
        shader_file.close();
    } catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FAILED_TO_READ_FILE: " << e.what() << std::endl;
        throw std::runtime_error(file_path.u8string());
    }
    return shader_stream.str();
}

uint32_t Shader::CompileShader(std::string_view shader_code, uint32_t shader_type) const
{
    int32_t success = 0;
    char info_log[512];
    const char* shader_code_ptr = shader_code.data();

    uint32_t shader_id = glCreateShader(shader_type);
    glShaderSource(shader_id, 1, &shader_code_ptr, nullptr);
    glCompileShader(shader_id);
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader_id, 512, nullptr, info_log);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << info_log << std::endl;
        throw std::runtime_error("error");
    }
    return shader_id;
}

int32_t Shader::GetUniformLocation(std::string_view name) const
{
    auto it = location_cache_.find(name);
    if (it != location_cache_.end()) {
        return it->second;
    }
    int32_t location = glGetUniformLocation(program_id_, name.data());
    location_cache_[name] = location;
    return location;
}