#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>

namespace Vulkan {

    class Framebuffer {
    public:
        Framebuffer() = default;
        ~Framebuffer();

        Framebuffer(const Framebuffer&) = delete;
        Framebuffer& operator=(const Framebuffer&) = delete;

        Framebuffer(Framebuffer&& other) noexcept;
        Framebuffer& operator=(Framebuffer&& other) noexcept;

        void create(VkDevice device, VkRenderPass renderPass,
            VkImageView imageView, VkExtent2D extent);

        void create(VkDevice device, VkRenderPass renderPass,
            const std::vector<VkImageView>& attachments,
            VkExtent2D extent);

        void cleanup();

        VkFramebuffer get() const { return m_framebuffer; }
        bool isValid() const { return m_framebuffer != VK_NULL_HANDLE; }

    private:
        VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
    };

    class FramebufferCache {
    public:
        FramebufferCache() = default;
        ~FramebufferCache() { cleanup(); }

        void create(VkDevice device, VkRenderPass renderPass,
            const std::vector<VkImageView>& imageViews,
            VkExtent2D extent);

        void createWithDepth(VkDevice device, VkRenderPass renderPass,
            const std::vector<VkImageView>& colorImageViews,
            VkImageView depthImageView,
            VkExtent2D extent);

        void cleanup();

        VkFramebuffer get(uint32_t index) const { return m_framebuffers[index].get(); }
        size_t size() const { return m_framebuffers.size(); }

    private:
        std::vector<Framebuffer> m_framebuffers;
    };

}