#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <unordered_map>
#include <string>
#include <array>
#include "../resources/Shader.hpp"
#include "../resources/Texture.hpp"
#include "../utils/Math.hpp"

namespace Renderer {

    struct MaterialParameters {
        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        float metallicFactor = 0.0f;
        float roughnessFactor = 1.0f;
        float emissiveFactor = 0.0f;
        float alphaCutoff = 0.5f;
        glm::vec3 emissiveColor = glm::vec3(0.0f);
        float normalScale = 1.0f;
        float occlusionStrength = 1.0f;
        bool doubleSided = false;
        bool alphaBlend = false;
        bool alphaTest = false;

        bool operator==(const MaterialParameters& other) const {
            return baseColorFactor == other.baseColorFactor &&
                metallicFactor == other.metallicFactor &&
                roughnessFactor == other.roughnessFactor &&
                emissiveFactor == other.emissiveFactor &&
                alphaCutoff == other.alphaCutoff &&
                emissiveColor == other.emissiveColor &&
                normalScale == other.normalScale &&
                occlusionStrength == other.occlusionStrength &&
                doubleSided == other.doubleSided &&
                alphaBlend == other.alphaBlend &&
                alphaTest == other.alphaTest;
        }

        bool operator!=(const MaterialParameters& other) const {
            return !(*this == other);
        }
    };

    enum class TextureSlot {
        Albedo = 0,
        Normal = 1,
        MetallicRoughness = 2,
        AmbientOcclusion = 3,
        Emissive = 4,
        Height = 5,
        Opacity = 6,
        Custom0 = 7,
        Custom1 = 8,
        Custom2 = 9,
        Custom3 = 10,
        Count = 11
    };

    class Material {
    public:
        Material() = default;
        ~Material() = default;

        void init(const std::shared_ptr<ShaderProgram>& shader);
        void init(const std::shared_ptr<ShaderProgram>& shader,
            const std::shared_ptr<Resources::Texture>& albedoTexture);

        void setTexture(TextureSlot slot, const std::shared_ptr<Resources::Texture>& texture);

        void setAlbedoTexture(const std::shared_ptr<Resources::Texture>& texture) {
            setTexture(TextureSlot::Albedo, texture);
        }

        void setNormalTexture(const std::shared_ptr<Resources::Texture>& texture) {
            setTexture(TextureSlot::Normal, texture);
        }

        void setMetallicRoughnessTexture(const std::shared_ptr<Resources::Texture>& texture) {
            setTexture(TextureSlot::MetallicRoughness, texture);
        }

        void setAOTexture(const std::shared_ptr<Resources::Texture>& texture) {
            setTexture(TextureSlot::AmbientOcclusion, texture);
        }

        void setEmissiveTexture(const std::shared_ptr<Resources::Texture>& texture) {
            setTexture(TextureSlot::Emissive, texture);
        }

        std::shared_ptr<Resources::Texture> getTexture(TextureSlot slot) const;
        bool hasTexture(TextureSlot slot) const;

        void setParameters(const MaterialParameters& parameters) { m_parameters = parameters; }
        const MaterialParameters& getParameters() const { return m_parameters; }

        void setBaseColorFactor(const glm::vec4& color) { m_parameters.baseColorFactor = color; }
        void setMetallicFactor(float metallic) { m_parameters.metallicFactor = metallic; }
        void setRoughnessFactor(float roughness) { m_parameters.roughnessFactor = roughness; }
        void setEmissiveFactor(float emissive) { m_parameters.emissiveFactor = emissive; }
        void setEmissiveColor(const glm::vec3& color) { m_parameters.emissiveColor = color; }
        void setNormalScale(float scale) { m_parameters.normalScale = scale; }
        void setDoubleSided(bool doubleSided) { m_parameters.doubleSided = doubleSided; }
        void setAlphaBlend(bool alphaBlend) { m_parameters.alphaBlend = alphaBlend; }

        std::shared_ptr<ShaderProgram> getShader() const { return m_shader; }
        void setShader(const std::shared_ptr<ShaderProgram>& shader) { m_shader = shader; }

        void bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const;
        void bindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
            uint32_t firstSet = 0) const;

        void allocateDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool,
            VkDescriptorSetLayout layout);

        VkDescriptorSet getDescriptorSet() const { return m_descriptorSet; }
        bool hasDescriptorSet() const { return m_descriptorSet != VK_NULL_HANDLE; }
        void updateDescriptorSet(VkDevice device);

        uint64_t getPipelineKey() const;
        void fillPipelineCreateInfo(VkGraphicsPipelineCreateInfo& createInfo,
            VkPipelineLayout pipelineLayout,
            VkRenderPass renderPass) const;

        static std::shared_ptr<Material> createDefault(const std::shared_ptr<ShaderProgram>& shader);
        static std::shared_ptr<Material> createTextured(const std::shared_ptr<ShaderProgram>& shader,
            const std::shared_ptr<Resources::Texture>& texture);

    private:
        void updateDescriptorSetInternal(VkDevice device);
        VkDescriptorImageInfo getImageInfo(TextureSlot slot) const;

        std::shared_ptr<ShaderProgram> m_shader;
        std::array<std::shared_ptr<Resources::Texture>, static_cast<size_t>(TextureSlot::Count)> m_textures;
        MaterialParameters m_parameters;

        VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        bool m_descriptorSetDirty = true;
    };

    class MaterialManager {
    public:
        MaterialManager() = default;
        ~MaterialManager() = default;

        std::shared_ptr<Material> getMaterial(const std::shared_ptr<ShaderProgram>& shader,
            const MaterialParameters& parameters);

        void clear();
        size_t getCachedCount() const { return m_materialCache.size(); }

    private:
        struct MaterialKey {
            std::shared_ptr<ShaderProgram> shader;
            MaterialParameters parameters;

            bool operator==(const MaterialKey& other) const {
                return shader == other.shader && parameters == other.parameters;
            }
        };

        struct MaterialKeyHash {
            size_t operator()(const MaterialKey& key) const {
                size_t h1 = std::hash<std::shared_ptr<ShaderProgram>>{}(key.shader);
                size_t h2 = std::hash<float>{}(key.parameters.metallicFactor) ^
                    (std::hash<float>{}(key.parameters.roughnessFactor) << 1) ^
                    (std::hash<float>{}(key.parameters.emissiveFactor) << 2);
                return h1 ^ (h2 << 1);
            }
        };

        std::unordered_map<MaterialKey, std::shared_ptr<Material>, MaterialKeyHash> m_materialCache;
    };

}