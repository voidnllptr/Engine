#include "Texture.hpp"
#include <stb_image.h>
#include <stdexcept>

static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

namespace Resources {

    Texture::Texture(Texture&& other) noexcept
        : m_image(other.m_image)
        , m_memory(other.m_memory)
        , m_imageView(other.m_imageView)
        , m_sampler(other.m_sampler)
        , m_width(other.m_width)
        , m_height(other.m_height)
        , m_channels(other.m_channels)
        , m_format(other.m_format)
        , m_device(other.m_device) {

        other.m_image = VK_NULL_HANDLE;
        other.m_memory = VK_NULL_HANDLE;
        other.m_imageView = VK_NULL_HANDLE;
        other.m_sampler = VK_NULL_HANDLE;
        other.m_width = 0;
        other.m_height = 0;
        other.m_channels = 0;
        other.m_format = VK_FORMAT_UNDEFINED;
        other.m_device = nullptr;
    }

    Texture& Texture::operator=(Texture&& other) noexcept {
        if (this != &other) {
            cleanup();

            m_image = other.m_image;
            m_memory = other.m_memory;
            m_imageView = other.m_imageView;
            m_sampler = other.m_sampler;
            m_width = other.m_width;
            m_height = other.m_height;
            m_channels = other.m_channels;
            m_format = other.m_format;
            m_device = other.m_device;

            other.m_image = VK_NULL_HANDLE;
            other.m_memory = VK_NULL_HANDLE;
            other.m_imageView = VK_NULL_HANDLE;
            other.m_sampler = VK_NULL_HANDLE;
            other.m_width = 0;
            other.m_height = 0;
            other.m_channels = 0;
            other.m_format = VK_FORMAT_UNDEFINED;
            other.m_device = nullptr;
        }
        return *this;
    }

