#include "Framebuffer.hpp"

namespace Vulkan {

    Framebuffer::~Framebuffer() {
        cleanup();
    }

    Framebuffer::Framebuffer(Framebuffer&& other) noexcept
        : m_framebuffer(other.m_framebuffer)
        , m_device(other.m_device) {
        other.m_framebuffer = VK_NULL_HANDLE;
        other.m_device = VK_NULL_HANDLE;
    }

    Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept {
        if (this != &other) {
            cleanup();
            m_framebuffer = other.m_framebuffer;
            m_device = other.m_device;
            other.m_framebuffer = VK_NULL_HANDLE;
            other.m_device = VK_NULL_HANDLE;
        }
        return *this;
    }

    void Framebuffer::create(VkDevice device, VkRenderPass renderPass,
        VkImageView imageView, VkExtent2D extent) {
        create(device, renderPass, std::vector<VkImageView>{imageView}, extent);
    }

    void Framebuffer::create(VkDevice device, VkRenderPass renderPass,
        const std::vector<VkImageView>& attachments,
        VkExtent2D extent) {
        cleanup();
        m_device = device;

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_framebuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }

    void Framebuffer::cleanup() {
        if (m_device != VK_NULL_HANDLE && m_framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
            m_framebuffer = VK_NULL_HANDLE;
        }
        m_device = VK_NULL_HANDLE;
    }

    void FramebufferCache::create(VkDevice device, VkRenderPass renderPass,
        const std::vector<VkImageView>& imageViews,
        VkExtent2D extent) {
        cleanup();
        m_framebuffers.resize(imageViews.size());

        for (size_t i = 0; i < imageViews.size(); i++) {
            m_framebuffers[i].create(device, renderPass, imageViews[i], extent);
        }
    }

    void FramebufferCache::createWithDepth(VkDevice device, VkRenderPass renderPass,
        const std::vector<VkImageView>& colorImageViews,
        VkImageView depthImageView,
        VkExtent2D extent) {
        cleanup();
        m_framebuffers.resize(colorImageViews.size());

        for (size_t i = 0; i < colorImageViews.size(); i++) {
            std::vector<VkImageView> attachments = { colorImageViews[i], depthImageView };
            m_framebuffers[i].create(device, renderPass, attachments, extent);
        }
    }

    void FramebufferCache::cleanup() {
        for (auto& framebuffer : m_framebuffers) {
            framebuffer.cleanup();
        }
        m_framebuffers.clear();
    }

}