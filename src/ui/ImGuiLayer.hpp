#pragma once

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>

namespace UI {

    class ImGuiLayer {
    public:
        ImGuiLayer() = default;
        ~ImGuiLayer();

        ImGuiLayer(const ImGuiLayer&) = delete;
        ImGuiLayer& operator=(const ImGuiLayer&) = delete;

        void init(SDL_Window* window, VkInstance instance, VkPhysicalDevice physicalDevice,
            VkDevice device, VkQueue graphicsQueue, uint32_t queueFamily,
            VkRenderPass renderPass, uint32_t minImageCount, uint32_t imageCount,
            VkCommandPool commandPool);

        void shutdown();

        void beginFrame();
        void endFrame(VkCommandBuffer commandBuffer);

        VkDescriptorPool getDescriptorPool() const { return m_descriptorPool; }
        bool isInitialized() const { return m_initialized; }

    private:
        void createDescriptorPool(VkDevice device, uint32_t imageCount);
        void createFontTexture(VkCommandPool commandPool, VkQueue graphicsQueue, uint32_t queueFamily);

        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        VkCommandPool m_commandPool = VK_NULL_HANDLE;
        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkRenderPass m_renderPass = VK_NULL_HANDLE;
        uint32_t m_queueFamily = 0;
        bool m_initialized = false;
    };

}