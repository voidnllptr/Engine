#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

#include "../utils/File.hpp"

namespace Renderer {

    class ShaderModule {
    public:
        ShaderModule() = default;
        ~ShaderModule();

        ShaderModule(const ShaderModule&) = delete;
        ShaderModule& operator=(const ShaderModule&) = delete;

        ShaderModule(ShaderModule&& other) noexcept;
        ShaderModule& operator=(ShaderModule&& other) noexcept;

        void create(VkDevice device, const std::string& filepath);
        void create(VkDevice device, const std::vector<char>& code);
        void cleanup();

        VkShaderModule get() const { return m_shaderModule; }

    private:
        VkShaderModule m_shaderModule = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
    };

    class ShaderProgram {
    public:
        void load(VkDevice device, const std::string& vertPath, const std::string& fragPath);
        void cleanup();

        std::vector<VkPipelineShaderStageCreateInfo> getStageCreateInfos() const;

        const ShaderModule& getVertex() const { return m_vertexShader; }
        const ShaderModule& getFragment() const { return m_fragmentShader; }

    private:
        VkDevice m_device = VK_NULL_HANDLE;
        ShaderModule m_vertexShader;
        ShaderModule m_fragmentShader;
    };

}