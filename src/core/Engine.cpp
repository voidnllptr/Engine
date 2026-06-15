#include "Engine.hpp"
#include <cstring>
#include <stdexcept>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_sdl3.h>

namespace Core {

    Engine::Engine()
        : m_vkInstance(VK_NULL_HANDLE)
        , m_window(nullptr)
        , m_isRunning(false)
        , m_shouldClose(false)
        , m_resized(false)
        , m_currentFrame(0) {
        init();
    }

    Engine::~Engine() {
        cleanup();
    }

    void Engine::run() {
        m_isRunning = true;
        mainLoop();
    }

    void Engine::shutdown() {
        m_isRunning = false;
    }

    bool Engine::loadModel(const std::string& name, const std::string& filepath) {
        auto model = m_modelManager.getModel(
            m_device, m_memoryAllocator,
            m_commandBuffers.getCommandPool(),
            m_device.getGraphicsQueue(),
            filepath
        );

        if (model) {
            std::string finalName = name;
            int counter = 1;
            while (m_scene.getObject(finalName) != nullptr) {
                finalName = name + "_" + std::to_string(counter);
                counter++;
            }

            auto gameObject = std::make_shared<SceneObject>(finalName, model);
            gameObject->setTextureDescriptorSet(m_defaultTextureDescriptorSet);
            m_scene.addObject(gameObject);
            std::cout << "[Engine] Model loaded: " << finalName << std::endl;
            return true;
        }
        return false;
    }

    bool Engine::removeModel(const std::string& name) {
        return m_scene.removeObject(name);
    }

    void Engine::clearModels() {
        m_scene.clear();
    }

    void Engine::setCameraPosition(const glm::vec3& position) {
        m_scene.setCameraPosition(position);
    }

    void Engine::setCameraTarget(const glm::vec3& target) {
        m_scene.setCameraTarget(target);
    }

    void Engine::setCameraFOV(float fov) {
        auto& camera = m_scene.getCamera();
        float aspect = camera.getAspectRatio();
        camera.setPerspective(fov, aspect, camera.getNearPlane(), camera.getFarPlane());
    }

    glm::vec3 Engine::getCameraPosition() const {
        return m_scene.getCamera().getPosition();
    }

    glm::vec3 Engine::getCameraTarget() const {
        glm::vec3 forward = m_scene.getCamera().getForward();
        glm::vec3 pos = m_scene.getCamera().getPosition();
        return pos + forward * 10.0f;
    }

    float Engine::getCameraFOV() const {
        return m_scene.getCamera().getFov();
    }

    void Engine::addLight(const Renderer::Light& light) {
        m_scene.addLight(light);
    }

    void Engine::removeLight(size_t index) {
        m_scene.removeLight(index);
    }

    void Engine::setAmbientLight(const glm::vec3& color) {
        m_scene.setAmbientColor(color);
    }

    void Engine::setObjectPosition(const std::string& name, const glm::vec3& position) {
        auto* obj = m_scene.getObject(name);
        if (obj) obj->setPosition(position);
    }

    void Engine::setObjectRotation(const std::string& name, const glm::vec3& rotation) {
        auto* obj = m_scene.getObject(name);
        if (obj) obj->setRotationEuler(rotation.x, rotation.y, rotation.z);
    }

    void Engine::setObjectScale(const std::string& name, const glm::vec3& scale) {
        auto* obj = m_scene.getObject(name);
        if (obj) obj->setScale(scale);
    }

    void Engine::setObjectVisible(const std::string& name, bool visible) {
        auto* obj = m_scene.getObject(name);
        if (obj) obj->setVisible(visible);
    }

    glm::vec3 Engine::getObjectPosition(const std::string& name) const {
        auto* obj = m_scene.getObject(name);
        return obj ? obj->getPosition() : glm::vec3(0.0f);
    }

    glm::vec3 Engine::getObjectRotation(const std::string& name) const {
        auto* obj = m_scene.getObject(name);
        return obj ? obj->getRotationEuler() : glm::vec3(0.0f);
    }

