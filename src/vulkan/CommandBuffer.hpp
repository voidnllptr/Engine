#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace Vulkan {

    class Device;

    class CommandBuffer {
    public:
        CommandBuffer() = default;
        ~CommandBuffer();

        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer& operator=(const CommandBuffer&) = delete;

        CommandBuffer(CommandBuffer&& other) noexcept;
        CommandBuffer& operator=(CommandBuffer&& other) noexcept;

        void create(Device& device, uint32_t imageCount);
        void cleanup();

        void begin(uint32_t index, bool isSingleUse = false);
        void end(uint32_t index);
        void beginRenderPass(uint32_t index, VkRenderPass renderPass,
            VkFramebuffer framebuffer, VkExtent2D extent,
            VkClearValue clearColor, VkClearValue clearDepth);
        void endRenderPass(uint32_t index);

        VkCommandBuffer get(uint32_t index) const { return m_commandBuffers[index]; }
        VkCommandBuffer getCommandBuffer(uint32_t index) const { return m_commandBuffers[index]; }
        VkCommandPool getCommandPool() const { return m_commandPool; }
        uint32_t getCount() const { return static_cast<uint32_t>(m_commandBuffers.size()); }
        bool isValid() const { return m_commandPool != VK_NULL_HANDLE; }

        void reset(uint32_t index);
        void resetAll();
        void waitForFence(VkFence fence);

    private:
        void createCommandPool(Device& device);
        void createCommandBuffers(Device& device, uint32_t imageCount);
        void destroy();

        VkCommandPool m_commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> m_commandBuffers;
        Device* m_device = nullptr;
    };

}