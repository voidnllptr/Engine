#include "Buffer.hpp"
#include <cstring>
#include <iostream>

namespace Vulkan {

    Buffer::Buffer(Buffer&& other) noexcept
        : m_buffer(other.m_buffer)
        , m_allocation(other.m_allocation)
        , m_size(other.m_size)
        , m_allocator(other.m_allocator) {
        other.m_buffer = VK_NULL_HANDLE;
        other.m_allocation = VK_NULL_HANDLE;
        other.m_size = 0;
        other.m_allocator = nullptr;
    }

    Buffer& Buffer::operator=(Buffer&& other) noexcept {
        if (this != &other) {
            cleanup();
            m_buffer = other.m_buffer;
            m_allocation = other.m_allocation;
            m_size = other.m_size;
            m_allocator = other.m_allocator;
            other.m_buffer = VK_NULL_HANDLE;
            other.m_allocation = VK_NULL_HANDLE;
            other.m_size = 0;
            other.m_allocator = nullptr;
        }
        return *this;
    }

    void Buffer::create(MemoryAllocator& allocator, VkDeviceSize size, VkBufferUsageFlags usage, const void* data) {
        cleanup();
        m_allocator = &allocator;
        m_size = size;

        VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        if (usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) {
            memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
        }

        m_buffer = allocator.createBuffer(size, usage, memoryUsage, m_allocation, data);
    }

    void Buffer::createVertex(MemoryAllocator& allocator, const void* data, VkDeviceSize size) {
        std::cout << "[BUFFER] Creating vertex buffer: size=" << size << " bytes" << std::endl;
        create(allocator, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, data);
    }

    void Buffer::createIndex(MemoryAllocator& allocator, const void* data, VkDeviceSize size) {
        create(allocator, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, data);
    }

    void Buffer::createUniform(MemoryAllocator& allocator, VkDeviceSize size) {
        create(allocator, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, nullptr);
    }

    void Buffer::createStaging(MemoryAllocator& allocator, const void* data, VkDeviceSize size) {
        create(allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, data);
    }

    void Buffer::cleanup() {
        if (m_allocator != nullptr && m_buffer != VK_NULL_HANDLE) {
            m_allocator->destroyBuffer(m_buffer, m_allocation);
            m_buffer = VK_NULL_HANDLE;
            m_allocation = VK_NULL_HANDLE;
        }
        m_allocator = nullptr;
        m_size = 0;
    }

    void Buffer::upload(const void* data, VkDeviceSize size, VkDeviceSize offset) {
        if (m_allocator != nullptr && m_allocation != VK_NULL_HANDLE) {
            void* mappedData;
            vmaMapMemory(m_allocator->get(), m_allocation, &mappedData);
            memcpy(static_cast<char*>(mappedData) + offset, data, static_cast<size_t>(size));
            vmaUnmapMemory(m_allocator->get(), m_allocation);
        }
    }

    void Buffer::bindVertex(VkCommandBuffer commandBuffer, VkDeviceSize offset) const {
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_buffer, &offset);
    }

    void Buffer::bindIndex(VkCommandBuffer commandBuffer, VkDeviceSize offset) const {
        vkCmdBindIndexBuffer(commandBuffer, m_buffer, offset, VK_INDEX_TYPE_UINT32);
    }

}