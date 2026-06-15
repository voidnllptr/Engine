#include "Sync.hpp"
#include "Device.hpp"

namespace Vulkan {

    Semaphore::~Semaphore() {
    }

    Semaphore::Semaphore(Semaphore&& other) noexcept
        : m_semaphore(other.m_semaphore)
        , m_device(other.m_device) {
        other.m_semaphore = VK_NULL_HANDLE;
        other.m_device = nullptr;
    }

    Semaphore& Semaphore::operator=(Semaphore&& other) noexcept {
        if (this != &other) {
            if (m_device != nullptr && m_semaphore != VK_NULL_HANDLE) {
                cleanup();
            }
            m_semaphore = other.m_semaphore;
            m_device = other.m_device;
            other.m_semaphore = VK_NULL_HANDLE;
            other.m_device = nullptr;
        }
        return *this;
    }

    void Semaphore::create(Device& device) {
        m_device = &device;

        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(device.get(), &createInfo, nullptr, &m_semaphore) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create semaphore!");
        }
    }

    void Semaphore::cleanup() {
        if (m_device != nullptr && m_semaphore != VK_NULL_HANDLE) {
            vkDestroySemaphore(m_device->get(), m_semaphore, nullptr);
            m_semaphore = VK_NULL_HANDLE;
        }
        m_device = nullptr;
    }

    Fence::~Fence() {
    }

    Fence::Fence(Fence&& other) noexcept
        : m_fence(other.m_fence)
        , m_device(other.m_device) {
        other.m_fence = VK_NULL_HANDLE;
        other.m_device = nullptr;
    }

    Fence& Fence::operator=(Fence&& other) noexcept {
        if (this != &other) {
            if (m_device != nullptr && m_fence != VK_NULL_HANDLE) {
                cleanup();
            }
            m_fence = other.m_fence;
            m_device = other.m_device;
            other.m_fence = VK_NULL_HANDLE;
            other.m_device = nullptr;
        }
        return *this;
    }

    void Fence::create(Device& device, bool signaled) {
        m_device = &device;

        VkFenceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        if (signaled) {
            createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        }

        if (vkCreateFence(device.get(), &createInfo, nullptr, &m_fence) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create fence!");
        }
    }

    void Fence::cleanup() {
        if (m_device != nullptr && m_fence != VK_NULL_HANDLE) {
            vkDestroyFence(m_device->get(), m_fence, nullptr);
            m_fence = VK_NULL_HANDLE;
        }
        m_device = nullptr;
    }

    void Fence::wait(uint64_t timeout) {
        if (m_device != nullptr && m_fence != VK_NULL_HANDLE) {
            vkWaitForFences(m_device->get(), 1, &m_fence, VK_TRUE, timeout);
        }
    }

    void Fence::reset() {
        if (m_device != nullptr && m_fence != VK_NULL_HANDLE) {
            vkResetFences(m_device->get(), 1, &m_fence);
        }
    }

    SyncManager::~SyncManager() {
    }

    SyncManager::SyncManager(SyncManager&& other) noexcept
        : m_imageAvailableSemaphores(std::move(other.m_imageAvailableSemaphores))
        , m_renderFinishedSemaphores(std::move(other.m_renderFinishedSemaphores))
        , m_inFlightFences(std::move(other.m_inFlightFences))
        , m_device(other.m_device) {
        other.m_device = nullptr;
    }

    SyncManager& SyncManager::operator=(SyncManager&& other) noexcept {
        if (this != &other) {
            if (m_device != nullptr) {
                cleanup();
            }
            m_imageAvailableSemaphores = std::move(other.m_imageAvailableSemaphores);
            m_renderFinishedSemaphores = std::move(other.m_renderFinishedSemaphores);
            m_inFlightFences = std::move(other.m_inFlightFences);
            m_device = other.m_device;
            other.m_device = nullptr;
        }
        return *this;
    }

    void SyncManager::create(Device& device, uint32_t frameCount) {
        m_device = &device;

        m_imageAvailableSemaphores.resize(frameCount);
        m_renderFinishedSemaphores.resize(frameCount);
        m_inFlightFences.resize(frameCount);

        for (uint32_t i = 0; i < frameCount; i++) {
            m_imageAvailableSemaphores[i].create(device);
            m_renderFinishedSemaphores[i].create(device);
            m_inFlightFences[i].create(device, true);
        }
    }

    void SyncManager::cleanup() {
        if (m_device != nullptr) {
            for (auto& semaphore : m_imageAvailableSemaphores) {
                semaphore.cleanup();
            }
            for (auto& semaphore : m_renderFinishedSemaphores) {
                semaphore.cleanup();
            }
            for (auto& fence : m_inFlightFences) {
                fence.cleanup();
            }
        }

        m_imageAvailableSemaphores.clear();
        m_renderFinishedSemaphores.clear();
        m_inFlightFences.clear();
        m_device = nullptr;
    }

    void SyncManager::waitAll() {
        if (m_device != nullptr) {
            for (auto& fence : m_inFlightFences) {
                if (fence.isValid()) {
                    fence.wait();
                }
            }
        }
    }

}