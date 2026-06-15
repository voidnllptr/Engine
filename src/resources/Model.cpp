#include "Model.hpp"
#include <cstring>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <filesystem>

#define TINYOBJLOADER_NO_CONSTEXPR
#define TINYOBJLOADER_NOEXCEPT
#define TINYOBJLOADER_USE_CPP11_FEATURES
#define TINYOBJLOADER_IMPLEMENTATION

#include <tiny_obj_loader.h>

namespace Resources {

    Model::Model(Model&& other) noexcept
        : m_name(std::move(other.m_name))
        , m_meshes(std::move(other.m_meshes))
        , m_materials(std::move(other.m_materials))
        , m_position(other.m_position)
        , m_rotation(other.m_rotation)
        , m_scale(other.m_scale)
        , m_device(other.m_device) {
        other.m_device = nullptr;
    }

    Model& Model::operator=(Model&& other) noexcept {
        if (this != &other) {
            cleanup();
            m_name = std::move(other.m_name);
            m_meshes = std::move(other.m_meshes);
            m_materials = std::move(other.m_materials);
            m_position = other.m_position;
            m_rotation = other.m_rotation;
            m_scale = other.m_scale;
            m_device = other.m_device;
            other.m_device = nullptr;
        }
        return *this;
    }

    bool Model::loadOBJ(Vulkan::Device& device, Vulkan::MemoryAllocator& allocator,
        VkCommandPool commandPool, VkQueue graphicsQueue,
        const std::string& filepath, const std::string& mtlBasePath) {
        return loadFromOBJ(device, allocator, commandPool, graphicsQueue, filepath, mtlBasePath);
    }

    bool Model::loadGLTF(Vulkan::Device& device, Vulkan::MemoryAllocator& allocator,
        VkCommandPool commandPool, VkQueue graphicsQueue,
        const std::string& filepath) {
        (void)device;
        (void)allocator;
        (void)commandPool;
        (void)graphicsQueue;
        (void)filepath;
        std::cerr << "GLTF loading not yet implemented. Use OBJ format." << std::endl;
        return false;
    }

    void Model::cleanup() {
        m_meshes.clear();
        m_materials.clear();
        m_device = nullptr;
    }

    void Model::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const {
        for (const auto& mesh : m_meshes) {
            if (!mesh->isValid()) continue;
            mesh->bind(commandBuffer);
            mesh->draw(commandBuffer, pipelineLayout);
        }
    }

    void Model::drawWithMaterials(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
        const std::vector<std::shared_ptr<Renderer::Material>>& materials) const {
        for (const auto& mesh : m_meshes) {
            if (!mesh->isValid()) continue;
            mesh->bind(commandBuffer);
            for (const auto& primitive : mesh->getPrimitives()) {
                if (primitive.materialIndex >= 0 &&
                    primitive.materialIndex < static_cast<int>(materials.size()) &&
                    materials[primitive.materialIndex]) {
                    materials[primitive.materialIndex]->bindDescriptorSets(commandBuffer, pipelineLayout);
                }
                vkCmdDrawIndexed(commandBuffer, primitive.indexCount, 1,
                    primitive.firstIndex, primitive.vertexOffset, 0);
            }
        }
    }

    void Model::setRotation(float angle, const glm::vec3& axis) {
        m_rotation = glm::angleAxis(glm::radians(angle), glm::normalize(axis));
    }

