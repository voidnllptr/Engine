#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Renderer {

    class Camera {
    public:
        Camera();
        explicit Camera(const glm::vec3& position, float pitch = 0.0f, float yaw = -90.0f);

        void setPerspective(float fov, float aspectRatio, float nearPlane, float farPlane);
        void setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
        void setAspectRatio(float aspectRatio);

        void setPosition(const glm::vec3& position);
        void setRotation(float pitch, float yaw);
        void setPitch(float pitch);
        void setYaw(float yaw);

        void moveForward(float delta);
        void moveRight(float delta);
        void moveUp(float delta);

        void rotate(float deltaPitch, float deltaYaw);

        glm::mat4 getViewMatrix() const;
        glm::mat4 getProjectionMatrix() const;

        const glm::vec3& getPosition() const { return m_position; }
        float getPitch() const { return m_pitch; }
        float getYaw() const { return m_yaw; }

        glm::vec3 getForward() const;
        glm::vec3 getRight() const;
        glm::vec3 getUp() const;

        float getFov() const { return m_fov; }
        float getAspectRatio() const { return m_aspectRatio; }
        float getNearPlane() const { return m_nearPlane; }
        float getFarPlane() const { return m_farPlane; }

        bool isOrthographic() const { return m_isOrthographic; }

    private:
        void updateViewMatrix();
        void updateProjectionMatrix() const;

        glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 3.0f);
        glm::vec3 m_front = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 m_right = glm::vec3(1.0f, 0.0f, 0.0f);

        float m_pitch = 0.0f;
        float m_yaw = -90.0f;
        float m_roll = 0.0f;

        float m_fov = 45.0f;
        float m_aspectRatio = 16.0f / 9.0f;
        float m_nearPlane = 0.1f;
        float m_farPlane = 100.0f;

        float m_orthoLeft = -1.0f;
        float m_orthoRight = 1.0f;
        float m_orthoBottom = -1.0f;
        float m_orthoTop = 1.0f;

        bool m_isOrthographic = false;

        mutable bool m_viewDirty = true;
        mutable bool m_projDirty = true;
        mutable glm::mat4 m_cachedViewMatrix = glm::mat4(1.0f);
        mutable glm::mat4 m_cachedProjectionMatrix = glm::mat4(1.0f);
    };

}