    glm::vec3 Engine::getObjectScale(const std::string& name) const {
        auto* obj = m_scene.getObject(name);
        return obj ? obj->getScale() : glm::vec3(1.0f);
    }

    std::vector<std::string> Engine::getObjectNames() const {
        std::vector<std::string> names;
        for (const auto& obj : m_scene.getObjects()) {
            if (obj) names.push_back(obj->getName());
        }
        return names;
    }

    void Engine::setObjectTexture(const std::string& name, const std::string& texturePath) {
        auto* obj = m_scene.getObject(name);
        if (!obj) return;

        if (texturePath.empty()) {
            removeObjectTexture(name);
            return;
        }

        auto texture = std::make_shared<Resources::Texture>();
        try {
            texture->loadFromFile(
                m_device, m_memoryAllocator,
                m_commandBuffers.getCommandPool(),
                m_device.getGraphicsQueue(),
                texturePath
            );

            if (!texture->isValid()) return;

            obj->setTexture(texture);
            VkDescriptorSet descriptorSet = getOrCreateTextureDescriptorSet(texture);
            obj->setTextureDescriptorSet(descriptorSet);
            std::cout << "[Engine] Texture applied to object '" << name << "'" << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "[Engine] Failed to load texture: " << e.what() << std::endl;
        }
    }

    void Engine::setObjectTextureByUniqueId(const std::string& uniqueId, const std::string& texturePath) {
        auto* obj = m_scene.getObjectByUniqueId(uniqueId);
        if (obj) setObjectTexture(obj->getName(), texturePath);
    }

    void Engine::removeObjectTexture(const std::string& name) {
        auto* obj = m_scene.getObject(name);
        if (obj) {
            obj->removeTexture();
            obj->setTextureDescriptorSet(m_defaultTextureDescriptorSet);
        }
    }

    void Engine::removeObjectTextureByUniqueId(const std::string& uniqueId) {
        auto* obj = m_scene.getObjectByUniqueId(uniqueId);
        if (obj) removeObjectTexture(obj->getName());
    }

    VkDescriptorSet Engine::getOrCreateTextureDescriptorSet(const std::shared_ptr<Resources::Texture>& texture) {
        auto it = m_textureDescriptorCache.find(texture.get());
        if (it != m_textureDescriptorCache.end()) {
            return it->second;
        }

        VkDescriptorSet descriptorSet;
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_descriptorSetLayout;

        if (vkAllocateDescriptorSets(m_device.get(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate texture descriptor set!");
        }

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture->getImageView();
        imageInfo.sampler = texture->getSampler();

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = descriptorSet;
        write.dstBinding = 2;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(m_device.get(), 1, &write, 0, nullptr);

        m_textureDescriptorCache[texture.get()] = descriptorSet;
        return descriptorSet;
    }

    void Engine::setupTextureDescriptorSet(uint32_t frameIndex, const std::shared_ptr<Resources::Texture>& texture) {
    }

    void Engine::createDepthResources() {
        VkExtent2D extent = m_swapchain.getExtent();

        std::vector<VkFormat> candidates = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
        m_depthFormat = VK_FORMAT_D32_SFLOAT;
        for (auto format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_device.getPhysicalDevice(), format, &props);
            if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                m_depthFormat = format;
                break;
            }
        }

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = extent.width;
        imageInfo.extent.height = extent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = m_depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(m_device.get(), &imageInfo, nullptr, &m_depthImage) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create depth image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_device.get(), m_depthImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = m_device.findMemoryType(
            memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        if (vkAllocateMemory(m_device.get(), &allocInfo, nullptr, &m_depthImageMemory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate depth image memory!");
        }

        vkBindImageMemory(m_device.get(), m_depthImage, m_depthImageMemory, 0);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_depthImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_device.get(), &viewInfo, nullptr, &m_depthImageView) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create depth image view!");
        }

        std::cout << "[Engine] Depth resources created with format: " << m_depthFormat << std::endl;
    }

