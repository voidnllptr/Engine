#include "Pipeline.hpp"
#include "Device.hpp"
#include <fstream>
#include <stdexcept>
#include <cstring>

namespace Vulkan {

    Pipeline::~Pipeline() {
        cleanup();
    }

    Pipeline::Pipeline(Pipeline&& other) noexcept
        : m_graphicsPipeline(other.m_graphicsPipeline)
        , m_pipelineLayout(other.m_pipelineLayout)
        , m_device(other.m_device) {
        other.m_graphicsPipeline = VK_NULL_HANDLE;
        other.m_pipelineLayout = VK_NULL_HANDLE;
        other.m_device = nullptr;
    }

    Pipeline& Pipeline::operator=(Pipeline&& other) noexcept {
        if (this != &other) {
            cleanup();
            m_graphicsPipeline = other.m_graphicsPipeline;
            m_pipelineLayout = other.m_pipelineLayout;
            m_device = other.m_device;
            other.m_graphicsPipeline = VK_NULL_HANDLE;
            other.m_pipelineLayout = VK_NULL_HANDLE;
            other.m_device = nullptr;
        }
        return *this;
    }

    std::vector<char> Pipeline::readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }
        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    VkShaderModule Pipeline::createShaderModule(Device& device, const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device.get(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module!");
        }
        return shaderModule;
    }

    void Pipeline::createGraphicsPipeline(
        Device& device,
        VkRenderPass renderPass,
        const std::string& vertShaderPath,
        const std::string& fragShaderPath,
        const std::vector<VkVertexInputBindingDescription>& bindingDescriptions,
        const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions,
        VkPipelineLayout pipelineLayout) {

        m_device = &device;
        m_pipelineLayout = pipelineLayout;

        auto vertCode = readFile(vertShaderPath);
        auto fragCode = readFile(fragShaderPath);

        VkShaderModule vertShaderModule = createShaderModule(device, vertCode);
        VkShaderModule fragShaderModule = createShaderModule(device, fragCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
            vertShaderStageInfo,
            fragShaderStageInfo
        };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = nullptr;
        viewportState.scissorCount = 1;
        viewportState.pScissors = nullptr;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = /*VK_CULL_MODE_BACK_BIT*/VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;

        if (vkCreateGraphicsPipelines(device.get(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
            vkDestroyShaderModule(device.get(), vertShaderModule, nullptr);
            vkDestroyShaderModule(device.get(), fragShaderModule, nullptr);
            throw std::runtime_error("Failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device.get(), vertShaderModule, nullptr);
        vkDestroyShaderModule(device.get(), fragShaderModule, nullptr);
    }

    void Pipeline::bind(VkCommandBuffer commandBuffer) const {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
    }

    void Pipeline::cleanup() {
        if (m_device != nullptr) {
            if (m_graphicsPipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(m_device->get(), m_graphicsPipeline, nullptr);
                m_graphicsPipeline = VK_NULL_HANDLE;
            }
        }
        m_device = nullptr;
    }

    VertexInputDescription VertexInputDescription::getDefaultVertexDescription() {
        VertexInputDescription description;

        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(float) * 8;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        description.bindings.push_back(bindingDescription);

        VkVertexInputAttributeDescription positionAttribute{};
        positionAttribute.binding = 0;
        positionAttribute.location = 0;
        positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
        positionAttribute.offset = 0;
        description.attributes.push_back(positionAttribute);

        VkVertexInputAttributeDescription colorAttribute{};
        colorAttribute.binding = 0;
        colorAttribute.location = 1;
        colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
        colorAttribute.offset = sizeof(float) * 3;
        description.attributes.push_back(colorAttribute);

        VkVertexInputAttributeDescription texCoordAttribute{};
        texCoordAttribute.binding = 0;
        texCoordAttribute.location = 2;
        texCoordAttribute.format = VK_FORMAT_R32G32_SFLOAT;
        texCoordAttribute.offset = sizeof(float) * 6;
        description.attributes.push_back(texCoordAttribute);

        return description;
    }

}