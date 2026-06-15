#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace Vulkan {

    class Device;

    class Swapchain {
    public:

        Swapchain() = default;
        ~Swapchain();

        Swapchain(const Swapchain&) = delete;
        Swapchain& operator=(const Swapchain&) = delete;

        Swapchain(Swapchain&& other) noexcept;
        Swapchain& operator=(Swapchain&& other) noexcept;

        void init(Device& device, VkSurfaceKHR surface, int windowWidth, int windowHeight);
        void cleanup();

        VkSwapchainKHR get() const { return m_swapchain; }
        VkFormat getImageFormat() const { return m_imageFormat; }
        VkColorSpaceKHR getColorSpace() const { return m_colorSpace; }
        VkExtent2D getExtent() const { return m_extent; }
        const std::vector<VkImage>& getImages() const { return m_images; }
        const std::vector<VkImageView>& getImageViews() const { return m_imageViews; }
        uint32_t getImageCount() const { return static_cast<uint32_t>(m_images.size()); }

        uint32_t acquireNextImage(VkSemaphore semaphore, VkFence fence = VK_NULL_HANDLE);

        VkResult present(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore);

        void recreate(Device& device, VkSurfaceKHR surface, int windowWidth, int windowHeight);

        bool needsRecreation(VkSurfaceKHR surface) const;

    private:
        void createSwapchain(Device& device, VkSurfaceKHR surface, int windowWidth, int windowHeight);
        void createImageViews(Device& device);
        void destroy(Device& device);

        VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availableModes);
        VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, int windowWidth, int windowHeight);

        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
        VkFormat m_imageFormat = VK_FORMAT_UNDEFINED;
        VkColorSpaceKHR m_colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        VkExtent2D m_extent = { 0, 0 };

        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_imageViews;

        Device* m_device = nullptr;
    };

}