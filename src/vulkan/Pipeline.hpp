#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <memory>

namespace Vulkan {

    class Device;

    class Pipeline {
    public:
        Pipeline() = default;
        ~Pipeline();

        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        Pipeline(Pipeline&& other) noexcept;
        Pipeline& operator=(Pipeline&& other) noexcept;

        void createGraphicsPipeline(
            Device& device,
            VkRenderPass renderPass,
            const std::string& vertShaderPath,
            const std::string& fragShaderPath,
            const std::vector<VkVertexInputBindingDescription>& bindingDescriptions,
            const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions,
            VkPipelineLayout pipelineLayout
        );

        void cleanup();

        VkPipeline get() const { return m_graphicsPipeline; }
        VkPipelineLayout getLayout() const { return m_pipelineLayout; }
        bool isValid() const { return m_graphicsPipeline != VK_NULL_HANDLE; }
        void bind(VkCommandBuffer commandBuffer) const;

    private:
        VkShaderModule createShaderModule(Device& device, const std::vector<char>& code);
        std::vector<char> readFile(const std::string& filename);

        VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        Device* m_device = nullptr;
    };

    struct VertexInputDescription {
        std::vector<VkVertexInputBindingDescription> bindings;
        std::vector<VkVertexInputAttributeDescription> attributes;

        static VertexInputDescription getDefaultVertexDescription();
    };

}