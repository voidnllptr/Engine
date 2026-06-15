// src/resources/Mesh.hpp
#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <memory>
#include "../vulkan/Device.hpp"
#include "../vulkan/Buffer.hpp"
#include "../vulkan/Memory.hpp"  // ← Добавлен MemoryAllocator
#include "../utils/Math.hpp"

namespace Resources {

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
        glm::vec3 color;
        glm::vec3 tangent;
        glm::vec3 bitangent;

        static VkVertexInputBindingDescription getBindingDescription();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        static std::vector<Vertex> createTriangleVertices();
        static std::vector<Vertex> createQuadVertices();
        static std::vector<Vertex> createCubeVertices();
        static std::vector<uint32_t> createCubeIndices();
    };

    struct Primitive {
        uint32_t firstIndex = 0;
        uint32_t indexCount = 0;
        uint32_t vertexOffset = 0;
        int32_t materialIndex = -1;

        glm::vec3 minBounds = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 maxBounds = glm::vec3(std::numeric_limits<float>::lowest());

        void updateBounds(const glm::vec3& vertex);
        void finalizeBounds();
    };

    class Mesh {
    public:
        Mesh() = default;
        ~Mesh() { cleanup(); }

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;

        void create(Vulkan::Device& device,
            Vulkan::MemoryAllocator& allocator,
            const std::vector<Vertex>& vertices,
            const std::vector<uint32_t>& indices,
            const std::vector<Primitive>& primitives = {});


        void createSimple(Vulkan::Device& device,
            Vulkan::MemoryAllocator& allocator,
            const std::vector<Vertex>& vertices,
            const std::vector<uint32_t>& indices);

        void cleanup();

        void bind(VkCommandBuffer commandBuffer) const;
        void draw(VkCommandBuffer commandBuffer,
            VkPipelineLayout pipelineLayout = VK_NULL_HANDLE,
            uint32_t instanceCount = 1) const;
        void drawPrimitive(VkCommandBuffer commandBuffer,
            uint32_t primitiveIndex,
            uint32_t instanceCount = 1) const;

        void addPrimitive(const Primitive& primitive);
        void addPrimitive(uint32_t indexCount, int32_t materialIndex = -1);
        void setPrimitiveMaterial(uint32_t primitiveIndex, int32_t materialIndex);

        const std::vector<Primitive>& getPrimitives() const { return m_primitives; }
        Primitive& getPrimitive(uint32_t index) { return m_primitives[index]; }

        VkBuffer getVertexBuffer() const { return m_vertexBuffer.get(); }
        VkBuffer getIndexBuffer() const { return m_indexBuffer.get(); }
        uint32_t getVertexCount() const { return m_vertexCount; }
        uint32_t getIndexCount() const { return m_indexCount; }
        const std::string& getName() const { return m_name; }
        void setName(const std::string& name) { m_name = name; }
        bool isValid() const { return m_vertexBuffer.isValid() && m_indexBuffer.isValid(); }

        const glm::vec3& getMinBounds() const { return m_minBounds; }
        const glm::vec3& getMaxBounds() const { return m_maxBounds; }
        void calculateBounds();

    private:
        void uploadBuffers(Vulkan::Device& device,
            Vulkan::MemoryAllocator& allocator,
            const void* vertexData, VkDeviceSize vertexSize,
            const void* indexData, VkDeviceSize indexSize);

        std::string m_name;

        Vulkan::Buffer m_vertexBuffer;
        Vulkan::Buffer m_indexBuffer;

        uint32_t m_vertexCount = 0;
        uint32_t m_indexCount = 0;
        VkIndexType m_indexType = VK_INDEX_TYPE_UINT32;

        std::vector<Primitive> m_primitives;

        glm::vec3 m_minBounds = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 m_maxBounds = glm::vec3(std::numeric_limits<float>::lowest());

        Vulkan::Device* m_device = nullptr;
    };

    std::unique_ptr<Mesh> createTriangleMesh(Vulkan::Device& device, Vulkan::MemoryAllocator& allocator);
    std::unique_ptr<Mesh> createQuadMesh(Vulkan::Device& device, Vulkan::MemoryAllocator& allocator,
        float width = 1.0f, float height = 1.0f);
    std::unique_ptr<Mesh> createCubeMesh(Vulkan::Device& device, Vulkan::MemoryAllocator& allocator);
    std::unique_ptr<Mesh> createSphereMesh(Vulkan::Device& device, Vulkan::MemoryAllocator& allocator,
        float radius = 0.5f, int segments = 32);
    std::unique_ptr<Mesh> createPlaneMesh(Vulkan::Device& device, Vulkan::MemoryAllocator& allocator,
        float width = 1.0f, float depth = 1.0f);

}