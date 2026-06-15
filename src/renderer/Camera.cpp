#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace Renderer {

    Camera::Camera() {
        updateViewMatrix();
        setPerspective(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
    }

    Camera::Camera(const glm::vec3& position, float pitch, float yaw)
        : m_position(position)
        , m_pitch(pitch)
        , m_yaw(yaw) {
        updateViewMatrix();
        setPerspective(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
    }

    void Camera::setPerspective(float fov, float aspectRatio, float nearPlane, float farPlane) {
        m_isOrthographic = false;
        m_fov = fov;
        m_aspectRatio = aspectRatio;
        m_nearPlane = nearPlane;
        m_farPlane = farPlane;
        m_projDirty = true;
    }

    void Camera::setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
        m_isOrthographic = true;
        m_orthoLeft = left;
        m_orthoRight = right;
        m_orthoBottom = bottom;
        m_orthoTop = top;
        m_nearPlane = nearPlane;
        m_farPlane = farPlane;
        m_projDirty = true;
    }

    void Camera::setAspectRatio(float aspectRatio) {
        m_aspectRatio = aspectRatio;
        m_projDirty = true;
    }

    void Camera::setPosition(const glm::vec3& position) {
        m_position = position;
        m_viewDirty = true;
    }

    void Camera::setRotation(float pitch, float yaw) {
        m_pitch = glm::clamp(pitch, -89.0f, 89.0f);
        m_yaw = yaw;
        m_viewDirty = true;
    }

    void Camera::setPitch(float pitch) {
        m_pitch = glm::clamp(pitch, -89.0f, 89.0f);
        m_viewDirty = true;
    }

    void Camera::setYaw(float yaw) {
        m_yaw = yaw;
        m_viewDirty = true;
    }

    void Camera::moveForward(float delta) {
        m_position += getForward() * delta;
        m_viewDirty = true;
    }

    void Camera::moveRight(float delta) {
        m_position += getRight() * delta;
        m_viewDirty = true;
    }

    void Camera::moveUp(float delta) {
        m_position += getUp() * delta;
        m_viewDirty = true;
    }

    void Camera::rotate(float deltaPitch, float deltaYaw) {
        m_pitch = glm::clamp(m_pitch + deltaPitch, -89.0f, 89.0f);
        m_yaw += deltaYaw;
        m_viewDirty = true;
    }

    glm::mat4 Camera::getViewMatrix() const {
        if (m_viewDirty) {
            const_cast<Camera*>(this)->updateViewMatrix();
        }
        return m_cachedViewMatrix;
    }

    glm::mat4 Camera::getProjectionMatrix() const {
        if (m_projDirty) {
            const_cast<Camera*>(this)->updateProjectionMatrix();
        }
        return m_cachedProjectionMatrix;
    }

    glm::vec3 Camera::getForward() const {
        if (m_viewDirty) {
            const_cast<Camera*>(this)->updateViewMatrix();
        }
        return m_front;
    }

    glm::vec3 Camera::getRight() const {
        if (m_viewDirty) {
            const_cast<Camera*>(this)->updateViewMatrix();
        }
        return m_right;
    }

    glm::vec3 Camera::getUp() const {
        if (m_viewDirty) {
            const_cast<Camera*>(this)->updateViewMatrix();
        }
        return m_up;
    }

    void Camera::updateViewMatrix() {
        glm::vec3 front;
        front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        front.y = sin(glm::radians(m_pitch));
        front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_front = glm::normalize(front);

        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        m_right = glm::normalize(glm::cross(m_front, worldUp));

        m_up = glm::normalize(glm::cross(m_right, m_front));

        m_cachedViewMatrix = glm::lookAt(m_position, m_position + m_front, m_up);
        m_viewDirty = false;
    }

    void Camera::updateProjectionMatrix() const {
        if (m_isOrthographic) {
            m_cachedProjectionMatrix = glm::ortho(
                m_orthoLeft, m_orthoRight,
                m_orthoBottom, m_orthoTop,
                m_nearPlane, m_farPlane
            );
        }
        else {
            m_cachedProjectionMatrix = glm::perspective(
                glm::radians(m_fov),
                m_aspectRatio,
                m_nearPlane,
                m_farPlane
            );
            m_cachedProjectionMatrix[1][1] *= -1.0f;
        }
        m_projDirty = false;
    }

}