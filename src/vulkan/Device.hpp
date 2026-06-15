#pragma once
#include <vulkan/vulkan.h>
#include <optional>
#include <vector>
#include <string>

namespace Vulkan {

	struct QueueFamilyIndices {

		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentationFamily;
		std::optional<uint32_t> computeFamily;
		std::optional<uint32_t> transferFamily;

		bool isComplete() const;
	};

	struct SwapChainSupportDetails {

		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class Device {

		VkInstance m_instance;
		VkSurfaceKHR m_surface;
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;
		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;
		QueueFamilyIndices m_queueIndices;

		bool checkSwapChainSupport(VkPhysicalDevice device) const;
		bool isDeviceSuitable(VkPhysicalDevice device);
		VkPhysicalDevice selectBestDevice(const std::vector<VkPhysicalDevice>& devices);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		void createLogicalDevice();

	public:
		Device() = default;
		~Device();

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;

		Device(Device&& other) noexcept;
		Device& operator=(Device&& other) noexcept;

		void init(VkInstance instance, VkSurfaceKHR surface);

		VkDevice get() const;
		VkPhysicalDevice getPhysicalDevice() const;
		VkQueue getGraphicsQueue() const;
		VkQueue getPresentQueue() const;
		const QueueFamilyIndices& getQueueFamilies() const;
		SwapChainSupportDetails getSwapChainSupportDetails() const;

		VkSurfaceFormatKHR findSuitableSurfaceFormat(
			const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR findSuitablePresentMode(
			const std::vector<VkPresentModeKHR>& availableModes);
		VkExtent2D findSuitableExtent(
			const VkSurfaceCapabilitiesKHR& capabilities,
			int windowWidth, int windowHeight);
		void waitIdle() const;

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
		void cleanup();
	};
}