#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include "../vulkan/Device.hpp"
#include "../vulkan/Buffer.hpp"
#include "../vulkan/Memory.hpp"

namespace Resources {

    class Texture {
    public:
        Texture() = default;
        ~Texture() { cleanup(); }

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;

        void loadFromFile(
            Vulkan::Device& device,
            Vulkan::MemoryAllocator& allocator,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            const std::string& filepath
        );

        void createEmpty(
            Vulkan::Device& device,
            int width,
            int height,
            VkFormat format,
            VkImageUsageFlags usage,
            VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT
        );

        void createFromData(
            Vulkan::Device& device,
            Vulkan::MemoryAllocator& allocator,
            const void* data,
            int width,
            int height,
            int channels,
            VkCommandPool commandPool,
            VkQueue graphicsQueue
        );

        void cleanup();

        VkImage getImage() const { return m_image; }
        VkImageView getImageView() const { return m_imageView; }
        VkSampler getSampler() const { return m_sampler; }
        Vulkan::Device* getDevice() const { return m_device; }

        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }
        int getChannels() const { return m_channels; }
        VkFormat getFormat() const { return m_format; }

        bool isValid() const { return m_image != VK_NULL_HANDLE; }

        void createSampler(
            Vulkan::Device& device,
            VkFilter magFilter = VK_FILTER_LINEAR,
            VkFilter minFilter = VK_FILTER_LINEAR,
            VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            float maxLod = VK_LOD_CLAMP_NONE
        );

        void createImageView(Vulkan::Device& device, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);

    private:
        void createImage(
            Vulkan::Device& device,
            int width,
            int height,
            VkFormat format,
            VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags memoryProperties
        );

        void transitionImageLayout(
            Vulkan::Device& device,
            VkCommandPool commandPool,
            VkQueue graphicsQueue,
            VkImageLayout oldLayout,
            VkImageLayout newLayout
        );

        void copyBufferToImage(
            Vulkan::Device& device,
            Vulkan::Buffer& stagingBuffer,
            VkCommandPool commandPool,
            VkQueue graphicsQueue
        );

        VkFormat getFormatFromChannels(int channels);

        VkImage m_image = VK_NULL_HANDLE;
        VkDeviceMemory m_memory = VK_NULL_HANDLE;
        VkImageView m_imageView = VK_NULL_HANDLE;
        VkSampler m_sampler = VK_NULL_HANDLE;

        int m_width = 0;
        int m_height = 0;
        int m_channels = 0;
        VkFormat m_format = VK_FORMAT_UNDEFINED;

        Vulkan::Device* m_device = nullptr;
    };

    struct TextureSamplerConfig {
        VkFilter magFilter = VK_FILTER_LINEAR;
        VkFilter minFilter = VK_FILTER_LINEAR;
        VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        float maxAnisotropy = 16.0f;
        bool anisotropyEnable = true;
        float minLod = 0.0f;
        float maxLod = VK_LOD_CLAMP_NONE;
        VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        bool compareEnable = false;
        VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;

        static TextureSamplerConfig getDefault() {
            return TextureSamplerConfig();
        }

        static TextureSamplerConfig getLinearClamp() {
            TextureSamplerConfig config;
            config.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            return config;
        }

        static TextureSamplerConfig getNearestRepeat() {
            TextureSamplerConfig config;
            config.magFilter = VK_FILTER_NEAREST;
            config.minFilter = VK_FILTER_NEAREST;
            return config;
        }
    };

} // namespace Resources