    void Texture::loadFromFile(Vulkan::Device& device, Vulkan::MemoryAllocator& allocator,
        VkCommandPool commandPool, VkQueue graphicsQueue,
        const std::string& filepath) {
        cleanup();
        m_device = &device;

        stbi_uc* pixels = stbi_load(filepath.c_str(), &m_width, &m_height, &m_channels, STBI_rgb_alpha);

        if (!pixels) {
            throw std::runtime_error("Failed to load texture: " + filepath);
        }

        m_channels = 4;
        VkDeviceSize imageSize = static_cast<VkDeviceSize>(m_width) * m_height * 4;
        m_format = VK_FORMAT_R8G8B8A8_SRGB;

        Vulkan::Buffer stagingBuffer;
        stagingBuffer.createStaging(allocator, pixels, imageSize);

        stbi_image_free(pixels);

        createImage(
            device,
            m_width,
            m_height,
            m_format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        copyBufferToImage(device, stagingBuffer, commandPool, graphicsQueue);

        transitionImageLayout(device, commandPool, graphicsQueue,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        createImageView(device);
        createSampler(device);

        stagingBuffer.cleanup();
    }

    void Texture::createEmpty(Vulkan::Device& device, int width, int height,
        VkFormat format, VkImageUsageFlags usage,
        VkImageAspectFlags aspectFlags) {
        cleanup();
        m_device = &device;
        m_width = width;
        m_height = height;
        m_format = format;
        m_channels = 4;

        createImage(
            device,
            width,
            height,
            format,
            VK_IMAGE_TILING_OPTIMAL,
            usage,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        createImageView(device, aspectFlags);
        createSampler(device);
    }

    void Texture::createFromData(Vulkan::Device& device, Vulkan::MemoryAllocator& allocator,
        const void* data, int width, int height, int channels,
        VkCommandPool commandPool, VkQueue graphicsQueue) {

        cleanup();
        m_device = &device;
        m_width = width;
        m_height = height;
        m_channels = channels;
        m_format = getFormatFromChannels(channels);

        VkDeviceSize imageSize = static_cast<VkDeviceSize>(width) * height * channels;

        Vulkan::Buffer stagingBuffer;
        stagingBuffer.createStaging(allocator, data, imageSize);

        createImage(
            device,
            width,
            height,
            m_format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        transitionImageLayout(device, commandPool, graphicsQueue,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        vkDeviceWaitIdle(device.get());

        copyBufferToImage(device, stagingBuffer, commandPool, graphicsQueue);

        vkDeviceWaitIdle(device.get());

        transitionImageLayout(device, commandPool, graphicsQueue,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDeviceWaitIdle(device.get());

        createImageView(device);
        createSampler(device);

        stagingBuffer.cleanup();

        vkDeviceWaitIdle(device.get());
    }

    void Texture::createImage(Vulkan::Device& device, int width, int height,
        VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags memoryProperties) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(width);
        imageInfo.extent.height = static_cast<uint32_t>(height);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(device.get(), &imageInfo, nullptr, &m_image) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device.get(), m_image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(
            device.getPhysicalDevice(),
            memRequirements.memoryTypeBits,
            memoryProperties
        );

        if (vkAllocateMemory(device.get(), &allocInfo, nullptr, &m_memory) != VK_SUCCESS) {
            vkDestroyImage(device.get(), m_image, nullptr);
            m_image = VK_NULL_HANDLE;
            throw std::runtime_error("Failed to allocate image memory!");
        }

        vkBindImageMemory(device.get(), m_image, m_memory, 0);
    }

    void Texture::createSampler(Vulkan::Device& device, VkFilter magFilter, VkFilter minFilter,
        VkSamplerAddressMode addressMode, float maxLod) {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = magFilter;
        samplerInfo.minFilter = minFilter;
        samplerInfo.addressModeU = addressMode;
        samplerInfo.addressModeV = addressMode;
        samplerInfo.addressModeW = addressMode;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 16.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = maxLod;

        if (vkCreateSampler(device.get(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create texture sampler!");
        }
    }

    void Texture::createImageView(Vulkan::Device& device, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device.get(), &viewInfo, nullptr, &m_imageView) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create texture image view!");
        }
    }

    void Texture::transitionImageLayout(Vulkan::Device& device, VkCommandPool commandPool,
        VkQueue graphicsQueue, VkImageLayout oldLayout,
        VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer;
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        vkAllocateCommandBuffers(device.get(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkAccessFlags srcAccessMask = 0;
        VkAccessFlags dstAccessMask = 0;
        VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            srcAccessMask = 0;
            dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = srcAccessMask;
        barrier.dstAccessMask = dstAccessMask;

        vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0,
            0, nullptr, 0, nullptr, 1, &barrier);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkDeviceWaitIdle(device.get());

        vkFreeCommandBuffers(device.get(), commandPool, 1, &commandBuffer);
    }

    void Texture::copyBufferToImage(Vulkan::Device& device, Vulkan::Buffer& stagingBuffer,
        VkCommandPool commandPool, VkQueue graphicsQueue) {
        VkCommandBuffer commandBuffer;
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        vkAllocateCommandBuffers(device.get(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            static_cast<uint32_t>(m_width),
            static_cast<uint32_t>(m_height),
            1
        };

        vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.get(), m_image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device.get(), commandPool, 1, &commandBuffer);
    }

    VkFormat Texture::getFormatFromChannels(int channels) {
        switch (channels) {
        case 1: return VK_FORMAT_R8_UNORM;
        case 2: return VK_FORMAT_R8G8_UNORM;
        case 3: return VK_FORMAT_R8G8B8_UNORM;
        case 4: return VK_FORMAT_R8G8B8A8_SRGB;
        default: return VK_FORMAT_R8G8B8A8_SRGB;
        }
    }

    void Texture::cleanup() {
        if (m_device != nullptr) {
            if (m_sampler != VK_NULL_HANDLE) {
                vkDestroySampler(m_device->get(), m_sampler, nullptr);
                m_sampler = VK_NULL_HANDLE;
            }
            if (m_imageView != VK_NULL_HANDLE) {
                vkDestroyImageView(m_device->get(), m_imageView, nullptr);
                m_imageView = VK_NULL_HANDLE;
            }
            if (m_image != VK_NULL_HANDLE) {
                vkDestroyImage(m_device->get(), m_image, nullptr);
                m_image = VK_NULL_HANDLE;
            }
            if (m_memory != VK_NULL_HANDLE) {
                vkFreeMemory(m_device->get(), m_memory, nullptr);
                m_memory = VK_NULL_HANDLE;
            }
        }
        m_device = nullptr;
        m_width = 0;
        m_height = 0;
        m_channels = 0;
        m_format = VK_FORMAT_UNDEFINED;
    }

}