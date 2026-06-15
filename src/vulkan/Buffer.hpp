#pragma once

#include <vulkan/vulkan.h>
#include "Memory.hpp"

namespace Vulkan {

    class Buffer {
    public:
        Buffer() = default;
        ~Buffer() { cleanup(); }

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        Buffer(Buffer&& other) noexcept;
        Buffer& operator=(Buffer&& other) noexcept;

        void create(MemoryAllocator& allocator, VkDeviceSize size, VkBufferUsageFlags usage, const void* data = nullptr);
        void createVertex(MemoryAllocator& allocator, const void* data, VkDeviceSize size);
        void createIndex(MemoryAllocator& allocator, const void* data, VkDeviceSize size);
        void createUniform(MemoryAllocator& allocator, VkDeviceSize size);
        void createStaging(MemoryAllocator& allocator, const void* data, VkDeviceSize size);

        void cleanup();

        void upload(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);

        void bindVertex(VkCommandBuffer commandBuffer, VkDeviceSize offset = 0) const;
        void bindIndex(VkCommandBuffer commandBuffer, VkDeviceSize offset = 0) const;

        VkBuffer get() const { return m_buffer; }
        VmaAllocation getAllocation() const { return m_allocation; }
        VkDeviceSize getSize() const { return m_size; }
        bool isValid() const { return m_buffer != VK_NULL_HANDLE; }

    private:
        VkBuffer m_buffer = VK_NULL_HANDLE;
        VmaAllocation m_allocation = VK_NULL_HANDLE;
        VkDeviceSize m_size = 0;
        MemoryAllocator* m_allocator = nullptr;
    };

}