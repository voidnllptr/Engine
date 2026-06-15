#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <cmath>
#include <random>
#include <numbers>
#include <limits>
#include <vector>
#include <vulkan/vulkan.h>

namespace Utils {
    namespace Math {

        constexpr float PI = std::numbers::pi_v<float>;
        constexpr float TWO_PI = 2.0f * PI;
        constexpr float HALF_PI = PI / 2.0f;
        constexpr float EPSILON = 1e-6f;
        constexpr float INFINITY_F = std::numeric_limits<float>::infinity();

        using Vec2 = glm::vec2;
        using Vec3 = glm::vec3;
        using Vec4 = glm::vec4;
        using Mat3 = glm::mat3;
        using Mat4 = glm::mat4;
        using Quat = glm::quat;

        inline float radians(float degrees) {
            return degrees * PI / 180.0f;
        }

        inline float degrees(float radians) {
            return radians * 180.0f / PI;
        }

        template<typename T>
        inline T lerp(const T& a, const T& b, float t) {
            return a + (b - a) * t;
        }

        template<typename T>
        inline T clamp(const T& value, const T& min, const T& max) {
            return std::max(min, std::min(value, max));
        }

        inline float smoothstep(float edge0, float edge1, float x) {
            float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
            return t * t * (3.0f - 2.0f * t);
        }

        inline bool nearZero(const Vec3& v) {
            return std::abs(v.x) < EPSILON &&
                std::abs(v.y) < EPSILON &&
                std::abs(v.z) < EPSILON;
        }

        inline float dot(const Vec3& a, const Vec3& b) {
            return glm::dot(a, b);
        }

        inline Vec3 cross(const Vec3& a, const Vec3& b) {
            return glm::cross(a, b);
        }

        inline Vec3 normalize(const Vec3& v) {
            return glm::normalize(v);
        }

        inline float length(const Vec3& v) {
            return glm::length(v);
        }

        inline Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
            return glm::lookAt(eye, target, up);
        }

        inline Mat4 perspective(float fov, float aspect, float nearPlane, float farPlane) {
            return glm::perspective(radians(fov), aspect, nearPlane, farPlane);
        }

        inline Mat4 orthographic(float left, float right, float bottom, float top,
            float nearPlane, float farPlane) {
            return glm::ortho(left, right, bottom, top, nearPlane, farPlane);
        }

        inline Mat4 translate(const Mat4& matrix, const Vec3& translation) {
            return glm::translate(matrix, translation);
        }

        inline Mat4 rotate(const Mat4& matrix, float angle, const Vec3& axis) {
            return glm::rotate(matrix, radians(angle), axis);
        }

        inline Mat4 scale(const Mat4& matrix, const Vec3& scale) {
            return glm::scale(matrix, scale);
        }

        inline Mat4 createModelMatrix(const Vec3& position, const Quat& rotation, const Vec3& scale) {
            return glm::translate(glm::mat4(1.0f), position) *
                glm::toMat4(rotation) *
                glm::scale(glm::mat4(1.0f), scale);
        }

        inline Quat eulerToQuat(float pitch, float yaw, float roll) {
            return glm::quat(glm::vec3(radians(pitch), radians(yaw), radians(roll)));
        }

        inline Vec3 quatToEuler(const Quat& q) {
            glm::vec3 eulerRad = glm::eulerAngles(q);
            return Vec3(degrees(eulerRad.x), degrees(eulerRad.y), degrees(eulerRad.z));
        }

        inline Vec3 getForward(const Quat& rotation) {
            return rotation * Vec3(0.0f, 0.0f, -1.0f);
        }

        inline Vec3 getRight(const Quat& rotation) {
            return rotation * Vec3(1.0f, 0.0f, 0.0f);
        }

        inline Vec3 getUp(const Quat& rotation) {
            return rotation * Vec3(0.0f, 1.0f, 0.0f);
        }

        inline Quat angleAxis(float angle, const Vec3& axis) {
            return glm::angleAxis(radians(angle), normalize(axis));
        }

        inline Quat slerp(const Quat& a, const Quat& b, float t) {
            return glm::slerp(a, b, t);
        }

        inline float randomFloat(float min = 0.0f, float max = 1.0f) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dis(min, max);
            return dis(gen);
        }

        inline Vec3 randomVec3(float min = 0.0f, float max = 1.0f) {
            return Vec3(randomFloat(min, max), randomFloat(min, max), randomFloat(min, max));
        }

        struct Material {
            Vec3 albedo = Vec3(1.0f);
            float metallic = 0.0f;
            float roughness = 0.5f;
            float ao = 1.0f;

            Material() = default;
            Material(const Vec3& albedo, float metallic, float roughness, float ao = 1.0f)
                : albedo(albedo), metallic(metallic), roughness(roughness), ao(ao) {
            }
        };

        struct Light {
            Vec3 position = Vec3(0.0f);
            Vec3 color = Vec3(1.0f);
            float intensity = 1.0f;

            Light() = default;
            Light(const Vec3& position, const Vec3& color, float intensity = 1.0f)
                : position(position), color(color), intensity(intensity) {
            }
        };

        struct CameraUBO {
            alignas(16) Mat4 view;
            alignas(16) Mat4 proj;
            alignas(16) Vec3 position;
            float padding;
        };

        struct ModelUBO {
            alignas(16) Mat4 model;
            alignas(16) Mat4 normalMatrix;
        };

        struct GlobalUBO {
            alignas(16) Mat4 view;
            alignas(16) Mat4 proj;
            alignas(16) Vec3 cameraPos;
            float padding1;
            alignas(16) Light lights[4];
            int lightCount;
            Vec3 ambientColor;
            float padding2;
        };

        struct Vertex {
            Vec3 position;
            Vec3 normal;
            Vec2 texCoord;
            Vec3 color;

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
                std::vector<VkVertexInputBindingDescription> bindings(1);
                bindings[0].binding = 0;
                bindings[0].stride = sizeof(Vertex);
                bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                return bindings;
            }

            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
                std::vector<VkVertexInputAttributeDescription> attributes(4);

                attributes[0].binding = 0;
                attributes[0].location = 0;
                attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributes[0].offset = offsetof(Vertex, position);

                attributes[1].binding = 0;
                attributes[1].location = 1;
                attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributes[1].offset = offsetof(Vertex, normal);

                attributes[2].binding = 0;
                attributes[2].location = 2;
                attributes[2].format = VK_FORMAT_R32G32_SFLOAT;
                attributes[2].offset = offsetof(Vertex, texCoord);

                attributes[3].binding = 0;
                attributes[3].location = 3;
                attributes[3].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributes[3].offset = offsetof(Vertex, color);

                return attributes;
            }
        };

    }
}