    void Engine::initSwapchain() {
        int width = m_window->getWidth();
        int height = m_window->getHeight();
        VkSurfaceKHR surface = m_window->getSurface();

        m_swapchain.init(m_device, surface, width, height);
    }

    void Engine::initRenderPass() {
        m_renderPass.create(m_device.get(), m_swapchain.getImageFormat());
    }

    void Engine::init() {
        try {
            initSDL();
            initVulkan();
            initWindow();
            initVulkanDevice();
            initSwapchain();
            initRenderPass();
            initFramebuffers();
            initCommandBuffers();
            initSync();
            initDefaultShaders();

            createDescriptorSetLayout();
            createDescriptorPool();
            createUniformBuffers();
            createDescriptorSets();
            createDefaultTexture();

            initDefaultPipeline();
            initDefaultScene();
            initImGui();
            m_editorUI.init(this);
            setupUICallbacks();

            std::cout << "Engine initialized successfully!" << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to initialize engine: " << e.what() << std::endl;
            throw;
        }
    }

    void Engine::setupUICallbacks() {
        m_editorUI.setOnObjectLoaded([this](const std::string& name, const std::string& filepath) {
            loadModel(name, filepath);
            });

        m_editorUI.setOnTransformChanged([this](const std::string& name, const UI::TransformData& transform) {
            setObjectPosition(name, transform.position);
            setObjectRotation(name, transform.rotation);
            setObjectScale(name, transform.scale);
            });

        m_editorUI.setOnCameraChanged([this](const UI::CameraData& camera) {
            setCameraPosition(camera.position);
            setCameraTarget(camera.target);
            setCameraFOV(camera.fov);
            });

        m_editorUI.setOnObjectSelected([this](const std::string& name) {
            if (!name.empty()) {
                UI::TransformData transform;
                transform.position = getObjectPosition(name);
                transform.rotation = getObjectRotation(name);
                transform.scale = getObjectScale(name);
                m_editorUI.getSelectedObjectTransform() = transform;
            }
            });

        m_editorUI.setOnTextureLoaded([this](const std::string& name, const std::string& texturePath) {
            setObjectTexture(name, texturePath);
            });
    }

