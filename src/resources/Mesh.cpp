#include "Mesh.hpp"
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <iostream>

namespace Resources {

    VkVertexInputBindingDescription Vertex::getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(6);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, normal);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, color);

        attributeDescriptions[4].binding = 0;
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[4].offset = offsetof(Vertex, tangent);

        attributeDescriptions[5].binding = 0;
        attributeDescriptions[5].location = 5;
        attributeDescriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[5].offset = offsetof(Vertex, bitangent);

        return attributeDescriptions;
    }

    std::vector<Vertex> Vertex::createTriangleVertices() {
        return {
            {{ 0.0f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}}
        };
    }

    std::vector<Vertex> Vertex::createQuadVertices() {
        return {
            {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}}
        };
    }

    std::vector<Vertex> Vertex::createCubeVertices() {
        return {
            {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},

            {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},

            {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},

            {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},

            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},

            {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}}
        };
    }

    std::vector<uint32_t> Vertex::createCubeIndices() {
        return {
            0, 1, 2,  2, 3, 0,
            4, 5, 6,  6, 7, 4,
            8, 9,10, 10,11, 8,
            12,13,14, 14,15,12,
            16,17,18, 18,19,16,
            20,21,22, 22,23,20
        };
    }

    void Primitive::updateBounds(const glm::vec3& vertex) {
        minBounds = glm::min(minBounds, vertex);
        maxBounds = glm::max(maxBounds, vertex);
    }

    void Primitive::finalizeBounds() {}

    Mesh::Mesh(Mesh&& other) noexcept
        : m_name(std::move(other.m_name))
        , m_vertexBuffer(std::move(other.m_vertexBuffer))
        , m_indexBuffer(std::move(other.m_indexBuffer))
        , m_vertexCount(other.m_vertexCount)
        , m_indexCount(other.m_indexCount)
        , m_indexType(other.m_indexType)
        , m_primitives(std::move(other.m_primitives))
        , m_minBounds(other.m_minBounds)
        , m_maxBounds(other.m_maxBounds)
        , m_device(other.m_device) {
        other.m_vertexCount = 0;
        other.m_indexCount = 0;
        other.m_device = nullptr;
    }

    Mesh& Mesh::operator=(Mesh&& other) noexcept {
        if (this != &other) {
            cleanup();
            m_name = std::move(other.m_name);
            m_vertexBuffer = std::move(other.m_vertexBuffer);
            m_indexBuffer = std::move(other.m_indexBuffer);
            m_vertexCount = other.m_vertexCount;
            m_indexCount = other.m_indexCount;
            m_indexType = other.m_indexType;
            m_primitives = std::move(other.m_primitives);
            m_minBounds = other.m_minBounds;
            m_maxBounds = other.m_maxBounds;
            m_device = other.m_device;
            other.m_vertexCount = 0;
            other.m_indexCount = 0;
            other.m_device = nullptr;
        }
        return *this;
    }

    void Mesh::create(Vulkan::Device& device,
        Vulkan::MemoryAllocator& allocator,
        const std::vector<Vertex>& vertices,
        const std::vector<uint32_t>& indices,
        const std::vector<Primitive>& primitives) {
        cleanup();
        m_device = &device;
        m_vertexCount = static_cast<uint32_t>(vertices.size());
        m_indexCount = static_cast<uint32_t>(indices.size());
        m_indexType = VK_INDEX_TYPE_UINT32;

        if (!primitives.empty()) {
            m_primitives = primitives;
        }
        else if (!indices.empty()) {
            Primitive prim;
            prim.firstIndex = 0;
            prim.indexCount = m_indexCount;
            prim.vertexOffset = 0;
            prim.materialIndex = -1;
            m_primitives.push_back(prim);
        }

        uploadBuffers(device, allocator,
            vertices.data(), sizeof(Vertex) * vertices.size(),
            indices.data(), sizeof(uint32_t) * indices.size());

        calculateBounds();
    }

    void Mesh::createSimple(Vulkan::Device& device,
        Vulkan::MemoryAllocator& allocator,
        const std::vector<Vertex>& vertices,
        const std::vector<uint32_t>& indices) {
        create(device, allocator, vertices, indices, {});
    }

    void Mesh::uploadBuffers(Vulkan::Device& device,
        Vulkan::MemoryAllocator& allocator,
        const void* vertexData, VkDeviceSize vertexSize,
        const void* indexData, VkDeviceSize indexSize) {
        m_vertexBuffer.createVertex(allocator, vertexData, vertexSize);

        if (indexData && indexSize > 0) {
            if (m_indexType == VK_INDEX_TYPE_UINT32) {
                m_indexBuffer.createIndex(allocator, indexData, indexSize);
            }
            else {
                m_indexBuffer.create(allocator, indexSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexData);
            }
        }
    }

    void Mesh::cleanup() {
        m_vertexBuffer.cleanup();
        m_indexBuffer.cleanup();
        m_primitives.clear();
        m_vertexCount = 0;
        m_indexCount = 0;
        m_device = nullptr;
    }

    void Mesh::bind(VkCommandBuffer commandBuffer) const {
        if (m_vertexBuffer.isValid()) {
            m_vertexBuffer.bindVertex(commandBuffer);
        }
        if (m_indexBuffer.isValid()) {
            m_indexBuffer.bindIndex(commandBuffer);
        }
    }

    void Mesh::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t instanceCount) const {
        (void)pipelineLayout;

        if (!m_indexBuffer.isValid()) {
            vkCmdDraw(commandBuffer, m_vertexCount, instanceCount, 0, 0);
            return;
        }

        for (const auto& primitive : m_primitives) {
            vkCmdDrawIndexed(commandBuffer, primitive.indexCount, instanceCount,
                primitive.firstIndex, primitive.vertexOffset, 0);
        }
    }

    void Mesh::drawPrimitive(VkCommandBuffer commandBuffer, uint32_t primitiveIndex, uint32_t instanceCount) const {
        if (primitiveIndex >= m_primitives.size()) return;

        const auto& primitive = m_primitives[primitiveIndex];

        if (m_indexBuffer.isValid()) {
            vkCmdDrawIndexed(commandBuffer, primitive.indexCount, instanceCount,
                primitive.firstIndex, primitive.vertexOffset, 0);
        }
        else {
            vkCmdDraw(commandBuffer, primitive.indexCount, instanceCount, primitive.vertexOffset, 0);
        }
    }

    void Mesh::addPrimitive(const Primitive& primitive) {
        m_primitives.push_back(primitive);
    }

    void Mesh::addPrimitive(uint32_t indexCount, int32_t materialIndex) {
        Primitive prim;
        prim.firstIndex = m_indexCount;
        prim.indexCount = indexCount;
        prim.vertexOffset = 0;
        prim.materialIndex = materialIndex;
        m_primitives.push_back(prim);
        m_indexCount += indexCount;
    }

    void Mesh::setPrimitiveMaterial(uint32_t primitiveIndex, int32_t materialIndex) {
        if (primitiveIndex < m_primitives.size()) {
            m_primitives[primitiveIndex].materialIndex = materialIndex;
        }
    }

    void Mesh::calculateBounds() {}

    std::unique_ptr<Mesh> createTriangleMesh(Vulkan::Device& device, Vulkan::MemoryAllocator& allocator) {
        auto vertices = Vertex::createTriangleVertices();
        std::vector<uint32_t> indices = { 0, 1, 2 };

        auto mesh = std::make_unique<Mesh>();
        mesh->createSimple(device, allocator, vertices, indices);
        mesh->setName("Triangle");
        return mesh;
    }

    std::unique_ptr<Mesh> createQuadMesh(Vulkan::Device& device, Vulkan::MemoryAllocator& allocator,
        float width, float height) {
        auto vertices = Vertex::createQuadVertices();
        for (auto& v : vertices) {
            v.position.x *= width;
            v.position.y *= height;
        }

        std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

        auto mesh = std::make_unique<Mesh>();
        mesh->createSimple(device, allocator, vertices, indices);
        mesh->setName("Quad");
        return mesh;
    }

    std::unique_ptr<Mesh> createCubeMesh(Vulkan::Device& device, Vulkan::MemoryAllocator& allocator) {
        auto vertices = Vertex::createCubeVertices();
        auto indices = Vertex::createCubeIndices();

        auto mesh = std::make_unique<Mesh>();
        mesh->createSimple(device, allocator, vertices, indices);
        mesh->setName("Cube");
        return mesh;
    }

    std::unique_ptr<Mesh> createSphereMesh(Vulkan::Device& device, Vulkan::MemoryAllocator& allocator,
        float radius, int segments) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for (int lat = 0; lat <= segments; ++lat) {
            float theta = static_cast<float>(lat) * glm::pi<float>() / static_cast<float>(segments);
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);

            for (int lon = 0; lon <= segments; ++lon) {
                float phi = static_cast<float>(lon) * 2.0f * glm::pi<float>() / static_cast<float>(segments);
                float sinPhi = sin(phi);
                float cosPhi = cos(phi);

                Vertex v;
                v.position.x = radius * sinTheta * cosPhi;
                v.position.y = radius * cosTheta;
                v.position.z = radius * sinTheta * sinPhi;
                v.normal = glm::normalize(v.position);
                v.texCoord.x = static_cast<float>(lon) / static_cast<float>(segments);
                v.texCoord.y = static_cast<float>(lat) / static_cast<float>(segments);
                v.color = glm::vec3(1.0f);
                v.tangent = glm::vec3(0.0f);
                v.bitangent = glm::vec3(0.0f);
                vertices.push_back(v);
            }
        }

        for (int lat = 0; lat < segments; ++lat) {
            for (int lon = 0; lon < segments; ++lon) {
                uint32_t first = static_cast<uint32_t>((lat * (segments + 1)) + lon);
                uint32_t second = static_cast<uint32_t>(((lat + 1) * (segments + 1)) + lon);

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }

        auto mesh = std::make_unique<Mesh>();
        mesh->createSimple(device, allocator, vertices, indices);
        mesh->setName("Sphere");
        return mesh;
    }

    std::unique_ptr<Mesh> createPlaneMesh(Vulkan::Device& device, Vulkan::MemoryAllocator& allocator,
        float width, float depth) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        float halfWidth = width * 0.5f;
        float halfDepth = depth * 0.5f;

        vertices.push_back({ {-halfWidth, 0.0f, -halfDepth}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} });
        vertices.push_back({ { halfWidth, 0.0f, -halfDepth}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} });
        vertices.push_back({ { halfWidth, 0.0f,  halfDepth}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} });
        vertices.push_back({ {-halfWidth, 0.0f,  halfDepth}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} });

        indices = { 0, 1, 2, 2, 3, 0 };

        auto mesh = std::make_unique<Mesh>();
        mesh->createSimple(device, allocator, vertices, indices);
        mesh->setName("Plane");
        return mesh;
    }

}