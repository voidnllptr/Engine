#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace Vulkan {

    class MemoryAllocator {
    public:
        MemoryAllocator() = default;
        ~MemoryAllocator() { cleanup(); }

        MemoryAllocator(const MemoryAllocator&) = delete;
        MemoryAllocator& operator=(const MemoryAllocator&) = delete;

        MemoryAllocator(MemoryAllocator&& other) noexcept;
        MemoryAllocator& operator=(MemoryAllocator&& other) noexcept;

        void init(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
        void cleanup();

        bool isValid() const { return m_allocator != VK_NULL_HANDLE; }
        VmaAllocator get() const { return m_allocator; }

        VkBuffer createBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VmaMemoryUsage memoryUsage,
            VmaAllocation& outAllocation,
            const void* data = nullptr
        );

        VkBuffer createVertexBuffer(VkDeviceSize size, VmaAllocation& outAllocation, const void* data = nullptr);
        VkBuffer createIndexBuffer(VkDeviceSize size, VmaAllocation& outAllocation, const void* data = nullptr);
        VkBuffer createUniformBuffer(VkDeviceSize size, VmaAllocation& outAllocation, const void* data = nullptr);
        VkBuffer createStagingBuffer(VkDeviceSize size, VmaAllocation& outAllocation, const void* data = nullptr);

        void destroyBuffer(VkBuffer buffer, VmaAllocation allocation);

    private:
        VmaAllocator m_allocator = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
    };

}