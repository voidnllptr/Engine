#include "Device.hpp"
#include <set>
#include <stdexcept>
#include <algorithm>
#include <iostream>

bool Vulkan::QueueFamilyIndices::isComplete() const
{
	return graphicsFamily.has_value() && presentationFamily.has_value();
}

bool Vulkan::Device::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);
    if (!indices.isComplete()) {
        return false;
    }

    bool extensionsSupported = checkDeviceExtensionSupport(device);
    if (!extensionsSupported) {
        return false;
    }

    if (!checkSwapChainSupport(device)) {
        return false; 
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

 
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

    std::cout << "Device: " << properties.deviceName << " is suitable" << std::endl;
    std::cout << "  VRAM: " << memoryProperties.memoryHeaps[0].size / (1024 * 1024 * 1024) << " GB" << std::endl;

    return true;
}

VkPhysicalDevice Vulkan::Device::selectBestDevice(const std::vector<VkPhysicalDevice>& devices) {

    struct DeviceCandidate {
        VkPhysicalDevice device;
        uint32_t score;
        QueueFamilyIndices indices;
    };

    std::vector<DeviceCandidate> candidates;

    for (const auto& device : devices) {
        if (!isDeviceSuitable(device)) {
            continue;
        }

        uint32_t score = 0;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceMemoryProperties memoryProperties;

        vkGetPhysicalDeviceProperties(device, &properties);
        vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

        switch (properties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            score += 10000;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            score += 5000;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            score += 2000;
            break;
        default:
            score += 1000;
        }

        for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; ++i) {
            if (memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                VkDeviceSize vramGB = memoryProperties.memoryHeaps[i].size / (1024 * 1024 * 1024);
                score += static_cast<uint32_t>(vramGB);
                break;
            }
        }

        candidates.push_back({ device, score, findQueueFamilies(device) });

        std::cout << "  " << properties.deviceName
            << " - Score: " << score
            << " (Type: " << (int)properties.deviceType << ")" << std::endl;
    }

    if (candidates.empty()) {
        return VK_NULL_HANDLE;
    }

    std::sort(candidates.begin(), candidates.end(), [](const DeviceCandidate& a, DeviceCandidate& b) { return a.score > b.score; });

    m_queueIndices = candidates[0].indices;

    VkPhysicalDeviceProperties selectedProps;
    vkGetPhysicalDeviceProperties(candidates[0].device, &selectedProps);
    std::cout << "\n✓ Selected device: " << selectedProps.deviceName << std::endl;

    return candidates[0].device;
}

Vulkan::QueueFamilyIndices Vulkan::Device::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        const auto& queueFamily = queueFamilies[i];

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        if ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
            !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            indices.computeFamily = i;
        }

        if ((queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
            !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
            !(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            indices.transferFamily = i;
        }

        if (m_surface != VK_NULL_HANDLE) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
            if (presentSupport) {
                indices.presentationFamily = i;
            }
        }
    }

    if (!indices.transferFamily.has_value() && indices.graphicsFamily.has_value()) {
        indices.transferFamily = indices.graphicsFamily;
    }

    if (!indices.computeFamily.has_value() && indices.graphicsFamily.has_value()) {
        indices.computeFamily = indices.graphicsFamily;
    }

    return indices;
}

bool Vulkan::Device::checkSwapChainSupport(VkPhysicalDevice device) const
{
    if (m_surface == VK_NULL_HANDLE) {
        return false;
    }

    uint32_t formatCount = 0;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        device,
        m_surface,
        &formatCount,
        nullptr
    );

    if (result != VK_SUCCESS || formatCount == 0) {
        return false;
    }

    uint32_t presentModeCount = 0;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        device,
        m_surface,
        &presentModeCount,
        nullptr
    );

    if (result != VK_SUCCESS || presentModeCount == 0) {
        return false;
    }

    VkSurfaceCapabilitiesKHR capabilities;
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device,
        m_surface,
        &capabilities
    );

    if (result != VK_SUCCESS) {
        return false;
    }

    if (capabilities.maxImageExtent.width == 0 ||
        capabilities.maxImageExtent.height == 0) {
        return false;
    }

    return true;
}