    glm::mat4 Model::getModelMatrix() const {
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m_position);
        glm::mat4 rotationMatrix = glm::toMat4(m_rotation);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), m_scale);
        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    bool Model::loadFromOBJ(Vulkan::Device& device, Vulkan::MemoryAllocator& allocator,
        VkCommandPool commandPool, VkQueue graphicsQueue,
        const std::string& filepath, const std::string& mtlBasePath) {

        m_device = &device;

        size_t lastSlash = filepath.find_last_of("/\\");
        m_name = (lastSlash != std::string::npos) ? filepath.substr(lastSlash + 1) : filepath;

        std::string basePath = mtlBasePath.empty() ?
            filepath.substr(0, filepath.find_last_of("/\\") + 1) :
            mtlBasePath;

        std::cout << "[Model] Loading OBJ: " << filepath << std::endl;
        std::cout << "[Model] Base path: " << basePath << std::endl;

        tinyobj::ObjReader reader;
        tinyobj::ObjReaderConfig readerConfig;
        readerConfig.mtl_search_path = basePath;
        readerConfig.triangulate = true;
        readerConfig.vertex_color = false;

        if (!reader.ParseFromFile(filepath, readerConfig)) {
            if (!reader.Error().empty()) {
                std::cerr << "[Model] TinyObjReader Error: " << reader.Error() << std::endl;
            }
            std::cerr << "[Model] Failed to load OBJ: " << filepath << std::endl;
            return false;
        }

        if (!reader.Warning().empty()) {
            std::cout << "[Model] TinyObjReader Warning: " << reader.Warning() << std::endl;
        }

        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();
        auto& materials = reader.GetMaterials();

        std::cout << "[Model] Vertices: " << attrib.vertices.size() / 3 << std::endl;
        std::cout << "[Model] Shapes: " << shapes.size() << std::endl;
        std::cout << "[Model] Materials: " << materials.size() << std::endl;

        for (size_t s = 0; s < shapes.size(); ++s) {
            const auto& shape = shapes[s];

            if (shape.mesh.indices.empty()) {
                std::cout << "[Model] Shape '" << shape.name << "' has no indices, skipping" << std::endl;
                continue;
            }

            std::cout << "[Model] Processing shape: " << shape.name << std::endl;
            std::cout << "[Model]   Indices: " << shape.mesh.indices.size() << std::endl;

            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            std::unordered_map<std::string, uint32_t> vertexMap;

            for (size_t i = 0; i < shape.mesh.indices.size(); ++i) {
                const auto& idx = shape.mesh.indices[i];

                std::string key = std::to_string(idx.vertex_index) + "_" +
                    std::to_string(idx.normal_index) + "_" +
                    std::to_string(idx.texcoord_index);

                if (vertexMap.find(key) == vertexMap.end()) {
                    Vertex vertex{};
                    if (idx.vertex_index >= 0 && idx.vertex_index * 3 + 2 < attrib.vertices.size()) {
                        vertex.position.x = attrib.vertices[3 * idx.vertex_index + 0];
                        vertex.position.y = attrib.vertices[3 * idx.vertex_index + 1];
                        vertex.position.z = attrib.vertices[3 * idx.vertex_index + 2];
                    }

                    if (idx.normal_index >= 0 && idx.normal_index * 3 + 2 < attrib.normals.size()) {
                        vertex.normal.x = attrib.normals[3 * idx.normal_index + 0];
                        vertex.normal.y = attrib.normals[3 * idx.normal_index + 1];
                        vertex.normal.z = attrib.normals[3 * idx.normal_index + 2];
                    }
                    else {
                        vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f);
                    }

                    if (idx.texcoord_index >= 0 && idx.texcoord_index * 2 + 1 < attrib.texcoords.size()) {
                        vertex.texCoord.x = attrib.texcoords[2 * idx.texcoord_index + 0];
                        vertex.texCoord.y = 1.0f - attrib.texcoords[2 * idx.texcoord_index + 1];
                    }
                    else {
                        vertex.texCoord = glm::vec2(0.0f);
                    }

                    vertex.color = glm::vec3(1.0f);
                    vertex.tangent = glm::vec3(0.0f);
                    vertex.bitangent = glm::vec3(0.0f);

                    vertexMap[key] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(vertexMap[key]);
            }

            if (!vertices.empty() && !indices.empty()) {
                std::vector<Primitive> primitives;
                Primitive singlePrimitive;
                singlePrimitive.firstIndex = 0;
                singlePrimitive.indexCount = static_cast<uint32_t>(indices.size());
                singlePrimitive.materialIndex = -1;
                singlePrimitive.vertexOffset = 0;
                primitives.push_back(singlePrimitive);

                auto mesh = std::make_unique<Mesh>();
                mesh->setName(shape.name);
                mesh->create(device, allocator, vertices, indices, primitives);
                m_meshes.push_back(std::move(mesh));

                std::cout << "[Model]   Created mesh with " << vertices.size()
                    << " vertices, " << indices.size() << " indices, "
                    << primitives.size() << " primitive" << std::endl;
            }
        }

        std::cout << "[Model] Load complete. Meshes: " << m_meshes.size() << std::endl;
        return !m_meshes.empty();
    }

    std::shared_ptr<Model> ModelManager::getModel(Vulkan::Device& device,
        Vulkan::MemoryAllocator& allocator,
        VkCommandPool commandPool,
        VkQueue graphicsQueue,
        const std::string& filepath) {

        auto it = m_modelCache.find(filepath);
        if (it != m_modelCache.end()) {
            std::cout << "[ModelManager] Returning cached model: " << filepath << std::endl;
            return it->second;
        }

        std::cout << "[ModelManager] Loading new model: " << filepath << std::endl;

        auto model = std::make_shared<Model>();
        if (model->loadOBJ(device, allocator, commandPool, graphicsQueue, filepath)) {
            m_modelCache[filepath] = model;
            std::cout << "[ModelManager] Model loaded and cached" << std::endl;
            return model;
        }

        std::cerr << "[ModelManager] Failed to load model: " << filepath << std::endl;
        return nullptr;
    }

    void ModelManager::clear() {
        m_modelCache.clear();
        std::cout << "[ModelManager] Cache cleared" << std::endl;
    }

    void ModelManager::removeModel(const std::string& filepath) {
        m_modelCache.erase(filepath);
        std::cout << "[ModelManager] Removed from cache: " << filepath << std::endl;
    }

    std::shared_ptr<Model> Model::createCube(Vulkan::Device& device,
        Vulkan::MemoryAllocator& allocator,
        VkCommandPool commandPool,
        VkQueue graphicsQueue) {
        (void)commandPool;
        (void)graphicsQueue;

        auto mesh = createCubeMesh(device, allocator);

        auto model = std::make_shared<Model>();
        model->m_meshes.push_back(std::move(mesh));
        model->m_name = "Cube";
        model->m_device = &device;

        return model;
    }

    std::shared_ptr<Model> Model::createSphere(Vulkan::Device& device,
        Vulkan::MemoryAllocator& allocator,
        VkCommandPool commandPool,
        VkQueue graphicsQueue,
        float radius, int segments) {
        (void)commandPool;
        (void)graphicsQueue;

        auto mesh = createSphereMesh(device, allocator, radius, segments);

        auto model = std::make_shared<Model>();
        model->m_meshes.push_back(std::move(mesh));
        model->m_name = "Sphere";
        model->m_device = &device;

        return model;
    }

    std::shared_ptr<Model> Model::createPlane(Vulkan::Device& device,
        Vulkan::MemoryAllocator& allocator,
        VkCommandPool commandPool,
        VkQueue graphicsQueue,
        float width, float depth) {
        (void)commandPool;
        (void)graphicsQueue;

        auto mesh = createPlaneMesh(device, allocator, width, depth);

        auto model = std::make_shared<Model>();
        model->m_meshes.push_back(std::move(mesh));
        model->m_name = "Plane";
        model->m_device = &device;

        return model;
    }

}