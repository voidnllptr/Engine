#include "SceneObject.hpp"
#include "../resources/Model.hpp"
#include <iostream>
#include <random>
#include <chrono>

namespace Core {

    static std::string generateUniqueId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 999999);

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

        return std::to_string(timestamp) + "_" + std::to_string(dis(gen));
    }

    SceneObject::SceneObject(const std::string& name)
        : m_name(name)
        , m_uniqueId(generateUniqueId()) {
    }

    SceneObject::SceneObject(const std::string& name, std::shared_ptr<Resources::Model> model)
        : m_name(name)
        , m_uniqueId(generateUniqueId())
        , m_model(model) {
    }

    SceneObject::SceneObject(SceneObject&& other) noexcept
        : m_name(std::move(other.m_name))
        , m_uniqueId(std::move(other.m_uniqueId))
        , m_model(std::move(other.m_model))
        , m_texture(std::move(other.m_texture))
        , m_position(other.m_position)
        , m_rotation(other.m_rotation)
        , m_scale(other.m_scale)
        , m_visible(other.m_visible) {
    }

    SceneObject& SceneObject::operator=(SceneObject&& other) noexcept {
        if (this != &other) {
            m_name = std::move(other.m_name);
            m_uniqueId = std::move(other.m_uniqueId);
            m_model = std::move(other.m_model);
            m_texture = std::move(other.m_texture);
            m_position = other.m_position;
            m_rotation = other.m_rotation;
            m_scale = other.m_scale;
            m_visible = other.m_visible;
        }
        return *this;
    }

    void SceneObject::setRotationEuler(float pitch, float yaw, float roll) {
        m_rotation = Utils::Math::eulerToQuat(pitch, yaw, roll);
    }

    Utils::Math::Mat4 SceneObject::getModelMatrix() const {
        return Utils::Math::createModelMatrix(m_position, m_rotation, m_scale);
    }

    Utils::Math::Vec3 SceneObject::getForward() const {
        return Utils::Math::getForward(m_rotation);
    }

    Utils::Math::Vec3 SceneObject::getRight() const {
        return Utils::Math::getRight(m_rotation);
    }

    Utils::Math::Vec3 SceneObject::getUp() const {
        return Utils::Math::getUp(m_rotation);
    }

    Utils::Math::Vec3 SceneObject::getRotationEuler() const {
        return Utils::Math::quatToEuler(m_rotation);
    }

    void SceneObject::translate(const Utils::Math::Vec3& delta) {
        m_position += delta;
    }

    void SceneObject::rotate(float angle, const Utils::Math::Vec3& axis) {
        Utils::Math::Quat delta = Utils::Math::angleAxis(angle, axis);
        m_rotation = delta * m_rotation;
        m_rotation = glm::normalize(m_rotation);
    }

    void SceneObject::scale(const Utils::Math::Vec3& delta) {
        m_scale *= delta;
    }

}