#include "Shader.hpp"
#include <fstream>
#include <stdexcept>
#include <cstring>

namespace Renderer {

    ShaderModule::~ShaderModule() {
        cleanup();
    }

    ShaderModule::ShaderModule(ShaderModule&& other) noexcept
        : m_shaderModule(other.m_shaderModule)
        , m_device(other.m_device) {
        other.m_shaderModule = VK_NULL_HANDLE;
        other.m_device = VK_NULL_HANDLE;
    }

    ShaderModule& ShaderModule::operator=(ShaderModule&& other) noexcept {
        if (this != &other) {
            cleanup();
            m_shaderModule = other.m_shaderModule;
            m_device = other.m_device;
            other.m_shaderModule = VK_NULL_HANDLE;
            other.m_device = VK_NULL_HANDLE;
        }
        return *this;
    }

    void ShaderModule::create(VkDevice device, const std::string& filepath) {
        if (filepath.empty()) {
            throw std::invalid_argument("Shader filepath cannot be empty");
        }
        auto code = Utils::readSPIRVFile(filepath);
        create(device, code);
    }

    void ShaderModule::create(VkDevice device, const std::vector<char>& code) {
        cleanup();

        m_device = device;

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(device, &createInfo, nullptr, &m_shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module!");
        }
    }

    void ShaderModule::cleanup() {
        if (m_device != VK_NULL_HANDLE && m_shaderModule != VK_NULL_HANDLE) {
            vkDestroyShaderModule(m_device, m_shaderModule, nullptr);
            m_shaderModule = VK_NULL_HANDLE;
        }
    }

    void ShaderProgram::load(VkDevice device, const std::string& vertPath, const std::string& fragPath) {
        m_device = device;
        m_vertexShader.create(device, vertPath);
        m_fragmentShader.create(device, fragPath);
    }

    void ShaderProgram::cleanup() {
        m_vertexShader.cleanup();
        m_fragmentShader.cleanup();
        m_device = VK_NULL_HANDLE;
    }

    std::vector<VkPipelineShaderStageCreateInfo> ShaderProgram::getStageCreateInfos() const {
        std::vector<VkPipelineShaderStageCreateInfo> stages;
        stages.reserve(2);

        VkPipelineShaderStageCreateInfo vertStage{};
        vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertStage.module = m_vertexShader.get();
        vertStage.pName = "main";
        stages.push_back(vertStage);

        VkPipelineShaderStageCreateInfo fragStage{};
        fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragStage.module = m_fragmentShader.get();
        fragStage.pName = "main";
        stages.push_back(fragStage);

        return stages;
    }

}