bool Vulkan::Device::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    std::vector<const char*> requiredExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    for (const auto& required : requiredExtensions) {
        bool found = false;
        for (const auto& available : availableExtensions) {
            if (strcmp(required, available.extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }

    return true;
}

void Vulkan::Device::createLogicalDevice()
{
    if (m_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Cannot create logical device: no physical device selected");
    }

    std::set<uint32_t> uniqueQueueFamilies;
    if (m_queueIndices.graphicsFamily.has_value()) {
        uniqueQueueFamilies.insert(m_queueIndices.graphicsFamily.value());
    }
    if (m_queueIndices.presentationFamily.has_value()) {
        uniqueQueueFamilies.insert(m_queueIndices.presentationFamily.value());
    }
    if (m_queueIndices.computeFamily.has_value()) {
        uniqueQueueFamilies.insert(m_queueIndices.computeFamily.value());
    }
    if (m_queueIndices.transferFamily.has_value()) {
        uniqueQueueFamilies.insert(m_queueIndices.transferFamily.value());
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    createInfo.enabledLayerCount = 0;

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }

    if (m_queueIndices.graphicsFamily.has_value()) {
        vkGetDeviceQueue(m_device, m_queueIndices.graphicsFamily.value(), 0, &m_graphicsQueue);
    }

    if (m_queueIndices.presentationFamily.has_value()) {
        vkGetDeviceQueue(m_device, m_queueIndices.presentationFamily.value(), 0, &m_presentQueue);
    }
}

Vulkan::Device::~Device() {

    cleanup();
}

void Vulkan::Device::cleanup() {
    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }
}

Vulkan::Device::Device(Device&& other) noexcept
    : m_instance(other.m_instance)
    , m_surface(other.m_surface)
    , m_physicalDevice(other.m_physicalDevice)
    , m_device(other.m_device)
    , m_graphicsQueue(other.m_graphicsQueue)
    , m_presentQueue(other.m_presentQueue)
    , m_queueIndices(other.m_queueIndices) {

    other.m_instance = VK_NULL_HANDLE;
    other.m_surface = VK_NULL_HANDLE;
    other.m_physicalDevice = VK_NULL_HANDLE;
    other.m_device = VK_NULL_HANDLE;
    other.m_graphicsQueue = VK_NULL_HANDLE;
    other.m_presentQueue = VK_NULL_HANDLE;
}

Vulkan::Device& Vulkan::Device::operator=(Device&& other) noexcept {
    if (this != &other) {
        if (m_device != VK_NULL_HANDLE) {
            vkDestroyDevice(m_device, nullptr);
        }

        m_instance = other.m_instance;
        m_surface = other.m_surface;
        m_physicalDevice = other.m_physicalDevice;
        m_device = other.m_device;
        m_graphicsQueue = other.m_graphicsQueue;
        m_presentQueue = other.m_presentQueue;
        m_queueIndices = other.m_queueIndices;

        other.m_instance = VK_NULL_HANDLE;
        other.m_surface = VK_NULL_HANDLE;
        other.m_physicalDevice = VK_NULL_HANDLE;
        other.m_device = VK_NULL_HANDLE;
        other.m_graphicsQueue = VK_NULL_HANDLE;
        other.m_presentQueue = VK_NULL_HANDLE;
    }
    return *this;
}

void Vulkan::Device::init(VkInstance instance, VkSurfaceKHR surface)
{
    m_instance = instance;
    m_surface = surface;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
    
    m_physicalDevice = selectBestDevice(devices);

    if (m_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }

    m_queueIndices = findQueueFamilies(m_physicalDevice);

    createLogicalDevice();
}

VkDevice Vulkan::Device::get() const
{
	return m_device;
}

VkPhysicalDevice Vulkan::Device::getPhysicalDevice() const
{
	return m_physicalDevice;
}

VkQueue Vulkan::Device::getGraphicsQueue() const
{
	return m_graphicsQueue;
}

VkQueue Vulkan::Device::getPresentQueue() const
{
	return m_presentQueue;
}

const Vulkan::QueueFamilyIndices& Vulkan::Device::getQueueFamilies() const
{
    return m_queueIndices;
}

Vulkan::SwapChainSupportDetails Vulkan::Device::getSwapChainSupportDetails() const
{
        SwapChainSupportDetails details;

        if (m_physicalDevice == VK_NULL_HANDLE || m_surface == VK_NULL_HANDLE) {
            return details;
        }

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &details.capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }


VkSurfaceFormatKHR Vulkan::Device::findSuitableSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    if (availableFormats.size() == 1 &&
        availableFormats[0].format == VK_FORMAT_UNDEFINED) {
        return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR Vulkan::Device::findSuitablePresentMode(
    const std::vector<VkPresentModeKHR>& availableModes)
{
    for (const auto& availableMode : availableModes) {
        if (availableMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availableMode;
        }
    }

    for (const auto& availableMode : availableModes) {
        if (availableMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            return availableMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Vulkan::Device::findSuitableExtent(
    const VkSurfaceCapabilitiesKHR & capabilities,
    int windowWidth, int windowHeight)
{
    if (capabilities.currentExtent.width != UINT32_MAX) {
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

void Vulkan::Device::waitIdle() const {
    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);
    }
}

uint32_t Vulkan::Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}
