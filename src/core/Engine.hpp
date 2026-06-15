#pragma once

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "Window.hpp"
#include "../vulkan/Device.hpp"
#include "../vulkan/Swapchain.hpp"
#include "../vulkan/RenderPass.hpp"
#include "../vulkan/Pipeline.hpp"
#include "../vulkan/CommandBuffer.hpp"
#include "../vulkan/Sync.hpp"
#include "../vulkan/Framebuffer.hpp"
#include "../vulkan/Buffer.hpp"
#include "../vulkan/Memory.hpp"
#include "../vulkan/Descriptors.hpp"
#include "../resources/Shader.hpp"
#include "../resources/Model.hpp"
#include "../resources/Mesh.hpp"
#include "../resources/Texture.hpp"
#include "../renderer/Camera.hpp"
#include "../renderer/Light.hpp"
#include "../renderer/Scene.hpp"
#include "../renderer/Renderer.hpp"
#include "../utils/Math.hpp"
#include "../ui/ImGuiLayer.hpp"
#include "../ui/EditorUI.hpp"
#include "Timer.hpp"
#include "SceneObject.hpp"
#include "WindowSettings.hpp"

#include <functional>
#include <unordered_map>

namespace Core {

    struct FrameUniformBuffer {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        alignas(16) glm::vec3 cameraPos;
    };

    struct LightingUniforms {
        glm::vec4 lightDir;
        glm::vec4 ambientColor;
        glm::vec4 lightColor;
        float intensity;
        float padding[3];
    };

    struct ModelPushConstants {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 normalMatrix;
    };

    class Engine {
    public:
        Engine();
        ~Engine();

        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;

        void run();
        void shutdown();

        bool loadModel(const std::string& name, const std::string& filepath);
        bool removeModel(const std::string& name);
        bool removeModelByUniqueId(const std::string& uniqueId);
        void clearModels();

        bool renameObject(const std::string& oldName, const std::string& newName);
        bool renameObjectByUniqueId(const std::string& uniqueId, const std::string& newName);

        Core::SceneObject* getObject(const std::string& name);
        const Core::SceneObject* getObject(const std::string& name) const;
        Core::SceneObject* getObjectByUniqueId(const std::string& uniqueId);
        const Core::SceneObject* getObjectByUniqueId(const std::string& uniqueId) const;
        std::vector<Core::SceneObject*> getObjectsByName(const std::string& name);
        std::vector<const Core::SceneObject*> getObjectsByName(const std::string& name) const;

        void setCameraPosition(const glm::vec3& position);
        void setCameraTarget(const glm::vec3& target);
        void setCameraFOV(float fov);
        Renderer::Camera& getCamera() { return m_scene.getCamera(); }

        glm::vec3 getCameraPosition() const;
        glm::vec3 getCameraTarget() const;
        float getCameraFOV() const;

        void addLight(const Renderer::Light& light);
        void removeLight(size_t index);
        void setAmbientLight(const glm::vec3& color);

        void setObjectPosition(const std::string& name, const glm::vec3& position);
        void setObjectRotation(const std::string& name, const glm::vec3& rotation);
        void setObjectScale(const std::string& name, const glm::vec3& scale);
        void setObjectVisible(const std::string& name, bool visible);

        void setObjectPositionByUniqueId(const std::string& uniqueId, const glm::vec3& position);
        void setObjectRotationByUniqueId(const std::string& uniqueId, const glm::vec3& rotation);
        void setObjectScaleByUniqueId(const std::string& uniqueId, const glm::vec3& scale);
        void setObjectVisibleByUniqueId(const std::string& uniqueId, bool visible);

        glm::vec3 getObjectPosition(const std::string& name) const;
        glm::vec3 getObjectRotation(const std::string& name) const;
        glm::vec3 getObjectScale(const std::string& name) const;

        glm::vec3 getObjectPositionByUniqueId(const std::string& uniqueId) const;
        glm::vec3 getObjectRotationByUniqueId(const std::string& uniqueId) const;
        glm::vec3 getObjectScaleByUniqueId(const std::string& uniqueId) const;

