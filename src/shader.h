#pragma once

#include <stdint.h>
#include <string>
#include <filesystem>
#include <string_view>
#include <unordered_map>

class Shader
{
public:
    Shader(const std::filesystem::path& vert_path, const std::filesystem::path& frag_path);
    ~Shader();

public:
    void Use();
    void setBool(std::string_view name, bool value) const;
    void setInteger(std::string_view name, int32_t value) const;
    void setFloat(std::string_view name, float value) const;

private:
    std::string ReadShaderFile(const std::filesystem::path& file_path) const;
    uint32_t CompileShader(std::string_view shader_code, uint32_t shader_type) const;
    int32_t GetUniformLocation(std::string_view name) const;

private:
    uint32_t program_id_;
    mutable std::unordered_map<std::string_view, int32_t> location_cache_;
};