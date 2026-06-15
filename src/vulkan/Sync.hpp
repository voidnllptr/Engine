#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>

namespace Vulkan {

    class Device;

    class Semaphore {
    public:
        Semaphore() = default;
        ~Semaphore();

        Semaphore(const Semaphore&) = delete;
        Semaphore& operator=(const Semaphore&) = delete;

        Semaphore(Semaphore&& other) noexcept;
        Semaphore& operator=(Semaphore&& other) noexcept;

        void create(Device& device);
        void cleanup();

        VkSemaphore get() const { return m_semaphore; }
        bool isValid() const { return m_semaphore != VK_NULL_HANDLE; }

    private:
        VkSemaphore m_semaphore = VK_NULL_HANDLE;
        Device* m_device = nullptr;
    };

    class Fence {
    public:
        Fence() = default;
        ~Fence();

        Fence(const Fence&) = delete;
        Fence& operator=(const Fence&) = delete;

        Fence(Fence&& other) noexcept;
        Fence& operator=(Fence&& other) noexcept;

        void create(Device& device, bool signaled = false);
        void cleanup();

        void wait(uint64_t timeout = UINT64_MAX);
        void reset();

        VkFence get() const { return m_fence; }
        bool isValid() const { return m_fence != VK_NULL_HANDLE; }

    private:
        VkFence m_fence = VK_NULL_HANDLE;
        Device* m_device = nullptr;
    };

    class SyncManager {
    public:
        SyncManager() = default;
        ~SyncManager();

        SyncManager(const SyncManager&) = delete;
        SyncManager& operator=(const SyncManager&) = delete;

        SyncManager(SyncManager&& other) noexcept;
        SyncManager& operator=(SyncManager&& other) noexcept;

        void create(Device& device, uint32_t frameCount);
        void cleanup();

        Semaphore& getImageAvailableSemaphore(uint32_t frameIndex) { return m_imageAvailableSemaphores[frameIndex]; }
        Semaphore& getRenderFinishedSemaphore(uint32_t frameIndex) { return m_renderFinishedSemaphores[frameIndex]; }
        Fence& getInFlightFence(uint32_t frameIndex) { return m_inFlightFences[frameIndex]; }

        VkSemaphore getImageAvailable(uint32_t frameIndex) const { return m_imageAvailableSemaphores[frameIndex].get(); }
        VkSemaphore getRenderFinished(uint32_t frameIndex) const { return m_renderFinishedSemaphores[frameIndex].get(); }
        VkFence getInFlightFence(uint32_t frameIndex) const { return m_inFlightFences[frameIndex].get(); }

        uint32_t getFrameCount() const { return static_cast<uint32_t>(m_imageAvailableSemaphores.size()); }

        void waitAll();

    private:
        std::vector<Semaphore> m_imageAvailableSemaphores;
        std::vector<Semaphore> m_renderFinishedSemaphores;
        std::vector<Fence> m_inFlightFences;

        Device* m_device = nullptr;
    };

}