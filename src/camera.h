#pragma once

#include <glm/glm.hpp>
#include <vector>

inline constexpr float YAW = -90.0f;
inline constexpr float PITCH = 0.0f;
inline constexpr float SPEED = 2.5f;
inline constexpr float SENSITIVITY = 0.1f;
inline constexpr float ZOOM = 45.0f;

enum class CameraMovement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera
{
public:
    Camera(const glm::vec3& position, const glm::vec3& up, float yaw, float pitch);
    Camera(glm::vec3&& position, glm::vec3&& up, float yaw, float pitch);
    ~Camera();

public:
    glm::mat4 GetViewMatrix() const;
    void ProcessKeyboard(CameraMovement direction, float delta_time);
    void ProcessMouseMovement(float x_offset, float y_offset, bool constrain_pitch);
    void ProcessMouseScroll(float y_offset);
    float GetZoom() const;

private:
    void UpdateCameraVectors();

private:
    glm::vec3 position_;
    glm::vec3 front_;
    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 left_;
    glm::vec3 world_up_;
    float yaw_;
    float pitch_;
    float movement_speed_;
    float mouse_sensitivity_;
    float zoom_;
};