#include "Renderer.hpp"
#include <stdexcept>
#include <cstring>

namespace Renderer {

    void Renderer::init(Vulkan::Device& device, Vulkan::Swapchain& swapchain,
        Vulkan::RenderPass& renderPass, Vulkan::MemoryAllocator& allocator) {
        m_device = &device;
        m_swapchain = &swapchain;
        m_renderPass = &renderPass;
        m_allocator = &allocator;

        m_descriptorManager.init(device);
        createUniformBuffers();
        createDescriptorSets();
    }

    void Renderer::cleanup() {
        if (m_device != nullptr) {
            m_device->waitIdle();
        }

        for (auto& buffer : m_frameUniformBuffers) {
            buffer.cleanup();
        }
        m_frameUniformBuffers.clear();

        m_descriptorManager.cleanup();

        m_device = nullptr;
        m_swapchain = nullptr;
        m_renderPass = nullptr;
        m_allocator = nullptr;
    }

    void Renderer::createUniformBuffers() {
        VkDeviceSize frameBufferSize = sizeof(FrameUniformBuffer);
        m_frameUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            m_frameUniformBuffers[i].createUniform(*m_allocator, frameBufferSize);
        }
    }

    void Renderer::updateFrameUniformBuffer(uint32_t frameIndex, const Camera& camera) {
        FrameUniformBuffer ubo{};
        ubo.view = camera.getViewMatrix();
        ubo.proj = camera.getProjectionMatrix();
        ubo.cameraPos = camera.getPosition();

        m_frameUniformBuffers[frameIndex].upload(&ubo, sizeof(ubo));
    }

    void Renderer::createDescriptorSets() {
        VkDescriptorSetLayout layout = m_descriptorManager.getCombinedLayout().get();
        VkDescriptorPool pool = m_descriptorManager.getCombinedPool().get();

        m_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = pool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &layout;

            if (vkAllocateDescriptorSets(m_device->get(), &allocInfo, &m_descriptorSets[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to allocate descriptor sets!");
            }

            VkDescriptorBufferInfo frameBufferInfo{};
            frameBufferInfo.buffer = m_frameUniformBuffers[i].get();
            frameBufferInfo.offset = 0;
            frameBufferInfo.range = sizeof(FrameUniformBuffer);

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = m_descriptorSets[i];
            write.dstBinding = 0;
            write.descriptorCount = 1;
            write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write.pBufferInfo = &frameBufferInfo;

            vkUpdateDescriptorSets(m_device->get(), 1, &write, 0, nullptr);
        }
    }

    void Renderer::beginFrame() {
    }

    void Renderer::endFrame() {
        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::renderScene(const Scene& scene, VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        (void)imageIndex;

        updateFrameUniformBuffer(m_currentFrame, scene.getCamera());
        bindPipeline(commandBuffer);
        bindDescriptorSets(commandBuffer, m_currentFrame);

        for (const auto& object : scene.getObjects()) {
            if (!object || !object->isVisible() || !object->getModel()) continue;

            PushConstants pushConstants{};
            pushConstants.model = object->getModelMatrix();
            pushConstants.normalMatrix = glm::transpose(glm::inverse(object->getModelMatrix()));

            vkCmdPushConstants(commandBuffer, VK_NULL_HANDLE,
                VK_SHADER_STAGE_VERTEX_BIT, 0,
                sizeof(PushConstants), &pushConstants);

            object->getModel()->draw(commandBuffer, VK_NULL_HANDLE);
        }
    }

    void Renderer::bindPipeline(VkCommandBuffer commandBuffer) {
    }

    void Renderer::bindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t frameIndex) {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            VK_NULL_HANDLE, 0, 1, &m_descriptorSets[frameIndex], 0, nullptr);
    }

    VkCommandBuffer Renderer::getCurrentCommandBuffer() const {
        return VK_NULL_HANDLE;
    }

    VkDescriptorSet Renderer::getCurrentDescriptorSet() const {
        return m_descriptorSets[m_currentFrame];
    }

}