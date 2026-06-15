#include "Memory.hpp"
#include <stdexcept>
#include <cstring>
#include <cstdio>

#define VMA_DEBUG_LOG_FORMAT(format, ...) \
    printf("[VMA] " format "\n", ##__VA_ARGS__)

#define VMA_LEAK_LOG_FORMAT(format, ...) \
    printf("[VMA LEAK] " format "\n", ##__VA_ARGS__)

#define VMA_DEBUG_LEAK 1

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace Vulkan {

    MemoryAllocator::MemoryAllocator(MemoryAllocator&& other) noexcept
        : m_allocator(other.m_allocator)
        , m_device(other.m_device) {
        other.m_allocator = VK_NULL_HANDLE;
        other.m_device = VK_NULL_HANDLE;
    }

    MemoryAllocator& MemoryAllocator::operator=(MemoryAllocator&& other) noexcept {
        if (this != &other) {
            cleanup();
            m_allocator = other.m_allocator;
            m_device = other.m_device;
            other.m_allocator = VK_NULL_HANDLE;
            other.m_device = VK_NULL_HANDLE;
        }
        return *this;
    }

    void MemoryAllocator::init(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device) {
        m_device = device;

        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device = device;
        allocatorInfo.instance = instance;
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;

        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;

        if (vmaCreateAllocator(&allocatorInfo, &m_allocator) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create VMA allocator!");
        }

        printf("[VMA] Allocator created successfully\n");
    }

    void MemoryAllocator::cleanup() {
        if (m_allocator != VK_NULL_HANDLE) {
            printf("[VMA] Destroying allocator...\n");
            vmaDestroyAllocator(m_allocator);
            m_allocator = VK_NULL_HANDLE;
            printf("[VMA] Allocator destroyed\n");
        }
        m_device = VK_NULL_HANDLE;
    }

    VkBuffer MemoryAllocator::createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage,
        VmaAllocation& outAllocation,
        const void* data) {

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = memoryUsage;
        allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VkBuffer buffer;
        VmaAllocationInfo allocInfoResult;

        VkResult result = vmaCreateBuffer(
            m_allocator,
            &bufferInfo,
            &allocInfo,
            &buffer,
            &outAllocation,
            &allocInfoResult
        );

        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer!");
        }

        if (data != nullptr) {
            memcpy(allocInfoResult.pMappedData, data, static_cast<size_t>(size));
        }

        return buffer;
    }

    VkBuffer MemoryAllocator::createVertexBuffer(VkDeviceSize size, VmaAllocation& outAllocation, const void* data) {
        printf("[VMA] Creating vertex buffer: size=%llu bytes\n", (unsigned long long)size);
        return createBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, outAllocation, data);
    }

    VkBuffer MemoryAllocator::createIndexBuffer(VkDeviceSize size, VmaAllocation& outAllocation, const void* data) {
        printf("[VMA] Creating index buffer: size=%llu bytes\n", (unsigned long long)size);
        return createBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, outAllocation, data);
    }

    VkBuffer MemoryAllocator::createUniformBuffer(VkDeviceSize size, VmaAllocation& outAllocation, const void* data) {
        printf("[VMA] Creating uniform buffer: size=%llu bytes\n", (unsigned long long)size);
        return createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, outAllocation, data);
    }

    VkBuffer MemoryAllocator::createStagingBuffer(VkDeviceSize size, VmaAllocation& outAllocation, const void* data) {
        printf("[VMA] Creating staging buffer: size=%llu bytes\n", (unsigned long long)size);
        return createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, outAllocation, data);
    }

    void MemoryAllocator::destroyBuffer(VkBuffer buffer, VmaAllocation allocation) {
        if (buffer != VK_NULL_HANDLE && m_allocator != VK_NULL_HANDLE) {
            printf("[VMA] Destroying buffer\n");
            vmaDestroyBuffer(m_allocator, buffer, allocation);
        }
    }

}