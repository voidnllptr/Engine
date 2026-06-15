#include "ImGuiLayer.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include <stdexcept>
#include <iostream>

namespace UI {

    ImGuiLayer::~ImGuiLayer() {
        shutdown();
    }

    void ImGuiLayer::init(SDL_Window* window, VkInstance instance,
        VkPhysicalDevice physicalDevice, VkDevice device,
        VkQueue graphicsQueue, uint32_t queueFamily,
        VkRenderPass renderPass, uint32_t minImageCount, uint32_t imageCount,
        VkCommandPool commandPool) {

        if (m_initialized) {
            std::cout << "[ImGui] Already initialized, shutting down first" << std::endl;
            shutdown();
        }

        m_device = device;
        m_commandPool = commandPool;
        m_graphicsQueue = graphicsQueue;
        m_renderPass = renderPass;
        m_queueFamily = queueFamily;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        int windowWidth, windowHeight;
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);

        float dpiScale = 1.0f;
#ifdef _WIN32
        float displayScale = SDL_GetDisplayContentScale(SDL_GetDisplayForWindow(window));
        if (displayScale > 0) {
            dpiScale = displayScale;
        }
#endif
        ImGuiStyle& style = ImGui::GetStyle();

        style.FramePadding = ImVec2(8.0f * dpiScale, 6.0f * dpiScale);
        style.ItemSpacing = ImVec2(8.0f * dpiScale, 8.0f * dpiScale);
        style.ItemInnerSpacing = ImVec2(6.0f * dpiScale, 6.0f * dpiScale);
        style.TouchExtraPadding = ImVec2(2.0f * dpiScale, 2.0f * dpiScale);
        style.IndentSpacing = 21.0f * dpiScale;
        style.ScrollbarSize = 14.0f * dpiScale;
        style.GrabMinSize = 10.0f * dpiScale;
        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.TabRounding = 4.0f;
        style.ScrollbarRounding = 9.0f;
        style.GrabRounding = 4.0f;
        style.FrameRounding = 4.0f;
        style.WindowRounding = 6.0f;
        style.ChildRounding = 4.0f;
        style.PopupRounding = 4.0f;

        style.ScaleAllSizes(dpiScale);

        float fontSize = 16.0f * dpiScale;
        io.Fonts->AddFontDefault();
        io.FontGlobalScale = dpiScale;

        std::cout << "[ImGui] DPI Scale: " << dpiScale << ", Window Size: " << windowWidth << "x" << windowHeight << std::endl;

        ImGui::StyleColorsDark();

        style.Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.5f, 0.9f, 0.6f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.6f, 1.0f, 0.8f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1f, 0.4f, 0.8f, 1.0f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.8f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);

        if (!ImGui_ImplSDL3_InitForVulkan(window)) {
            throw std::runtime_error("[ImGui] Failed to initialize SDL3 backend");
        }

        createDescriptorPool(device, imageCount);

        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.ApiVersion = VK_API_VERSION_1_0;
        initInfo.Instance = instance;
        initInfo.PhysicalDevice = physicalDevice;
        initInfo.Device = device;
        initInfo.QueueFamily = queueFamily;
        initInfo.Queue = graphicsQueue;
        initInfo.DescriptorPool = m_descriptorPool;
        initInfo.MinImageCount = minImageCount;
        initInfo.ImageCount = imageCount;
        initInfo.Allocator = nullptr;
        initInfo.CheckVkResultFn = nullptr;

        initInfo.PipelineInfoMain.RenderPass = renderPass;
        initInfo.PipelineInfoMain.Subpass = 0;
        initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        initInfo.UseDynamicRendering = false;

        if (!ImGui_ImplVulkan_Init(&initInfo)) {
            throw std::runtime_error("[ImGui] Failed to initialize Vulkan backend");
        }

        createFontTexture(commandPool, graphicsQueue, queueFamily);

        m_initialized = true;
        std::cout << "[ImGui] Initialization complete!" << std::endl;
    }

    void ImGuiLayer::createDescriptorPool(VkDevice device, uint32_t imageCount) {
        VkDescriptorPoolSize poolSizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 }
        };

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 100;
        poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
        poolInfo.pPoolSizes = poolSizes;

        VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("[ImGui] Failed to create descriptor pool!");
        }
    }

    void ImGuiLayer::createFontTexture(VkCommandPool commandPool, VkQueue graphicsQueue, uint32_t queueFamily) {
        VkCommandPool tempCommandPool = VK_NULL_HANDLE;

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamily;

        VkResult result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &tempCommandPool);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("[ImGui] Failed to create temporary command pool for font upload");
        }

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = tempCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        if (vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
            vkDestroyCommandPool(m_device, tempCommandPool, nullptr);
            throw std::runtime_error("[ImGui] Failed to allocate command buffer for font upload");
        }

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            vkDestroyCommandPool(m_device, tempCommandPool, nullptr);
            throw std::runtime_error("[ImGui] Failed to begin command buffer for font upload");
        }

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            vkDestroyCommandPool(m_device, tempCommandPool, nullptr);
            throw std::runtime_error("[ImGui] Failed to end command buffer for font upload");
        }

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        if (result != VK_SUCCESS) {
            vkDestroyCommandPool(m_device, tempCommandPool, nullptr);
            throw std::runtime_error("[ImGui] Failed to submit font upload command");
        }

        vkQueueWaitIdle(graphicsQueue);
        vkFreeCommandBuffers(m_device, tempCommandPool, 1, &commandBuffer);
        vkDestroyCommandPool(m_device, tempCommandPool, nullptr);
    }

    void ImGuiLayer::beginFrame() {
        if (!m_initialized) return;

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::endFrame(VkCommandBuffer commandBuffer) {
        if (!m_initialized) return;

        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData && drawData->CmdListsCount > 0) {
            ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer, VK_NULL_HANDLE);
        }
    }

    void ImGuiLayer::shutdown() {
        if (!m_initialized) return;

        if (m_device != VK_NULL_HANDLE) {
            vkDeviceWaitIdle(m_device);
        }

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        if (m_descriptorPool != VK_NULL_HANDLE && m_device != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
            m_descriptorPool = VK_NULL_HANDLE;
        }

        m_device = VK_NULL_HANDLE;
        m_commandPool = VK_NULL_HANDLE;
        m_graphicsQueue = VK_NULL_HANDLE;
        m_renderPass = VK_NULL_HANDLE;
        m_initialized = false;

        std::cout << "[ImGui] Shutdown complete!" << std::endl;
    }

}