    void Engine::initWindow() {
        Core::WindowSettings ws;

        SDL_WindowFlags flags = SDL_WINDOW_VULKAN;
        if (ws.resizable) flags |= SDL_WINDOW_RESIZABLE;
        if (ws.fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

        SDL_Window* rawWindow = SDL_CreateWindow(
            ws.title.c_str(),
            ws.width,
            ws.height,
            flags
        );

        if (!rawWindow) {
            throw std::runtime_error("Failed to create window!");
        }

        m_window = std::make_unique<Window>(m_vkInstance, rawWindow);
    }

    void Engine::initVulkan() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan Engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        uint32_t extensionCount = 0;
        const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.enabledExtensionCount = extensionCount;
        instanceCreateInfo.ppEnabledExtensionNames = extensions;

        const char* validationLayers[] = { "VK_LAYER_KHRONOS_validation" };
#ifdef _DEBUG
        instanceCreateInfo.enabledLayerCount = 1;
        instanceCreateInfo.ppEnabledLayerNames = validationLayers;
#else
        instanceCreateInfo.enabledLayerCount = 0;
#endif

        if (vkCreateInstance(&instanceCreateInfo, nullptr, &m_vkInstance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance!");
        }
    }

    void Engine::initVulkanDevice() {
        VkSurfaceKHR surface = m_window->getSurface();

        m_device.init(m_vkInstance, surface);
        m_memoryAllocator.init(m_vkInstance, m_device.getPhysicalDevice(), m_device.get());
        m_descriptorManager.init(m_device);
    }

    void Engine::initFramebuffers() {
        createDepthResources();
        m_framebuffers.createWithDepth(
            m_device.get(),
            m_renderPass.get(),
            m_swapchain.getImageViews(),
            m_depthImageView,
            m_swapchain.getExtent()
        );
    }

    void Engine::initCommandBuffers() {
        m_commandBuffers.create(m_device, MAX_FRAMES_IN_FLIGHT);
    }

    void Engine::initSync() {
        m_sync.create(m_device, MAX_FRAMES_IN_FLIGHT);
    }

    void Engine::initDefaultShaders() {
        m_defaultShaderProgram.load(
            m_device.get(),
            "shaders/compiled/default.vert.spv",
            "shaders/compiled/default.frag.spv"
        );
    }

    void Engine::initDefaultPipeline() {
        auto bindingDesc = Resources::Vertex::getBindingDescription();
        auto attributes = Resources::Vertex::getAttributeDescriptions();

        std::vector<VkVertexInputBindingDescription> bindings = { bindingDesc };
        std::vector<VkVertexInputAttributeDescription> attributeDescs = attributes;

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(ModelPushConstants);

        std::vector<VkDescriptorSetLayout> layouts = {
            m_descriptorSetLayout,
            m_descriptorSetLayout
        };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(m_device.get(), &pipelineLayoutInfo, nullptr, &m_defaultPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }

        m_defaultPipeline.createGraphicsPipeline(
            m_device,
            m_renderPass.get(),
            "shaders/compiled/default.vert.spv",
            "shaders/compiled/default.frag.spv",
            bindings,
            attributeDescs,
            m_defaultPipelineLayout
        );
    }

    void Engine::createUniformBuffers() {
        VkDeviceSize frameBufferSize = sizeof(FrameUniformBuffer);
        VkDeviceSize lightingBufferSize = sizeof(LightingUniforms);

        m_frameUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        m_lightingUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            m_frameUniformBuffers[i].createUniform(m_memoryAllocator, frameBufferSize);
            m_lightingUniformBuffers[i].createUniform(m_memoryAllocator, lightingBufferSize);
        }
    }

