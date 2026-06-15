#include "Swapchain.hpp"
#include "Device.hpp"
#include <algorithm>
#include <stdexcept>
#include <iostream>

namespace Vulkan {

    Swapchain::~Swapchain() {
        cleanup();
    }

    Swapchain::Swapchain(Swapchain&& other) noexcept
        : m_swapchain(other.m_swapchain)
        , m_imageFormat(other.m_imageFormat)
        , m_colorSpace(other.m_colorSpace)
        , m_extent(other.m_extent)
        , m_images(std::move(other.m_images))
        , m_imageViews(std::move(other.m_imageViews))
        , m_device(other.m_device) {

        other.m_swapchain = VK_NULL_HANDLE;
        other.m_device = nullptr;
    }

    Swapchain& Swapchain::operator=(Swapchain&& other) noexcept {
        if (this != &other) {
            cleanup();

            m_swapchain = other.m_swapchain;
            m_imageFormat = other.m_imageFormat;
            m_colorSpace = other.m_colorSpace;
            m_extent = other.m_extent;
            m_images = std::move(other.m_images);
            m_imageViews = std::move(other.m_imageViews);
            m_device = other.m_device;

            other.m_swapchain = VK_NULL_HANDLE;
            other.m_device = nullptr;
        }
        return *this;
    }

    void Swapchain::init(Device& device, VkSurfaceKHR surface, int windowWidth, int windowHeight) {
        m_device = &device;
        createSwapchain(device, surface, windowWidth, windowHeight);
        createImageViews(device);
    }

    void Swapchain::cleanup() {
        if (m_device != nullptr && m_device->get() != VK_NULL_HANDLE) {
            for (auto imageView : m_imageViews) {
                if (imageView != VK_NULL_HANDLE) {
                    vkDestroyImageView(m_device->get(), imageView, nullptr);
                }
            }
            m_imageViews.clear();

            if (m_swapchain != VK_NULL_HANDLE) {
                vkDestroySwapchainKHR(m_device->get(), m_swapchain, nullptr);
                m_swapchain = VK_NULL_HANDLE;
            }

            m_images.clear();
        }
        m_device = nullptr;
    }

    VkSurfaceFormatKHR Swapchain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
            return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
        }

        for (const auto& format : availableFormats) {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }

        for (const auto& format : availableFormats) {
            if (format.format == VK_FORMAT_R8G8B8A8_UNORM &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR Swapchain::choosePresentMode(const std::vector<VkPresentModeKHR>& availableModes) {
        for (const auto& mode : availableModes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return mode;
            }
        }

        for (const auto& mode : availableModes) {
            if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                return mode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D Swapchain::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, int windowWidth, int windowHeight) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(windowWidth),
            static_cast<uint32_t>(windowHeight)
        };

        actualExtent.width = std::clamp(actualExtent.width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height);

        return actualExtent;
    }

    uint32_t Swapchain::acquireNextImage(VkSemaphore semaphore, VkFence fence) {
        if (m_device == nullptr || m_swapchain == VK_NULL_HANDLE) {
            throw std::runtime_error("Swapchain not initialized!");
        }

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(
            m_device->get(),
            m_swapchain,
            UINT64_MAX,
            semaphore,
            fence,
            &imageIndex
        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            return static_cast<uint32_t>(-1);
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swapchain image!");
        }

        return imageIndex;
    }

    VkResult Swapchain::present(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore) {
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &waitSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapchain;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        return vkQueuePresentKHR(queue, &presentInfo);
    }

    void Swapchain::recreate(Device& device, VkSurfaceKHR surface, int windowWidth, int windowHeight) {
        vkDeviceWaitIdle(device.get());

        VkSwapchainKHR oldSwapchain = m_swapchain;

        for (auto& imageView : m_imageViews) {
            if (imageView != VK_NULL_HANDLE) {
                vkDestroyImageView(device.get(), imageView, nullptr);
            }
        }
        m_imageViews.clear();
        m_images.clear();

        createSwapchain(device, surface, windowWidth, windowHeight);
        createImageViews(device);

        if (oldSwapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device.get(), oldSwapchain, nullptr);
        }

        m_device = &device;
    }

    bool Swapchain::needsRecreation(VkSurfaceKHR surface) const {
        if (m_device == nullptr || m_device->get() == VK_NULL_HANDLE || m_swapchain == VK_NULL_HANDLE) {
            return true;
        }

        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->getPhysicalDevice(), surface, &capabilities);

        if (capabilities.currentExtent.width != m_extent.width ||
            capabilities.currentExtent.height != m_extent.height) {
            return true;
        }

        return false;
    }

    void Swapchain::createSwapchain(Device& device, VkSurfaceKHR surface, int windowWidth, int windowHeight) {
        auto swapChainSupport = device.getSwapChainSupportDetails();

        if (swapChainSupport.formats.empty()) {
            throw std::runtime_error("No surface formats available!");
        }
        if (swapChainSupport.presentModes.empty()) {
            throw std::runtime_error("No present modes available!");
        }

        VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapChainSupport.formats);
        m_imageFormat = surfaceFormat.format;
        m_colorSpace = surfaceFormat.colorSpace;

        VkPresentModeKHR presentMode = choosePresentMode(swapChainSupport.presentModes);

        m_extent = chooseExtent(swapChainSupport.capabilities, windowWidth, windowHeight);

        if (m_extent.width == 0 || m_extent.height == 0) {
            throw std::runtime_error("Invalid extent for swapchain!");
        }

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = m_imageFormat;
        createInfo.imageColorSpace = m_colorSpace;
        createInfo.imageExtent = m_extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        auto indices = device.getQueueFamilies();
        if (indices.graphicsFamily != indices.presentationFamily) {
            uint32_t queueFamilyIndices[] = {
                indices.graphicsFamily.value(),
                indices.presentationFamily.value()
            };
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = m_swapchain;

        VkResult result = vkCreateSwapchainKHR(device.get(), &createInfo, nullptr, &m_swapchain);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            throw std::runtime_error("Swapchain out of date during creation!");
        }
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swapchain!");
        }

        vkGetSwapchainImagesKHR(device.get(), m_swapchain, &imageCount, nullptr);
        m_images.resize(imageCount);
        vkGetSwapchainImagesKHR(device.get(), m_swapchain, &imageCount, m_images.data());
    }

    void Swapchain::createImageViews(Device& device) {
        m_imageViews.resize(m_images.size());

        for (size_t i = 0; i < m_images.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_imageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            VkResult result = vkCreateImageView(device.get(), &createInfo, nullptr, &m_imageViews[i]);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Failed to create image view " + std::to_string(i));
            }
        }
    }
}