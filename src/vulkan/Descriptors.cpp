#include "Descriptors.hpp"
#include "Device.hpp"
#include <stdexcept>

namespace Vulkan {

    DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) noexcept
        : m_layout(other.m_layout)
        , m_device(other.m_device) {
        other.m_layout = VK_NULL_HANDLE;
        other.m_device = nullptr;
    }

    DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& other) noexcept {
        if (this != &other) {
            cleanup();
            m_layout = other.m_layout;
            m_device = other.m_device;
            other.m_layout = VK_NULL_HANDLE;
            other.m_device = nullptr;
        }
        return *this;
    }

    void DescriptorSetLayout::create(Device& device, const std::vector<VkDescriptorSetLayoutBinding>& bindings) {
        cleanup();
        m_device = &device;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device.get(), &layoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout!");
        }
    }

    void DescriptorSetLayout::cleanup() {
        if (m_device != nullptr && m_layout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(m_device->get(), m_layout, nullptr);
            m_layout = VK_NULL_HANDLE;
        }
        m_device = nullptr;
    }

    DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept
        : m_pool(other.m_pool)
        , m_device(other.m_device)
        , m_maxSets(other.m_maxSets)
        , m_allocatedSets(other.m_allocatedSets) {
        other.m_pool = VK_NULL_HANDLE;
        other.m_device = nullptr;
        other.m_maxSets = 0;
        other.m_allocatedSets = 0;
    }

    DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other) noexcept {
        if (this != &other) {
            cleanup();
            m_pool = other.m_pool;
            m_device = other.m_device;
            m_maxSets = other.m_maxSets;
            m_allocatedSets = other.m_allocatedSets;
            other.m_pool = VK_NULL_HANDLE;
            other.m_device = nullptr;
            other.m_maxSets = 0;
            other.m_allocatedSets = 0;
        }
        return *this;
    }

    void DescriptorPool::create(Device& device, const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets) {
        cleanup();
        m_device = &device;
        m_maxSets = maxSets;
        m_allocatedSets = 0;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = maxSets;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

        if (vkCreateDescriptorPool(device.get(), &poolInfo, nullptr, &m_pool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor pool!");
        }
    }

    void DescriptorPool::cleanup() {
        if (m_device != nullptr && m_pool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(m_device->get(), m_pool, nullptr);
            m_pool = VK_NULL_HANDLE;
        }
        m_device = nullptr;
        m_maxSets = 0;
        m_allocatedSets = 0;
    }

    void DescriptorPool::allocateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet& outSet) {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        if (vkAllocateDescriptorSets(m_device->get(), &allocInfo, &outSet) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor set!");
        }
        m_allocatedSets++;
    }

    void DescriptorPool::allocateDescriptorSets(VkDescriptorSetLayout layout, uint32_t count, std::vector<VkDescriptorSet>& outSets) {
        outSets.resize(count);
        std::vector<VkDescriptorSetLayout> layouts(count, layout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_pool;
        allocInfo.descriptorSetCount = count;
        allocInfo.pSetLayouts = layouts.data();

        if (vkAllocateDescriptorSets(m_device->get(), &allocInfo, outSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor sets!");
        }
        m_allocatedSets += count;
    }

    void DescriptorPool::resetPool() {
        if (m_device != nullptr && m_pool != VK_NULL_HANDLE) {
            vkResetDescriptorPool(m_device->get(), m_pool, 0);
            m_allocatedSets = 0;
        }
    }

    float DescriptorPool::getUsageRatio() const {
        if (m_maxSets == 0) return 0.0f;
        return static_cast<float>(m_allocatedSets) / static_cast<float>(m_maxSets);
    }

    void DescriptorSetWriter::writeUniformBuffer(VkDescriptorSet dstSet, uint32_t binding,
        VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer;
        bufferInfo.offset = offset;
        bufferInfo.range = range;
        m_bufferInfos.push_back(bufferInfo);

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = dstSet;
        write.dstBinding = binding;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.pBufferInfo = &m_bufferInfos.back();
        m_writes.push_back(write);
    }

    void DescriptorSetWriter::writeImageSampler(VkDescriptorSet dstSet, uint32_t binding,
        VkImageView imageView, VkSampler sampler,
        VkImageLayout layout) {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = layout;
        imageInfo.imageView = imageView;
        imageInfo.sampler = sampler;
        m_imageInfos.push_back(imageInfo);

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = dstSet;
        write.dstBinding = binding;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = &m_imageInfos.back();
        m_writes.push_back(write);
    }

    void DescriptorSetWriter::writeStorageBuffer(VkDescriptorSet dstSet, uint32_t binding,
        VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer;
        bufferInfo.offset = offset;
        bufferInfo.range = range;
        m_bufferInfos.push_back(bufferInfo);

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = dstSet;
        write.dstBinding = binding;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        write.pBufferInfo = &m_bufferInfos.back();
        m_writes.push_back(write);
    }

    void DescriptorSetWriter::writeUniformBufferDynamic(VkDescriptorSet dstSet, uint32_t binding,
        VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer;
        bufferInfo.offset = offset;
        bufferInfo.range = range;
        m_bufferInfos.push_back(bufferInfo);

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = dstSet;
        write.dstBinding = binding;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        write.pBufferInfo = &m_bufferInfos.back();
        m_writes.push_back(write);
    }

    void DescriptorSetWriter::update(VkDevice device) {
        if (!m_writes.empty()) {
            vkUpdateDescriptorSets(device, static_cast<uint32_t>(m_writes.size()), m_writes.data(), 0, nullptr);
        }
    }

    void DescriptorSetWriter::clear() {
        m_writes.clear();
        m_bufferInfos.clear();
        m_imageInfos.clear();
    }

    void DescriptorManager::init(Device& device) {
        m_device = &device;

        createTextureLayout();
        createUniformLayout();
        createCombinedLayout();
        createMaterialLayout();

        createTexturePool();
        createUniformPool();
        createCombinedPool();
    }

    void DescriptorManager::createTextureLayout() {
        std::vector<VkDescriptorSetLayoutBinding> bindings(1);
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        m_textureLayout.create(*m_device, bindings);
    }

    void DescriptorManager::createUniformLayout() {
        std::vector<VkDescriptorSetLayoutBinding> bindings(1);
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        m_uniformLayout.create(*m_device, bindings);
    }

    void DescriptorManager::createCombinedLayout() {
        std::vector<VkDescriptorSetLayoutBinding> bindings(3);
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings[2].binding = 2;
        bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[2].descriptorCount = 1;
        bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        m_combinedLayout.create(*m_device, bindings);
    }

    void DescriptorManager::createMaterialLayout() {
        std::vector<VkDescriptorSetLayoutBinding> bindings(3);
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings[2].binding = 2;
        bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[2].descriptorCount = 1;
        bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        m_materialLayout.create(*m_device, bindings);
    }

    void DescriptorManager::createTexturePool() {
        std::vector<VkDescriptorPoolSize> poolSizes(1);
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[0].descriptorCount = 100;
        m_texturePool.create(*m_device, poolSizes, 100);
    }

    void DescriptorManager::createUniformPool() {
        std::vector<VkDescriptorPoolSize> poolSizes(1);
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = 100;
        m_uniformPool.create(*m_device, poolSizes, 100);
    }

    void DescriptorManager::createCombinedPool() {
        std::vector<VkDescriptorPoolSize> poolSizes(2);
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = 100;
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = 100;
        m_combinedPool.create(*m_device, poolSizes, 100);
    }

    void DescriptorManager::cleanup() {
        for (auto& pool : m_dynamicPools) {
            if (pool) {
                pool->cleanup();
            }
        }
        m_dynamicPools.clear();
        m_availablePools.clear();

        m_texturePool.cleanup();
        m_uniformPool.cleanup();
        m_combinedPool.cleanup();

        m_textureLayout.cleanup();
        m_uniformLayout.cleanup();
        m_combinedLayout.cleanup();
        m_materialLayout.cleanup();

        m_device = nullptr;
    }

    VkDescriptorSet DescriptorManager::allocateTextureSet() {
        VkDescriptorSet set;
        m_texturePool.allocateDescriptorSet(m_textureLayout.get(), set);
        return set;
    }

    VkDescriptorSet DescriptorManager::allocateUniformSet() {
        VkDescriptorSet set;
        m_uniformPool.allocateDescriptorSet(m_uniformLayout.get(), set);
        return set;
    }

    VkDescriptorSet DescriptorManager::allocateCombinedSet() {
        VkDescriptorSet set;
        m_combinedPool.allocateDescriptorSet(m_combinedLayout.get(), set);
        return set;
    }

    VkDescriptorSet DescriptorManager::allocateMaterialSet() {
        VkDescriptorSet set;

        for (auto& pool : m_availablePools) {
            try {
                pool->allocateDescriptorSet(m_materialLayout.get(), set);
                if (pool->getUsageRatio() > 0.8f) {
                    auto it = std::find(m_availablePools.begin(), m_availablePools.end(), pool);
                    if (it != m_availablePools.end()) {
                        m_availablePools.erase(it);
                    }
                }
                return set;
            }
            catch (const std::runtime_error&) {
                continue;
            }
        }

        auto newPool = createDynamicPool(getDefaultMaterialPoolSizes(), 16);
        newPool->allocateDescriptorSet(m_materialLayout.get(), set);
        m_availablePools.push_back(newPool);

        return set;
    }

    void DescriptorManager::allocateTextureSets(uint32_t count, std::vector<VkDescriptorSet>& outSets) {
        m_texturePool.allocateDescriptorSets(m_textureLayout.get(), count, outSets);
    }

    void DescriptorManager::allocateUniformSets(uint32_t count, std::vector<VkDescriptorSet>& outSets) {
        m_uniformPool.allocateDescriptorSets(m_uniformLayout.get(), count, outSets);
    }

    std::shared_ptr<DescriptorPool> DescriptorManager::createDynamicPool(
        const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets) {

        auto pool = std::make_shared<DescriptorPool>();
        pool->create(*m_device, poolSizes, maxSets);
        m_dynamicPools.push_back(pool);
        return pool;
    }

    void DescriptorManager::returnDynamicPool(std::shared_ptr<DescriptorPool> pool) {
        if (pool) {
            pool->resetPool();
            m_availablePools.push_back(pool);
        }
    }

    std::vector<VkDescriptorPoolSize> DescriptorManager::getDefaultMaterialPoolSizes() const {
        std::vector<VkDescriptorPoolSize> poolSizes(3);
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = 32;
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = 32;
        poolSizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLER;
        poolSizes[2].descriptorCount = 16;
        return poolSizes;
    }

}