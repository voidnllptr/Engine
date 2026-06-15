#pragma once

#include <vulkan/vulkan.h>
#include "Device.hpp"
#include <stdexcept>
#include <vector>

namespace Vulkan {

    class RenderPass {
    public:
        RenderPass() = default;
        ~RenderPass();

        RenderPass(const RenderPass&) = delete;
        RenderPass& operator=(const RenderPass&) = delete;

        RenderPass(RenderPass&& other) noexcept;
        RenderPass& operator=(RenderPass&& other) noexcept;

        void create(VkDevice device, VkFormat swapchainImageFormat);
        void cleanup();

        VkRenderPass get() const { return m_renderPass; }
        bool isValid() const { return m_renderPass != VK_NULL_HANDLE; }

    private:
        VkRenderPass m_renderPass = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
    };

}