    void Engine::createDescriptorSetLayout() {
        std::vector<VkDescriptorSetLayoutBinding> bindings(3);
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings[2].binding = 2;
        bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[2].descriptorCount = 1;
        bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(m_device.get(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout!");
        }
    }

    void Engine::createDescriptorPool() {
        const uint32_t MAX_TEXTURES = 100;

        std::vector<VkDescriptorPoolSize> poolSizes(2);
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT * 2 + MAX_TEXTURES;

        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = MAX_TEXTURES;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT * 2 + MAX_TEXTURES;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

        if (vkCreateDescriptorPool(m_device.get(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor pool!");
        }
    }

    void Engine::createDescriptorSets() {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
        allocInfo.pSetLayouts = layouts.data();

        m_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(m_device.get(), &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor sets!");
        }

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            VkDescriptorBufferInfo frameBufferInfo{};
            frameBufferInfo.buffer = m_frameUniformBuffers[i].get();
            frameBufferInfo.offset = 0;
            frameBufferInfo.range = sizeof(FrameUniformBuffer);

            VkDescriptorBufferInfo lightingBufferInfo{};
            lightingBufferInfo.buffer = m_lightingUniformBuffers[i].get();
            lightingBufferInfo.offset = 0;
            lightingBufferInfo.range = sizeof(LightingUniforms);

            std::vector<VkWriteDescriptorSet> writes(2);
            writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[0].dstSet = m_descriptorSets[i];
            writes[0].dstBinding = 0;
            writes[0].descriptorCount = 1;
            writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writes[0].pBufferInfo = &frameBufferInfo;

            writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[1].dstSet = m_descriptorSets[i];
            writes[1].dstBinding = 1;
            writes[1].descriptorCount = 1;
            writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writes[1].pBufferInfo = &lightingBufferInfo;

            vkUpdateDescriptorSets(m_device.get(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
        }
    }

    void Engine::createDefaultTexture() {
        uint32_t whitePixel = 0xFFFFFFFF;
        m_defaultTexture = std::make_shared<Resources::Texture>();
        m_defaultTexture->createFromData(
            m_device, m_memoryAllocator,
            &whitePixel, 1, 1, 4,
            m_commandBuffers.getCommandPool(),
            m_device.getGraphicsQueue()
        );

        m_defaultTextureDescriptorSet = getOrCreateTextureDescriptorSet(m_defaultTexture);
        std::cout << "[Engine] Default white texture created" << std::endl;
    }

    void Engine::updateFrameUniformBuffer(uint32_t frameIndex, const Renderer::Camera& camera) {
        FrameUniformBuffer ubo{};
        ubo.view = camera.getViewMatrix();
        ubo.proj = camera.getProjectionMatrix();
        ubo.proj[1][1] *= -1;
        ubo.cameraPos = camera.getPosition();
        m_frameUniformBuffers[frameIndex].upload(&ubo, sizeof(ubo));
    }

    void Engine::updateLightingUniformBuffer(uint32_t frameIndex) {
        LightingUniforms ubo{};
        ubo.lightDir = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        ubo.ambientColor = glm::vec4(0.15f, 0.15f, 0.15f, 0.0f);
        ubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
        ubo.intensity = 1.0f;
        m_lightingUniformBuffers[frameIndex].upload(&ubo, sizeof(ubo));
    }

    void Engine::initDefaultScene() {
        Renderer::Camera camera(glm::vec3(0.0f, 0.0f, -5.0f), 0.0f, 0.0f);
        camera.setPerspective(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
        m_scene.setCamera(camera);

        m_scene.addLight(Renderer::Light::createDirectional(
            glm::vec3(-1.0f, -1.0f, -1.0f),
            glm::vec3(1.0f, 1.0f, 1.0f), 1.0f));

        m_scene.addLight(Renderer::Light::createPoint(
            glm::vec3(2.0f, 3.0f, 2.0f),
            glm::vec3(1.0f, 0.5f, 0.3f), 0.5f));

        m_scene.setAmbientColor(glm::vec3(0.15f));

        m_testCube = Resources::Model::createCube(
            m_device, m_memoryAllocator,
            m_commandBuffers.getCommandPool(),
            m_device.getGraphicsQueue());

        if (m_testCube) {
            auto cubeObject = std::make_shared<SceneObject>("test_cube", m_testCube);
            cubeObject->setTextureDescriptorSet(m_defaultTextureDescriptorSet);
            m_scene.addObject(cubeObject);
        }
    }

    void Engine::cleanup() {
        m_device.waitIdle();

        m_scene.clear();
        m_modelManager.clear();
        m_testCube.reset();
        m_defaultTexture.reset();
        m_textureDescriptorCache.clear();

        for (auto& buffer : m_frameUniformBuffers) buffer.cleanup();
        for (auto& buffer : m_lightingUniformBuffers) buffer.cleanup();

        if (m_depthImageView != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device.get(), m_depthImageView, nullptr);
            vkDestroyImage(m_device.get(), m_depthImage, nullptr);
            vkFreeMemory(m_device.get(), m_depthImageMemory, nullptr);
        }

        if (m_descriptorSetLayout) vkDestroyDescriptorSetLayout(m_device.get(), m_descriptorSetLayout, nullptr);
        if (m_descriptorPool) vkDestroyDescriptorPool(m_device.get(), m_descriptorPool, nullptr);
        if (m_defaultPipelineLayout) vkDestroyPipelineLayout(m_device.get(), m_defaultPipelineLayout, nullptr);

        m_defaultShaderProgram.cleanup();
        m_defaultPipeline.cleanup();
        m_sync.cleanup();
        m_commandBuffers.cleanup();
        m_framebuffers.cleanup();
        m_renderPass.cleanup();
        m_swapchain.cleanup();
        m_descriptorManager.cleanup();
        m_memoryAllocator.cleanup();
        m_imGuiLayer.shutdown();

        if (m_window && m_window->getSurface() != VK_NULL_HANDLE && m_vkInstance != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(m_vkInstance, m_window->getSurface(), nullptr);
        }

        m_device.cleanup();

        if (m_vkInstance != VK_NULL_HANDLE) {
            vkDestroyInstance(m_vkInstance, nullptr);
            m_vkInstance = VK_NULL_HANDLE;
        }

        m_window.reset();

        SDL_Quit();
    }

    void Engine::initSDL() {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw std::runtime_error("Failed to initialize SDL!");
        }
    }

    void Engine::mainLoop() {
        m_timer.reset();
        while (m_isRunning && !m_shouldClose) {
            processInput();
            m_timer.tick();
            m_scene.update(m_timer.getDeltaTime());
            drawFrame();
        }
        m_device.waitIdle();
    }

    void Engine::drawFrame() {
        auto& fence = m_sync.getInFlightFence(m_currentFrame);
        fence.wait();
        fence.reset();

        m_commandBuffers.reset(m_currentFrame);

        VkSemaphore imageAvailableSemaphore = m_sync.getImageAvailableSemaphore(m_currentFrame).get();
        uint32_t imageIndex = m_swapchain.acquireNextImage(imageAvailableSemaphore);

        if (imageIndex == UINT32_MAX) {
            recreateSwapchain();
            return;
        }

        m_imGuiLayer.beginFrame();
        m_editorUI.draw();
        recordCommandBuffer(imageIndex);
        submitCommandBuffer(imageIndex);

        VkSemaphore renderFinishedSemaphore = m_sync.getRenderFinishedSemaphore(m_currentFrame).get();
        VkResult result = m_swapchain.present(m_device.getPresentQueue(), imageIndex, renderFinishedSemaphore);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_resized) {
            m_resized = false;
            recreateSwapchain();
            return;
        }

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Engine::recordCommandBuffer(uint32_t imageIndex) {
        m_commandBuffers.begin(m_currentFrame);

        VkCommandBuffer commandBuffer = m_commandBuffers.getCommandBuffer(m_currentFrame);

        VkClearValue clearValues[2];
        clearValues[0].color = { {0.1f, 0.1f, 0.15f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        m_commandBuffers.beginRenderPass(m_currentFrame, m_renderPass.get(),
            m_framebuffers.get(imageIndex),
            m_swapchain.getExtent(), clearValues[0], clearValues[1]);

        m_defaultPipeline.bind(commandBuffer);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapchain.getExtent().width);
        viewport.height = static_cast<float>(m_swapchain.getExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_swapchain.getExtent();
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        updateFrameUniformBuffer(m_currentFrame, m_scene.getCamera());
        updateLightingUniformBuffer(m_currentFrame);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_defaultPipelineLayout, 0, 1, &m_descriptorSets[m_currentFrame], 0, nullptr);

        for (const auto& object : m_scene.getObjects()) {
            if (!object || !object->isVisible() || !object->getModel()) continue;

            ModelPushConstants pushConstants{};
            pushConstants.model = object->getModelMatrix();
            pushConstants.normalMatrix = glm::transpose(glm::inverse(object->getModelMatrix()));

            vkCmdPushConstants(commandBuffer, m_defaultPipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelPushConstants), &pushConstants);

            VkDescriptorSet textureSet = m_defaultTextureDescriptorSet;
            if (object->hasTexture() && object->getTextureDescriptorSet() != VK_NULL_HANDLE) {
                textureSet = object->getTextureDescriptorSet();
            }

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_defaultPipelineLayout, 1, 1, &textureSet, 0, nullptr);

            for (const auto& mesh : object->getModel()->getMeshes()) {
                if (!mesh->isValid()) continue;

                VkBuffer vertexBuffers[] = { mesh->getVertexBuffer() };
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
                vkCmdBindIndexBuffer(commandBuffer, mesh->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(commandBuffer, mesh->getIndexCount(), 1, 0, 0, 0);
            }
        }

        m_imGuiLayer.endFrame(commandBuffer);

        m_commandBuffers.endRenderPass(m_currentFrame);
        m_commandBuffers.end(m_currentFrame);
    }

    void Engine::submitCommandBuffer(uint32_t imageIndex) {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_sync.getImageAvailableSemaphore(m_currentFrame).get() };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        VkCommandBuffer cmdBuffer = m_commandBuffers.getCommandBuffer(m_currentFrame);
        submitInfo.pCommandBuffers = &cmdBuffer;

        VkSemaphore signalSemaphores[] = { m_sync.getRenderFinishedSemaphore(m_currentFrame).get() };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VkFence fence = m_sync.getInFlightFence(m_currentFrame).get();
        vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo, fence);
    }

    void Engine::processInput() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            switch (event.type) {
            case SDL_EVENT_QUIT:
                m_shouldClose = true;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE)
                    m_shouldClose = true;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                m_resized = true;
                ImGui::GetIO().DisplaySize = ImVec2(
                    static_cast<float>(event.window.data1),
                    static_cast<float>(event.window.data2));
                break;
            }
        }

        if (m_window->isMinimized()) return;

        const bool* keys = SDL_GetKeyboardState(nullptr);
        float speed = 5.0f * m_timer.getDeltaTime();
        ImGuiIO& io = ImGui::GetIO();

        if (!io.WantCaptureKeyboard) {
            if (keys[SDL_SCANCODE_W]) m_scene.getCamera().moveForward(speed);
            if (keys[SDL_SCANCODE_S]) m_scene.getCamera().moveForward(-speed);
            if (keys[SDL_SCANCODE_A]) m_scene.getCamera().moveRight(-speed);
            if (keys[SDL_SCANCODE_D]) m_scene.getCamera().moveRight(speed);
            if (keys[SDL_SCANCODE_Q]) m_scene.getCamera().moveUp(speed);
            if (keys[SDL_SCANCODE_E]) m_scene.getCamera().moveUp(-speed);
        }
    }

    void Engine::recreateSwapchain() {
        std::cout << "Recreating swapchain..." << std::endl;

        vkDeviceWaitIdle(m_device.get());

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            m_sync.getInFlightFence(i).wait();
        }

        if (!m_window) return;

        int width = m_window->getWidth();
        int height = m_window->getHeight();

        while (width == 0 || height == 0) {
            width = m_window->getWidth();
            height = m_window->getHeight();
            SDL_PumpEvents();
        }

        m_framebuffers.cleanup();

        if (m_depthImageView != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device.get(), m_depthImageView, nullptr);
            vkDestroyImage(m_device.get(), m_depthImage, nullptr);
            vkFreeMemory(m_device.get(), m_depthImageMemory, nullptr);
            m_depthImageView = VK_NULL_HANDLE;
            m_depthImage = VK_NULL_HANDLE;
            m_depthImageMemory = VK_NULL_HANDLE;
        }

        VkSurfaceKHR surface = m_window->getSurface();

        try {
            m_swapchain.recreate(m_device, surface, width, height);
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to recreate swapchain: " << e.what() << std::endl;
            return;
        }

        createDepthResources();

        m_framebuffers.createWithDepth(
            m_device.get(),
            m_renderPass.get(),
            m_swapchain.getImageViews(),
            m_depthImageView,
            m_swapchain.getExtent()
        );

        ImGui::GetIO().DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));

        m_resized = false;

        std::cout << "Swapchain recreated successfully!" << std::endl;
    }

    bool Engine::removeModelByUniqueId(const std::string& uniqueId) {
        return m_scene.removeObjectByUniqueId(uniqueId);
    }

    bool Engine::renameObject(const std::string& oldName, const std::string& newName) {
        return m_scene.renameObject(oldName, newName);
    }

    bool Engine::renameObjectByUniqueId(const std::string& uniqueId, const std::string& newName) {
        auto* obj = m_scene.getObjectByUniqueId(uniqueId);
        if (obj) {
            obj->setName(newName);
            return true;
        }
        return false;
    }

    Core::SceneObject* Engine::getObject(const std::string& name) { return m_scene.getObject(name); }
    const Core::SceneObject* Engine::getObject(const std::string& name) const { return m_scene.getObject(name); }
    Core::SceneObject* Engine::getObjectByUniqueId(const std::string& uniqueId) { return m_scene.getObjectByUniqueId(uniqueId); }
    const Core::SceneObject* Engine::getObjectByUniqueId(const std::string& uniqueId) const { return m_scene.getObjectByUniqueId(uniqueId); }
    std::vector<Core::SceneObject*> Engine::getObjectsByName(const std::string& name) { return m_scene.getObjectsByName(name); }
    std::vector<const Core::SceneObject*> Engine::getObjectsByName(const std::string& name) const { return m_scene.getObjectsByName(name); }

    void Engine::setObjectPositionByUniqueId(const std::string& uniqueId, const glm::vec3& position) {
        auto* obj = m_scene.getObjectByUniqueId(uniqueId);
        if (obj) obj->setPosition(position);
    }

    void Engine::setObjectRotationByUniqueId(const std::string& uniqueId, const glm::vec3& rotation) {
        auto* obj = m_scene.getObjectByUniqueId(uniqueId);
        if (obj) obj->setRotationEuler(rotation.x, rotation.y, rotation.z);
    }

    void Engine::setObjectScaleByUniqueId(const std::string& uniqueId, const glm::vec3& scale) {
        auto* obj = m_scene.getObjectByUniqueId(uniqueId);
        if (obj) obj->setScale(scale);
    }

    void Engine::setObjectVisibleByUniqueId(const std::string& uniqueId, bool visible) {
        auto* obj = m_scene.getObjectByUniqueId(uniqueId);
        if (obj) obj->setVisible(visible);
    }

    glm::vec3 Engine::getObjectPositionByUniqueId(const std::string& uniqueId) const {
        auto* obj = m_scene.getObjectByUniqueId(uniqueId);
        return obj ? obj->getPosition() : glm::vec3(0.0f);
    }

    glm::vec3 Engine::getObjectRotationByUniqueId(const std::string& uniqueId) const {
        auto* obj = m_scene.getObjectByUniqueId(uniqueId);
        return obj ? obj->getRotationEuler() : glm::vec3(0.0f);
    }

    glm::vec3 Engine::getObjectScaleByUniqueId(const std::string& uniqueId) const {
        auto* obj = m_scene.getObjectByUniqueId(uniqueId);
        return obj ? obj->getScale() : glm::vec3(1.0f);
    }

    std::vector<std::string> Engine::getObjectDisplayNames() const {
        std::vector<std::string> names;
        for (const auto& obj : m_scene.getObjects()) {
            if (obj) names.push_back(obj->getDisplayName());
        }
        return names;
    }

    std::vector<std::string> Engine::getObjectUniqueIds() const {
        std::vector<std::string> ids;
        for (const auto& obj : m_scene.getObjects()) {
            if (obj) ids.push_back(obj->getUniqueId());
        }
        return ids;
    }

    std::string Engine::generateUniqueName(const std::string& baseName) {
        auto existingNames = getObjectNames();
        std::string newName = baseName;
        int counter = 1;
        while (std::find(existingNames.begin(), existingNames.end(), newName) != existingNames.end()) {
            newName = baseName + "_" + std::to_string(counter++);
        }
        return newName;
    }

    void Engine::initImGui() {
        uint32_t graphicsFamily = m_device.getQueueFamilies().graphicsFamily.value();
        m_imGuiLayer.init(m_window->getHandle(), m_vkInstance, m_device.getPhysicalDevice(),
            m_device.get(), m_device.getGraphicsQueue(), graphicsFamily,
            m_renderPass.get(), 2, static_cast<uint32_t>(m_swapchain.getImageViews().size()),
            m_commandBuffers.getCommandPool());
    }

}