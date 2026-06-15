#include "Material.hpp"
#include <cstring>

namespace Renderer {

    void Material::init(const std::shared_ptr<ShaderProgram>& shader) {
        m_shader = shader;
        m_descriptorSetDirty = true;
    }

    void Material::init(const std::shared_ptr<ShaderProgram>& shader,
        const std::shared_ptr<Resources::Texture>& albedoTexture) {
        m_shader = shader;
        m_textures[static_cast<size_t>(TextureSlot::Albedo)] = albedoTexture;
        m_descriptorSetDirty = true;
    }

    void Material::setTexture(TextureSlot slot, const std::shared_ptr<Resources::Texture>& texture) {
        size_t index = static_cast<size_t>(slot);
        if (index < m_textures.size()) {
            m_textures[index] = texture;
            m_descriptorSetDirty = true;
        }
    }

    std::shared_ptr<Resources::Texture> Material::getTexture(TextureSlot slot) const {
        size_t index = static_cast<size_t>(slot);
        if (index < m_textures.size()) {
            return m_textures[index];
        }
        return nullptr;
    }

    bool Material::hasTexture(TextureSlot slot) const {
        size_t index = static_cast<size_t>(slot);
        return index < m_textures.size() && m_textures[index] != nullptr;
    }

    void Material::bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const {
        bindDescriptorSets(commandBuffer, pipelineLayout);
    }

    void Material::bindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
        uint32_t firstSet) const {
        if (m_descriptorSet != VK_NULL_HANDLE) {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout, firstSet, 1, &m_descriptorSet, 0, nullptr);
        }
    }

    void Material::allocateDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool,
        VkDescriptorSetLayout layout) {
        m_device = device;

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        if (vkAllocateDescriptorSets(device, &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor set for material!");
        }

        m_descriptorSetDirty = true;
        updateDescriptorSet(device);
    }

    void Material::updateDescriptorSet(VkDevice device) {
        if (m_descriptorSetDirty && m_descriptorSet != VK_NULL_HANDLE) {
            updateDescriptorSetInternal(device);
            m_descriptorSetDirty = false;
        }
    }

    void Material::updateDescriptorSetInternal(VkDevice device) {
        std::vector<VkWriteDescriptorSet> writes;
        std::vector<VkDescriptorImageInfo> imageInfos;
        imageInfos.reserve(m_textures.size());

        for (size_t i = 0; i < m_textures.size(); ++i) {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            auto texture = m_textures[i];
            if (texture && texture->isValid()) {
                imageInfo.imageView = texture->getImageView();
                imageInfo.sampler = texture->getSampler();
            }
            else {
                continue;
            }

            imageInfos.push_back(imageInfo);

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = m_descriptorSet;
            write.dstBinding = static_cast<uint32_t>(i);
            write.dstArrayElement = 0;
            write.descriptorCount = 1;
            write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.pImageInfo = &imageInfos.back();
            writes.push_back(write);
        }

        if (!writes.empty()) {
            vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
        }
    }

    VkDescriptorImageInfo Material::getImageInfo(TextureSlot slot) const {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        auto texture = getTexture(slot);
        if (texture && texture->isValid()) {
            imageInfo.imageView = texture->getImageView();
            imageInfo.sampler = texture->getSampler();
        }

        return imageInfo;
    }

    uint64_t Material::getPipelineKey() const {
        uint64_t key = 0;

        if (m_shader) {
            key ^= reinterpret_cast<uint64_t>(m_shader.get());
        }

        key ^= (static_cast<uint64_t>(m_parameters.metallicFactor * 1000.0f) << 1);
        key ^= (static_cast<uint64_t>(m_parameters.roughnessFactor * 1000.0f) << 2);
        key ^= (static_cast<uint64_t>(m_parameters.alphaBlend) << 3);
        key ^= (static_cast<uint64_t>(m_parameters.alphaTest) << 4);
        key ^= (static_cast<uint64_t>(m_parameters.doubleSided) << 5);

        for (size_t i = 0; i < m_textures.size(); ++i) {
            if (m_textures[i] && m_textures[i]->isValid()) {
                key ^= (1ULL << (i + 16));
            }
        }

        return key;
    }

    void Material::fillPipelineCreateInfo(VkGraphicsPipelineCreateInfo& createInfo,
        VkPipelineLayout pipelineLayout,
        VkRenderPass renderPass) const {
        if (m_shader) {
            auto stages = m_shader->getStageCreateInfos();
            createInfo.stageCount = static_cast<uint32_t>(stages.size());
            createInfo.pStages = stages.data();
        }

        createInfo.layout = pipelineLayout;
        createInfo.renderPass = renderPass;
    }

    std::shared_ptr<Material> Material::createDefault(const std::shared_ptr<ShaderProgram>& shader) {
        auto material = std::make_shared<Material>();
        material->init(shader);

        MaterialParameters params;
        params.baseColorFactor = glm::vec4(1.0f);
        params.metallicFactor = 0.0f;
        params.roughnessFactor = 1.0f;
        material->setParameters(params);

        return material;
    }

    std::shared_ptr<Material> Material::createTextured(const std::shared_ptr<ShaderProgram>& shader,
        const std::shared_ptr<Resources::Texture>& texture) {
        auto material = std::make_shared<Material>();
        material->init(shader, texture);
        return material;
    }

    std::shared_ptr<Material> MaterialManager::getMaterial(const std::shared_ptr<ShaderProgram>& shader,
        const MaterialParameters& parameters) {
        MaterialKey key{ shader, parameters };

        auto it = m_materialCache.find(key);
        if (it != m_materialCache.end()) {
            return it->second;
        }

        auto material = std::make_shared<Material>();
        material->init(shader);
        material->setParameters(parameters);

        m_materialCache[key] = material;
        return material;
    }

    void MaterialManager::clear() {
        m_materialCache.clear();
    }

}