        void setObjectTexture(const std::string& name, const std::string& texturePath);
        void setObjectTextureByUniqueId(const std::string& uniqueId, const std::string& texturePath);
        void removeObjectTexture(const std::string& name);
        void removeObjectTextureByUniqueId(const std::string& uniqueId);

        std::vector<std::string> getObjectNames() const;
        std::vector<std::string> getObjectDisplayNames() const;
        std::vector<std::string> getObjectUniqueIds() const;

        Renderer::Scene& getScene() { return m_scene; }
        const Renderer::Scene& getScene() const { return m_scene; }

        float getDeltaTime() const { return m_timer.getDeltaTime(); }
        float getFPS() const { return m_timer.getFPS(); }

    private:
        void init();
        void cleanup();
        void initSDL();
        void initVulkan();
        void initWindow();
        void initVulkanDevice();
        void initSwapchain();
        void initRenderPass();
        void initFramebuffers();
        void initCommandBuffers();
        void initSync();
        void initDefaultShaders();
        void initDefaultPipeline();
        void initDefaultScene();
        void initImGui();
        void createDefaultTexture();

        void createUniformBuffers();
        void createDescriptorSetLayout();
        void createDescriptorPool();
        void createDescriptorSets();
        void updateFrameUniformBuffer(uint32_t frameIndex, const Renderer::Camera& camera);
        void updateLightingUniformBuffer(uint32_t frameIndex);

        void mainLoop();
        void drawFrame();
        void processInput();
        void recreateSwapchain();
        void recordCommandBuffer(uint32_t imageIndex);
        void submitCommandBuffer(uint32_t imageIndex);

        void setupUICallbacks();

        VkDescriptorSet getOrCreateTextureDescriptorSet(const std::shared_ptr<Resources::Texture>& texture);

        std::string generateUniqueName(const std::string& baseName);

        void createDepthResources();

        VkInstance m_vkInstance = VK_NULL_HANDLE;

        bool m_isRunning = false;
        bool m_shouldClose = false;
        bool m_resized = false;

        Core::Timer m_timer;

        std::unique_ptr<Window> m_window;
        Vulkan::Device m_device;
        Vulkan::Swapchain m_swapchain;
        Vulkan::RenderPass m_renderPass;
        Vulkan::Pipeline m_defaultPipeline;
        VkPipelineLayout m_defaultPipelineLayout = VK_NULL_HANDLE;
        Vulkan::CommandBuffer m_commandBuffers;
        Vulkan::SyncManager m_sync;
        Vulkan::FramebufferCache m_framebuffers;
        Vulkan::MemoryAllocator m_memoryAllocator;
        Vulkan::DescriptorManager m_descriptorManager;

        VkImage m_depthImage = VK_NULL_HANDLE;
        VkDeviceMemory m_depthImageMemory = VK_NULL_HANDLE;
        VkImageView m_depthImageView = VK_NULL_HANDLE;
        VkFormat m_depthFormat = VK_FORMAT_D32_SFLOAT;

        std::vector<Vulkan::Buffer> m_frameUniformBuffers;
        std::vector<Vulkan::Buffer> m_lightingUniformBuffers;
        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_descriptorSets;

        Renderer::ShaderProgram m_defaultShaderProgram;

        Renderer::Scene m_scene;
        Resources::ModelManager m_modelManager;

        std::shared_ptr<Resources::Model> m_testCube;
        std::shared_ptr<Resources::Texture> m_defaultTexture;

        UI::ImGuiLayer m_imGuiLayer;
        UI::EditorUI m_editorUI;

        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
        uint32_t m_currentFrame = 0;

        std::unordered_map<Resources::Texture*, VkDescriptorSet> m_textureDescriptorCache;
        std::unordered_map<VkDescriptorSet, std::vector<std::shared_ptr<SceneObject>>> m_textureGroups;
        VkDescriptorSet m_defaultTextureDescriptorSet = VK_NULL_HANDLE;
        bool m_textureCacheNeedsUpdate = true;

        std::vector<VkDescriptorSet> m_textureDescriptorSets;
        void setupTextureDescriptorSet(uint32_t frameIndex, const std::shared_ptr<Resources::Texture>& texture);
    };

}