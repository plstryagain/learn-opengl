#include "camera.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include <cmath>

Camera::Camera(const glm::vec3& position, const glm::vec3& up, float yaw, float pitch)
    : position_(position), world_up_(up), yaw_(yaw), pitch_(pitch), front_(glm::vec3{0.0f, 0.0f, -1.0f}), 
        movement_speed_(SPEED), mouse_sensitivity_(SENSITIVITY), zoom_(ZOOM)
{

}

Camera::Camera(glm::vec3&& position, glm::vec3&& up, float yaw, float pitch)
    : position_(position), world_up_(up), yaw_(yaw), pitch_(pitch), front_(glm::vec3{0.0f, 0.0f, -1.0f}), 
        movement_speed_(SPEED), mouse_sensitivity_(SENSITIVITY), zoom_(ZOOM)
{

}


Camera::~Camera()
{

}


glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(position_, position_ + front_, up_);
}

void Camera::ProcessKeyboard(CameraMovement direction, float delta_time)
{
    float velocity = movement_speed_ * delta_time;
    switch (direction) {
    case CameraMovement::FORWARD:
        position_ += front_ * velocity;
        break;
    case CameraMovement::BACKWARD:
        position_ -= front_ * velocity;
        break;
    case CameraMovement::LEFT:
        position_ -= right_ * velocity;
        break;
    case CameraMovement::RIGHT:
        position_ += right_ * velocity;
        break;
    }
}

void Camera::ProcessMouseMovement(float x_offset, float y_offset, bool constrain_pitch)
{
    x_offset *= mouse_sensitivity_;
    y_offset *= mouse_sensitivity_;

    yaw_  += x_offset;
    pitch_ += y_offset;

    if (constrain_pitch) {
        if (pitch_ > 89.0f) {
            pitch_ = 89.0f;
        }
        if (pitch_ < -89.0f) {
            pitch_ = -89.0f;
        }
    }
    UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(float y_offset)
{
    zoom_ -= y_offset;
    if (zoom_ < 1.0f) {
        zoom_ = 1.0f;
    }
    if (zoom_ > 45.0f) {
        zoom_ = 45.0f;
    }
}

float Camera::GetZoom() const
{
    return zoom_;
}

void Camera::UpdateCameraVectors()
{
    glm::vec3 front{};
    front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front.y = sin(glm::radians(pitch_));
    front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front_ = glm::normalize(front);
    right_ = glm::normalize(glm::cross(front, world_up_));
    up_ = glm::normalize(glm::cross(right_, front));
}