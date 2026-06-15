#pragma once

#include <string>
#include <vector>
#include <functional>
#include <glm/glm.hpp>

namespace Core {
    class Engine;
}

namespace UI {

    struct TransformData {
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f);
        glm::vec3 scale = glm::vec3(1.0f);
    };

    struct CameraData {
        glm::vec3 position = glm::vec3(5.0f, 5.0f, 5.0f);
        glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
        float fov = 60.0f;
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;
        float speed = 5.0f;
        float sensitivity = 0.3f;
    };

    class EditorUI {
    public:
        EditorUI();
        ~EditorUI();

        void init(Core::Engine* engine);
        void shutdown();

        void draw();

        TransformData& getSelectedObjectTransform() { return m_selectedTransform; }
        CameraData& getCameraData() { return m_cameraData; }
        bool isCameraControlled() const { return m_cameraControlled; }

        void loadObject(const std::string& name, const std::string& filepath);

        void setOnObjectLoaded(std::function<void(const std::string&, const std::string&)> callback);
        void setOnTransformChanged(std::function<void(const std::string&, const TransformData&)> callback);
        void setOnCameraChanged(std::function<void(const CameraData&)> callback);
        void setOnObjectSelected(std::function<void(const std::string&)> callback);
        void setOnTextureLoaded(std::function<void(const std::string&, const std::string&)> callback);

    private:
        void drawMainMenuBar();
        void drawLeftPanel();
        void drawRightPanel();
        void drawCenterPanel();

        void drawModelLoaderSection();
        void drawObjectListSection();
        void drawTransformSection();
        void drawCameraSection();
        void drawTextureSection();

        void drawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float speed = 0.1f);
        void drawFloatControl(const std::string& label, float& value, float min = 0.0f, float max = 100.0f, float speed = 1.0f);

        bool isValidObjFile(const std::string& filepath);

        Core::Engine* m_engine;

        bool m_showLeftPanel;
        bool m_showRightPanel;
        bool m_showModelLoader;
        bool m_showObjectList;
        bool m_showTransform;
        bool m_showCamera;
        bool m_showTextureLoader;

        char m_modelName[128];
        char m_modelPath[256];

        std::string m_selectedObject;
        TransformData m_selectedTransform;

        CameraData m_cameraData;
        bool m_cameraControlled;

        char m_texturePath[256];
        std::string m_currentTexturePreview;

        std::function<void(const std::string&, const std::string&)> m_onObjectLoaded;
        std::function<void(const std::string&, const TransformData&)> m_onTransformChanged;
        std::function<void(const CameraData&)> m_onCameraChanged;
        std::function<void(const std::string&)> m_onObjectSelected;
        std::function<void(const std::string&, const std::string&)> m_onTextureLoaded;
    };

}