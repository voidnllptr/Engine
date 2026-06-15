#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

namespace Vulkan {

    class Device;

    class DescriptorSetLayout {
    public:
        DescriptorSetLayout() = default;
        ~DescriptorSetLayout() { cleanup(); }

        DescriptorSetLayout(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

        DescriptorSetLayout(DescriptorSetLayout&& other) noexcept;
        DescriptorSetLayout& operator=(DescriptorSetLayout&& other) noexcept;

        void create(Device& device, const std::vector<VkDescriptorSetLayoutBinding>& bindings);
        void cleanup();

        VkDescriptorSetLayout get() const { return m_layout; }
        bool isValid() const { return m_layout != VK_NULL_HANDLE; }

    private:
        VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
        Device* m_device = nullptr;
    };

    class DescriptorPool {
    public:
        DescriptorPool() = default;
        ~DescriptorPool() { cleanup(); }

        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;

        DescriptorPool(DescriptorPool&& other) noexcept;
        DescriptorPool& operator=(DescriptorPool&& other) noexcept;

        void create(Device& device, const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets);
        void cleanup();

        void allocateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet& outSet);
        void allocateDescriptorSets(VkDescriptorSetLayout layout, uint32_t count, std::vector<VkDescriptorSet>& outSets);
        void resetPool();

        VkDescriptorPool get() const { return m_pool; }
        bool isValid() const { return m_pool != VK_NULL_HANDLE; }

        float getUsageRatio() const;

    private:
        VkDescriptorPool m_pool = VK_NULL_HANDLE;
        Device* m_device = nullptr;
        uint32_t m_maxSets = 0;
        uint32_t m_allocatedSets = 0;
    };

    class DescriptorSetWriter {
    public:
        DescriptorSetWriter() = default;

        void writeUniformBuffer(VkDescriptorSet dstSet, uint32_t binding,
            VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);

        void writeImageSampler(VkDescriptorSet dstSet, uint32_t binding,
            VkImageView imageView, VkSampler sampler,
            VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        void writeStorageBuffer(VkDescriptorSet dstSet, uint32_t binding,
            VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);

        void writeUniformBufferDynamic(VkDescriptorSet dstSet, uint32_t binding,
            VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);

        void update(VkDevice device);
        void clear();

    private:
        std::vector<VkWriteDescriptorSet> m_writes;
        std::vector<VkDescriptorBufferInfo> m_bufferInfos;
        std::vector<VkDescriptorImageInfo> m_imageInfos;
    };

    class DescriptorManager {
    public:
        DescriptorManager() = default;
        ~DescriptorManager() { cleanup(); }

        DescriptorManager(const DescriptorManager&) = delete;
        DescriptorManager& operator=(const DescriptorManager&) = delete;

        void init(Device& device);
        void cleanup();

        DescriptorSetLayout& getTextureLayout() { return m_textureLayout; }
        DescriptorSetLayout& getUniformLayout() { return m_uniformLayout; }
        DescriptorSetLayout& getCombinedLayout() { return m_combinedLayout; }
        DescriptorSetLayout& getMaterialLayout() { return m_materialLayout; }

        DescriptorPool& getTexturePool() { return m_texturePool; }
        DescriptorPool& getUniformPool() { return m_uniformPool; }
        DescriptorPool& getCombinedPool() { return m_combinedPool; }

        VkDescriptorSet allocateTextureSet();
        VkDescriptorSet allocateUniformSet();
        VkDescriptorSet allocateCombinedSet();
        VkDescriptorSet allocateMaterialSet();

        void allocateTextureSets(uint32_t count, std::vector<VkDescriptorSet>& outSets);
        void allocateUniformSets(uint32_t count, std::vector<VkDescriptorSet>& outSets);

        std::shared_ptr<DescriptorPool> createDynamicPool(const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets);
        void returnDynamicPool(std::shared_ptr<DescriptorPool> pool);

    private:
        void createTextureLayout();
        void createUniformLayout();
        void createCombinedLayout();
        void createMaterialLayout();
        void createTexturePool();
        void createUniformPool();
        void createCombinedPool();

        std::vector<VkDescriptorPoolSize> getDefaultMaterialPoolSizes() const;

        Device* m_device = nullptr;

        DescriptorSetLayout m_textureLayout;
        DescriptorSetLayout m_uniformLayout;
        DescriptorSetLayout m_combinedLayout;
        DescriptorSetLayout m_materialLayout;

        DescriptorPool m_texturePool;
        DescriptorPool m_uniformPool;
        DescriptorPool m_combinedPool;

        std::vector<std::shared_ptr<DescriptorPool>> m_dynamicPools;
        std::vector<std::shared_ptr<DescriptorPool>> m_availablePools;
    };

}