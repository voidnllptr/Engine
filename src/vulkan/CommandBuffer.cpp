#include "CommandBuffer.hpp"
#include "Device.hpp"
#include <stdexcept>

namespace Vulkan {

    CommandBuffer::~CommandBuffer() {
        cleanup();
    }

    CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
        : m_commandPool(other.m_commandPool)
        , m_commandBuffers(std::move(other.m_commandBuffers))
        , m_device(other.m_device) {
        other.m_commandPool = VK_NULL_HANDLE;
        other.m_device = nullptr;
    }

    CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept {
        if (this != &other) {
            cleanup();
            m_commandPool = other.m_commandPool;
            m_commandBuffers = std::move(other.m_commandBuffers);
            m_device = other.m_device;
            other.m_commandPool = VK_NULL_HANDLE;
            other.m_device = nullptr;
        }
        return *this;
    }

    void CommandBuffer::create(Device& device, uint32_t imageCount) {
        cleanup();
        m_device = &device;
        createCommandPool(device);
        createCommandBuffers(device, imageCount);
    }

    void CommandBuffer::cleanup() {
        if (m_device != nullptr) {
            destroy();
        }
        m_device = nullptr;
    }

    void CommandBuffer::begin(uint32_t index, bool isSingleUse) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (isSingleUse) {
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        }

        vkBeginCommandBuffer(m_commandBuffers[index], &beginInfo);
    }

    void CommandBuffer::end(uint32_t index) {
        vkEndCommandBuffer(m_commandBuffers[index]);
    }

    void CommandBuffer::beginRenderPass(uint32_t index, VkRenderPass renderPass,
        VkFramebuffer framebuffer, VkExtent2D extent,
        VkClearValue clearColor, VkClearValue clearDepth) {

        VkClearValue clearValues[2] = { clearColor, clearDepth };

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = framebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(m_commandBuffers[index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void CommandBuffer::endRenderPass(uint32_t index) {
        vkCmdEndRenderPass(m_commandBuffers[index]);
    }

    void CommandBuffer::reset(uint32_t index) {
        vkResetCommandBuffer(m_commandBuffers[index], 0);
    }

    void CommandBuffer::resetAll() {
        for (uint32_t i = 0; i < m_commandBuffers.size(); ++i) {
            reset(i);
        }
    }

    void CommandBuffer::waitForFence(VkFence fence) {
        vkWaitForFences(m_device->get(), 1, &fence, VK_TRUE, UINT64_MAX);
    }

    void CommandBuffer::createCommandPool(Device& device) {
        auto queueFamilies = device.getQueueFamilies();

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilies.graphicsFamily.value();

        if (vkCreateCommandPool(device.get(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool!");
        }
    }

    void CommandBuffer::createCommandBuffers(Device& device, uint32_t imageCount) {
        m_commandBuffers.resize(imageCount);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = imageCount;

        if (vkAllocateCommandBuffers(device.get(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers!");
        }
    }

    void CommandBuffer::destroy() {
        if (m_commandPool != VK_NULL_HANDLE && m_device != nullptr) {
            vkDestroyCommandPool(m_device->get(), m_commandPool, nullptr);
            m_commandPool = VK_NULL_HANDLE;
        }
        m_commandBuffers.clear();
    }

}