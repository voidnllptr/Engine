#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <array>
#include "../vulkan/Device.hpp"
#include "../vulkan/Swapchain.hpp"
#include "../vulkan/RenderPass.hpp"
#include "../vulkan/Pipeline.hpp"
#include "../vulkan/CommandBuffer.hpp"
#include "../vulkan/Sync.hpp"
#include "../vulkan/Descriptors.hpp"
#include "../vulkan/Buffer.hpp"
#include "../vulkan/Memory.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "Light.hpp"
#include "../utils/Math.hpp"
#include "../core/SceneObject.hpp"

namespace Renderer {

    struct FrameUniformBuffer {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        alignas(16) glm::vec3 cameraPos;
        alignas(16) float padding;
    };

    struct PushConstants {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 normalMatrix;
    };

    class Renderer {
    public:
        Renderer() = default;
        ~Renderer() { cleanup(); }

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        void init(Vulkan::Device& device, Vulkan::Swapchain& swapchain,
            Vulkan::RenderPass& renderPass, Vulkan::MemoryAllocator& allocator);
        void cleanup();

        void beginFrame();
        void endFrame();

        void renderScene(const Scene& scene, VkCommandBuffer commandBuffer, uint32_t imageIndex);

        VkCommandBuffer getCurrentCommandBuffer() const;
        VkDescriptorSet getCurrentDescriptorSet() const;
        Vulkan::DescriptorManager& getDescriptorManager() { return m_descriptorManager; }
        void setCurrentFrame(uint32_t frame) { m_currentFrame = frame; }

    private:
        void createUniformBuffers();
        void updateFrameUniformBuffer(uint32_t frameIndex, const Camera& camera);

        void createDescriptorSetLayout();
        void createDescriptorPool();
        void createDescriptorSets();

        void bindPipeline(VkCommandBuffer commandBuffer);
        void bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t frameIndex);

        Vulkan::Device* m_device = nullptr;
        Vulkan::Swapchain* m_swapchain = nullptr;
        Vulkan::RenderPass* m_renderPass = nullptr;
        Vulkan::MemoryAllocator* m_allocator = nullptr;

        Vulkan::DescriptorManager m_descriptorManager;
        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;

        std::vector<Vulkan::Buffer> m_frameUniformBuffers;
        std::vector<VkDescriptorSet> m_descriptorSets;

        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
        uint32_t m_currentFrame = 0;

        Vulkan::Pipeline* m_pipeline = nullptr;
    };

}