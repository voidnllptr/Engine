#pragma once

#include <string>
#include <memory>
#include <functional>
#include <vulkan/vulkan.h>
#include "../utils/Math.hpp"
#include "../resources/Texture.hpp"

namespace Resources {
    class Model;
}

namespace Core {

    class SceneObject {
    public:
        SceneObject() = default;
        explicit SceneObject(const std::string& name);
        SceneObject(const std::string& name, std::shared_ptr<Resources::Model> model);

        SceneObject(const SceneObject&) = delete;
        SceneObject& operator=(const SceneObject&) = delete;

        SceneObject(SceneObject&& other) noexcept;
        SceneObject& operator=(SceneObject&& other) noexcept;

        const std::string& getName() const { return m_name; }
        const std::string& getUniqueId() const { return m_uniqueId; }
        std::shared_ptr<Resources::Model> getModel() const { return m_model; }
        const Utils::Math::Vec3& getPosition() const { return m_position; }
        const Utils::Math::Quat& getRotation() const { return m_rotation; }
        const Utils::Math::Vec3& getScale() const { return m_scale; }
        bool isVisible() const { return m_visible; }

        void setName(const std::string& name) { m_name = name; }
        void setModel(std::shared_ptr<Resources::Model> model) { m_model = model; }
        void setPosition(const Utils::Math::Vec3& position) { m_position = position; }
        void setRotation(const Utils::Math::Quat& rotation) { m_rotation = rotation; }
        void setRotationEuler(float pitch, float yaw, float roll);
        void setScale(const Utils::Math::Vec3& scale) { m_scale = scale; }
        void setVisible(bool visible) { m_visible = visible; }

        Utils::Math::Mat4 getModelMatrix() const;
        Utils::Math::Vec3 getForward() const;
        Utils::Math::Vec3 getRight() const;
        Utils::Math::Vec3 getUp() const;

        Utils::Math::Vec3 getRotationEuler() const;

        void translate(const Utils::Math::Vec3& delta);
        void rotate(float angle, const Utils::Math::Vec3& axis);
        void scale(const Utils::Math::Vec3& delta);

        std::string getDisplayName() const {
            return m_name + "##" + m_uniqueId;
        }

        std::shared_ptr<Resources::Texture> getTexture() const { return m_texture; }
        bool hasTexture() const { return m_texture != nullptr && m_texture->isValid(); }
        void setTexture(std::shared_ptr<Resources::Texture> texture) {
            m_texture = texture;
        }
        void removeTexture() {
            m_texture.reset();
            m_textureDescriptorSet = VK_NULL_HANDLE;
        }

        VkDescriptorSet getTextureDescriptorSet() const { return m_textureDescriptorSet; }
        void setTextureDescriptorSet(VkDescriptorSet descriptorSet) {
            m_textureDescriptorSet = descriptorSet;
        }

    private:
        std::string m_name;
        std::string m_uniqueId;
        std::shared_ptr<Resources::Model> m_model;
        std::shared_ptr<Resources::Texture> m_texture;
        VkDescriptorSet m_textureDescriptorSet = VK_NULL_HANDLE;
        Utils::Math::Vec3 m_position = Utils::Math::Vec3(0.0f);
        Utils::Math::Quat m_rotation = Utils::Math::Quat(1.0f, 0.0f, 0.0f, 0.0f);
        Utils::Math::Vec3 m_scale = Utils::Math::Vec3(1.0f);
        bool m_visible = true;
    };

}