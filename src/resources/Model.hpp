#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <optional>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../vulkan/Device.hpp"
#include "../vulkan/Buffer.hpp"
#include "../vulkan/Memory.hpp"
#include "../renderer/Material.hpp"
#include "Mesh.hpp"

namespace Resources {

    class Model {
    public:
        Model() = default;
        ~Model() { cleanup(); }

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        Model(Model&& other) noexcept;
        Model& operator=(Model&& other) noexcept;

        bool loadOBJ(
            Vulkan::Device& device,
            Vulkan::MemoryAllocator& allocator,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            const std::string& filepath,
            const std::string& mtlBasePath = ""
        );

        bool loadGLTF(
            Vulkan::Device& device,
            Vulkan::MemoryAllocator& allocator,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            const std::string& filepath
        );

        void cleanup();

        void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const;
        void drawWithMaterials(
            VkCommandBuffer commandBuffer,
            VkPipelineLayout pipelineLayout,
            const std::vector<std::shared_ptr<Renderer::Material>>& materials
        ) const;

        const std::vector<std::unique_ptr<Mesh>>& getMeshes() const { return m_meshes; }
        const std::vector<std::shared_ptr<Renderer::Material>>& getMaterials() const { return m_materials; }
        const std::string& getName() const { return m_name; }
        bool isValid() const { return !m_meshes.empty(); }

        void setPosition(const glm::vec3& position) { m_position = position; }
        void setRotation(const glm::quat& rotation) { m_rotation = rotation; }
        void setRotation(float angle, const glm::vec3& axis);
        void setScale(const glm::vec3& scale) { m_scale = scale; }
        glm::mat4 getModelMatrix() const;

        const glm::vec3& getPosition() const { return m_position; }
        const glm::quat& getRotation() const { return m_rotation; }
        const glm::vec3& getScale() const { return m_scale; }

        static std::shared_ptr<Model> createCube(
            Vulkan::Device& device,
            Vulkan::MemoryAllocator& allocator,
            VkCommandPool commandPool,
            VkQueue graphicsQueue
        );

        static std::shared_ptr<Model> createSphere(
            Vulkan::Device& device,
            Vulkan::MemoryAllocator& allocator,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            float radius = 0.5f,
            int segments = 32
        );

        static std::shared_ptr<Model> createPlane(
            Vulkan::Device& device,
            Vulkan::MemoryAllocator& allocator,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            float width = 1.0f,
            float depth = 1.0f
        );

    private:
        bool loadFromOBJ(
            Vulkan::Device& device,
            Vulkan::MemoryAllocator& allocator,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            const std::string& filepath,
            const std::string& mtlBasePath
        );

        std::string m_name;
        std::vector<std::unique_ptr<Mesh>> m_meshes;
        std::vector<std::shared_ptr<Renderer::Material>> m_materials;

        glm::vec3 m_position = glm::vec3(0.0f);
        glm::quat m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 m_scale = glm::vec3(1.0f);

        Vulkan::Device* m_device = nullptr;
    };

    class ModelManager {
    public:
        ModelManager() = default;
        ~ModelManager() = default;

        std::shared_ptr<Model> getModel(
            Vulkan::Device& device,
            Vulkan::MemoryAllocator& allocator,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            const std::string& filepath
        );

        void clear();
        void removeModel(const std::string& filepath);
        size_t getCachedCount() const { return m_modelCache.size(); }

    private:
        std::unordered_map<std::string, std::shared_ptr<Model>> m_modelCache;
    };

}