#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include <algorithm>

namespace Renderer {

    enum class DrawCommandType {
        Indexed,
        NonIndexed,
        Indirect,
        IndexedIndirect
    };

    struct DrawParameters {
        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;
        uint32_t instanceCount = 1;
        uint32_t firstVertex = 0;
        uint32_t firstIndex = 0;
        int32_t vertexOffset = 0;
        uint32_t firstInstance = 0;

        static DrawParameters indexed(uint32_t indexCount, uint32_t instanceCount = 1,
            uint32_t firstIndex = 0, int32_t vertexOffset = 0) {
            DrawParameters params;
            params.indexCount = indexCount;
            params.instanceCount = instanceCount;
            params.firstIndex = firstIndex;
            params.vertexOffset = vertexOffset;
            return params;
        }

        static DrawParameters nonIndexed(uint32_t vertexCount, uint32_t instanceCount = 1,
            uint32_t firstVertex = 0) {
            DrawParameters params;
            params.vertexCount = vertexCount;
            params.instanceCount = instanceCount;
            params.firstVertex = firstVertex;
            return params;
        }
    };

    struct DrawCommand {
        DrawParameters parameters;
        DrawCommandType type = DrawCommandType::Indexed;

        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VkBuffer indexBuffer = VK_NULL_HANDLE;
        VkDeviceSize vertexBufferOffset = 0;
        VkDeviceSize indexBufferOffset = 0;

        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

        std::vector<uint8_t> pushConstants;
        uint32_t pushConstantOffset = 0;

        int sortKey = 0;

        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

        glm::vec3 boundsMin = glm::vec3(-std::numeric_limits<float>::max());
        glm::vec3 boundsMax = glm::vec3(std::numeric_limits<float>::max());
        bool useCulling = false;

        uint64_t getSortKey() const {
            uint64_t key = 0;
            key |= (static_cast<uint64_t>(sortKey) << 32);
            if (pipeline != VK_NULL_HANDLE) {
                key |= reinterpret_cast<uint64_t>(pipeline) & 0xFFFFFFFF;
            }
            return key;
        }

        bool operator<(const DrawCommand& other) const {
            return getSortKey() < other.getSortKey();
        }

        DrawCommand() = default;

        DrawCommand(VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indexCount,
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE)
            : vertexBuffer(vertexBuffer)
            , indexBuffer(indexBuffer)
            , descriptorSet(descriptorSet) {
            parameters = DrawParameters::indexed(indexCount);
        }

        DrawCommand(VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indexCount,
            VkDescriptorSet descriptorSet, const void* pushData, uint32_t pushSize)
            : vertexBuffer(vertexBuffer)
            , indexBuffer(indexBuffer)
            , descriptorSet(descriptorSet) {
            parameters = DrawParameters::indexed(indexCount);
            pushConstants.resize(pushSize);
            memcpy(pushConstants.data(), pushData, pushSize);
        }
    };

    struct DrawCommandBuffer {
        std::vector<DrawCommand> commands;

        void clear() { commands.clear(); }

        void add(const DrawCommand& command) { commands.push_back(command); }

        void add(DrawCommand&& command) { commands.emplace_back(std::move(command)); }

        size_t size() const { return commands.size(); }

        bool empty() const { return commands.empty(); }

        void sort() {
            std::sort(commands.begin(), commands.end(),
                [](const DrawCommand& a, const DrawCommand& b) { return a < b; });
        }

        void reserve(size_t capacity) { commands.reserve(capacity); }
    };

    struct IndirectDrawCommand {
        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;

        VkDrawIndirectCommand toVkDrawIndirectCommand() const {
            VkDrawIndirectCommand cmd;
            cmd.vertexCount = vertexCount;
            cmd.instanceCount = instanceCount;
            cmd.firstVertex = firstVertex;
            cmd.firstInstance = firstInstance;
            return cmd;
        }
    };

    struct IndirectDrawIndexedCommand {
        uint32_t indexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        int32_t vertexOffset;
        uint32_t firstInstance;

        VkDrawIndexedIndirectCommand toVkDrawIndexedIndirectCommand() const {
            VkDrawIndexedIndirectCommand cmd;
            cmd.indexCount = indexCount;
            cmd.instanceCount = instanceCount;
            cmd.firstIndex = firstIndex;
            cmd.vertexOffset = vertexOffset;
            cmd.firstInstance = firstInstance;
            return cmd;
        }
    };

    struct Frustum {
        glm::vec4 planes[6];

        bool isBoxVisible(const glm::vec3& min, const glm::vec3& max) const {
            for (int i = 0; i < 6; ++i) {
                const auto& plane = planes[i];
                glm::vec3 positiveVertex(
                    plane.x > 0 ? max.x : min.x,
                    plane.y > 0 ? max.y : min.y,
                    plane.z > 0 ? max.z : min.z
                );
                if (glm::dot(glm::vec3(plane), positiveVertex) + plane.w < 0) {
                    return false;
                }
            }
            return true;
        }
    };

    using DrawCommandExecutor = std::function<void(VkCommandBuffer, const DrawCommand&)>;

    inline void executeDrawCommand(VkCommandBuffer cmdBuffer, const DrawCommand& cmd) {
        if (cmd.pipeline != VK_NULL_HANDLE) {
            vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, cmd.pipeline);
        }

        if (cmd.descriptorSet != VK_NULL_HANDLE && cmd.pipelineLayout != VK_NULL_HANDLE) {
            vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                cmd.pipelineLayout, 0, 1, &cmd.descriptorSet, 0, nullptr);
        }

        if (!cmd.pushConstants.empty() && cmd.pipelineLayout != VK_NULL_HANDLE) {
            vkCmdPushConstants(cmdBuffer, cmd.pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                cmd.pushConstantOffset,
                static_cast<uint32_t>(cmd.pushConstants.size()),
                cmd.pushConstants.data());
        }

        if (cmd.vertexBuffer != VK_NULL_HANDLE) {
            VkDeviceSize offsets[] = { cmd.vertexBufferOffset };
            vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &cmd.vertexBuffer, offsets);
        }

        switch (cmd.type) {
        case DrawCommandType::Indexed:
            if (cmd.indexBuffer != VK_NULL_HANDLE) {
                vkCmdBindIndexBuffer(cmdBuffer, cmd.indexBuffer, cmd.indexBufferOffset, VK_INDEX_TYPE_UINT32);
            }
            vkCmdDrawIndexed(cmdBuffer, cmd.parameters.indexCount, cmd.parameters.instanceCount,
                cmd.parameters.firstIndex, cmd.parameters.vertexOffset, cmd.parameters.firstInstance);
            break;

        case DrawCommandType::NonIndexed:
            vkCmdDraw(cmdBuffer, cmd.parameters.vertexCount, cmd.parameters.instanceCount,
                cmd.parameters.firstVertex, cmd.parameters.firstInstance);
            break;

        default:
            break;
        }
    }

}