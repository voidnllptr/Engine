#include "EditorUI.hpp"
#include "../core/Engine.hpp"
#include "imgui.h"
#include "ImGuiFileDialog.h"
#include <iostream>
#include <cstring>
#include <filesystem>
#include <fstream>

namespace UI {

    EditorUI::EditorUI()
        : m_engine(nullptr)
        , m_showLeftPanel(true)
        , m_showRightPanel(true)
        , m_showModelLoader(true)
        , m_showObjectList(true)
        , m_showTransform(true)
        , m_showCamera(true)
        , m_showTextureLoader(true)
        , m_cameraControlled(false) {

        memset(m_modelName, 0, sizeof(m_modelName));
        strcpy_s(m_modelName, sizeof(m_modelName), "new_object");
        memset(m_modelPath, 0, sizeof(m_modelPath));
        strcpy_s(m_modelPath, sizeof(m_modelPath), "models/");
        memset(m_texturePath, 0, sizeof(m_texturePath));
        std::cout << "[EditorUI] Constructor" << std::endl;
    }

    EditorUI::~EditorUI() {
        shutdown();
    }

    void EditorUI::init(Core::Engine* engine) {
        m_engine = engine;

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.FrameRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.18f, 0.18f, 0.18f, 0.95f);

        style.WindowPadding = ImVec2(12.0f, 12.0f);
        style.FramePadding = ImVec2(8.0f, 6.0f);
        style.ItemSpacing = ImVec2(10.0f, 8.0f);
        style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);

        std::cout << "[EditorUI] Initialized" << std::endl;
    }

    void EditorUI::shutdown() {
        m_engine = nullptr;
        std::cout << "[EditorUI] Shutdown" << std::endl;
    }

    void EditorUI::draw() {
        drawMainMenuBar();
        drawLeftPanel();
        drawRightPanel();
        drawCenterPanel();
    }

    void EditorUI::drawMainMenuBar() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Load Object...", "Ctrl+O")) {
                    IGFD::FileDialogConfig config;
                    config.path = "./models";
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseObjFile", "Choose .obj File", ".obj", config);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Esc")) {
                    if (m_engine) {
                        m_engine->shutdown();
                    }
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Left Panel", nullptr, &m_showLeftPanel);
                ImGui::MenuItem("Right Panel", nullptr, &m_showRightPanel);
                ImGui::Separator();
                ImGui::MenuItem("Model Loader", nullptr, &m_showModelLoader);
                ImGui::MenuItem("Object List", nullptr, &m_showObjectList);
                ImGui::MenuItem("Transform", nullptr, &m_showTransform);
                ImGui::MenuItem("Texture Loader", nullptr, &m_showTextureLoader);
                ImGui::MenuItem("Camera Settings", nullptr, &m_showCamera);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }

    void EditorUI::drawLeftPanel() {
        if (!m_showLeftPanel) return;

        ImGuiIO& io = ImGui::GetIO();
        float menuBarHeight = ImGui::GetFrameHeight();

        float panelWidth = io.DisplaySize.x * 0.25f;
        panelWidth = std::max(280.0f, std::min(panelWidth, 500.0f));
        float panelHeight = io.DisplaySize.y - menuBarHeight - 10.0f;

        ImGui::SetNextWindowPos(ImVec2(0, menuBarHeight));
        ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse;

        if (ImGui::Begin("Left Panel", nullptr, flags)) {
            if (ImGui::BeginChild("LeftPanelContent", ImVec2(0, 0), false)) {
                if (m_showModelLoader) {
                    if (ImGui::CollapsingHeader("Model Loader", ImGuiTreeNodeFlags_DefaultOpen)) {
                        drawModelLoaderSection();
                    }
                    ImGui::Separator();
                }

                if (m_showObjectList) {
                    if (ImGui::CollapsingHeader("Scene Objects", ImGuiTreeNodeFlags_DefaultOpen)) {
                        drawObjectListSection();
                    }
                }
                ImGui::EndChild();
            }
        }
        ImGui::End();

        if (ImGuiFileDialog::Instance()->Display("ChooseObjFile")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

                std::string modelName = fileName;
                size_t lastdot = modelName.find_last_of('.');
                if (lastdot != std::string::npos) {
                    modelName = modelName.substr(0, lastdot);
                }

                loadObject(modelName, filePath);
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }

    void EditorUI::drawRightPanel() {
        if (!m_showRightPanel) return;

        ImGuiIO& io = ImGui::GetIO();
        float menuBarHeight = ImGui::GetFrameHeight();

        float panelWidth = io.DisplaySize.x * 0.25f;
        panelWidth = std::max(280.0f, std::min(panelWidth, 500.0f));
        float panelHeight = io.DisplaySize.y - menuBarHeight - 10.0f;

        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - panelWidth, menuBarHeight));
        ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse;

        if (ImGui::Begin("Right Panel", nullptr, flags)) {
            if (ImGui::BeginChild("RightPanelContent", ImVec2(0, 0), false)) {
                if (m_showTransform && !m_selectedObject.empty()) {
                    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                        drawTransformSection();
                    }
                    ImGui::Separator();
                }

                if (m_showTextureLoader && !m_selectedObject.empty()) {
                    if (ImGui::CollapsingHeader("Texture Loader", ImGuiTreeNodeFlags_DefaultOpen)) {
                        drawTextureSection();
                    }
                    ImGui::Separator();
                }

                if (m_showCamera) {
                    if (ImGui::CollapsingHeader("Camera Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                        drawCameraSection();
                    }
                }
                ImGui::EndChild();
            }
        }
        ImGui::End();
    }

    void EditorUI::drawCenterPanel() {
        ImGuiIO& io = ImGui::GetIO();
        float menuBarHeight = ImGui::GetFrameHeight();

        float leftPanelWidth = io.DisplaySize.x * 0.25f;
        leftPanelWidth = std::max(280.0f, std::min(leftPanelWidth, 500.0f));
        float rightPanelWidth = leftPanelWidth;

        float centerPanelX = leftPanelWidth + 10.0f;
        float centerPanelWidth = io.DisplaySize.x - leftPanelWidth - rightPanelWidth - 20.0f;
        float centerPanelHeight = 80.0f;

        ImGui::SetNextWindowPos(ImVec2(centerPanelX, menuBarHeight + 10.0f));
        ImGui::SetNextWindowSize(ImVec2(centerPanelWidth, centerPanelHeight));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse;

        if (ImGui::Begin("Info Panel", nullptr, flags)) {
            if (m_engine) {
                ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
                ImGui::Text("Delta Time: %.3f ms", ImGui::GetIO().DeltaTime * 1000.0f);
                ImGui::Text("Selected Object: %s", m_selectedObject.empty() ? "None" : m_selectedObject.c_str());
                ImGui::Text("Window Size: %.0f x %.0f", io.DisplaySize.x, io.DisplaySize.y);
            }
        }
        ImGui::End();
    }

    bool EditorUI::isValidObjFile(const std::string& filepath) {
        if (filepath.size() < 4) {
            std::cerr << "[EditorUI] File path too short" << std::endl;
            return false;
        }
        std::string extension = filepath.substr(filepath.size() - 4);
        if (extension != ".obj") {
            std::cerr << "[EditorUI] File does not have .obj extension: " << filepath << std::endl;
            return false;
        }

        if (!std::filesystem::exists(filepath)) {
            std::cerr << "[EditorUI] File does not exist: " << filepath << std::endl;
            return false;
        }

        auto fileSize = std::filesystem::file_size(filepath);
        if (fileSize == 0) {
            std::cerr << "[EditorUI] File is empty: " << filepath << std::endl;
            return false;
        }

        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "[EditorUI] Cannot open file: " << filepath << std::endl;
            return false;
        }

        bool hasVertices = false;
        bool hasFaces = false;
        std::string line;

        while (std::getline(file, line)) {
            if (line.empty()) continue;
            size_t pos = line.find_first_not_of(" \t");
            if (pos == std::string::npos) continue;
            if (line[pos] == '#') continue;

            if (line[pos] == 'v') {
                if (line.size() > pos + 1 && (line[pos + 1] == ' ' || line[pos + 1] == '\t')) {
                    hasVertices = true;
                }
            }
            else if (line[pos] == 'f') {
                if (line.size() > pos + 1) {
                    char nextChar = line[pos + 1];
                    if (nextChar == ' ' || nextChar == '\t' || std::isdigit(nextChar)) {
                        hasFaces = true;
                    }
                }
            }

            if (hasVertices && hasFaces) break;
        }

        file.close();

        std::cout << "[EditorUI] File preview (first 10 lines):" << std::endl;
        std::ifstream debugFile(filepath);
        for (int i = 0; i < 10 && std::getline(debugFile, line); ++i) {
            std::cout << "  " << line << std::endl;
        }
        debugFile.close();

        if (!hasVertices) {
            std::cerr << "[EditorUI] No vertex data found in .obj file" << std::endl;
            return false;
        }
        if (!hasFaces) {
            std::cerr << "[EditorUI] No face data found in .obj file" << std::endl;
            return false;
        }
        std::cout << "[EditorUI] File validation passed: " << filepath << std::endl;
        return true;
    }

    void EditorUI::drawModelLoaderSection() {
        ImGui::Text("Load 3D Model");
        ImGui::Separator();

        ImGui::InputText("Name", m_modelName, sizeof(m_modelName));

        ImGui::InputText("File Path", m_modelPath, sizeof(m_modelPath));
        ImGui::SameLine();

        if (ImGui::Button("Browse...", ImVec2(80, 0))) {
            IGFD::FileDialogConfig config;
            config.path = "./models";
            ImGuiFileDialog::Instance()->OpenDialog("ChooseObjFileSection", "Choose .obj File", ".obj", config);
        }

        ImGui::Spacing();

        float buttonWidth = ImGui::GetContentRegionAvail().x;

        if (ImGui::Button("Load OBJ", ImVec2(buttonWidth, 40))) {
            if (strlen(m_modelName) == 0) {
                std::cerr << "[EditorUI] Please enter a model name" << std::endl;
            }
            else if (strlen(m_modelPath) == 0) {
                std::cerr << "[EditorUI] Please select a file" << std::endl;
            }
            else {
                if (isValidObjFile(m_modelPath)) {
                    loadObject(m_modelName, m_modelPath);
                }
                else {
                    std::cerr << "[EditorUI] Invalid .obj file: " << m_modelPath << std::endl;
                }
            }
        }

        ImGui::Spacing();
        ImGui::TextDisabled("Supported formats: OBJ");

        if (ImGuiFileDialog::Instance()->Display("ChooseObjFileSection")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

                strcpy_s(m_modelPath, sizeof(m_modelPath), filePath.c_str());

                size_t lastdot = fileName.find_last_of('.');
                if (lastdot != std::string::npos) {
                    fileName = fileName.substr(0, lastdot);
                }
                strcpy_s(m_modelName, sizeof(m_modelName), fileName.c_str());
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }

    void EditorUI::drawObjectListSection() {
        if (!m_engine) {
            ImGui::TextDisabled("Engine not initialized");
            return;
        }

        auto objectNames = m_engine->getObjectNames();
        auto objectIds = m_engine->getObjectUniqueIds();

        ImGui::Text("Objects: %zu", objectNames.size());
        ImGui::Separator();

        float buttonWidth = ImGui::GetContentRegionAvail().x * 0.48f;

        if (ImGui::Button("Clear All", ImVec2(buttonWidth, 35))) {
            m_engine->clearModels();
            m_selectedObject.clear();
            if (m_onObjectSelected) {
                m_onObjectSelected("");
            }
            std::cout << "[UI] Clear All" << std::endl;
        }

        ImGui::SameLine();

        if (ImGui::Button("Refresh", ImVec2(buttonWidth, 35))) {
            if (!m_selectedObject.empty()) {
                bool found = false;
                for (const auto& name : objectNames) {
                    if (name == m_selectedObject) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    std::cout << "[UI] Refresh: selected object '" << m_selectedObject << "' no longer exists, clearing selection." << std::endl;
                    m_selectedObject.clear();
                    if (m_onObjectSelected) {
                        m_onObjectSelected("");
                    }
                }
            }
        }

        ImGui::Separator();

        float buttonHeight = std::max(40.0f, ImGui::GetFrameHeight() * 1.8f);

        for (size_t i = 0; i < objectNames.size(); ++i) {
            const auto& name = objectNames[i];
            const auto& uniqueId = objectIds[i];

            std::string displayName = name;

            int duplicateCount = 0;
            for (const auto& n : objectNames) {
                if (n == name) duplicateCount++;
            }

            if (duplicateCount > 1) {
                std::string shortId = uniqueId;
                if (shortId.length() > 8) {
                    shortId = shortId.substr(shortId.length() - 8);
                }
                displayName = name + " [" + shortId + "]";
            }

            bool isSelected = (m_selectedObject == name);

            if (isSelected) {
                ImVec4 color = ImGui::GetStyle().Colors[ImGuiCol_Header];
                ImGui::PushStyleColor(ImGuiCol_Button, color);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
            }

            ImGui::PushID(uniqueId.c_str());

            if (ImGui::Button(displayName.c_str(), ImVec2(-1, buttonHeight))) {
                std::cout << "[UI] Selected object: " << name << " (id: " << uniqueId << ")" << std::endl;
                m_selectedObject = name;

                if (m_engine) {
                    glm::vec3 pos = m_engine->getObjectPosition(name);
                    glm::vec3 rot = m_engine->getObjectRotation(name);
                    glm::vec3 scl = m_engine->getObjectScale(name);
                    m_selectedTransform.position = pos;
                    m_selectedTransform.rotation = rot;
                    m_selectedTransform.scale = scl;
                }

                if (m_onObjectSelected) {
                    m_onObjectSelected(name);
                }
            }

            ImGui::PopID();

            if (isSelected) {
                ImGui::PopStyleColor(2);
            }
        }
    }

    void EditorUI::drawTransformSection() {
        ImGui::Text("Selected: %s", m_selectedObject.c_str());
        ImGui::Separator();

        static char renameBuffer[128] = "";
        static bool renaming = false;

        if (!renaming) {
            ImGui::Text("Name: %s", m_selectedObject.c_str());
            ImGui::SameLine();
            if (ImGui::SmallButton("Rename")) {
                strcpy_s(renameBuffer, sizeof(renameBuffer), m_selectedObject.c_str());
                renaming = true;
            }
        }
        else {
            ImGui::InputText("New Name", renameBuffer, sizeof(renameBuffer));
            ImGui::SameLine();
            if (ImGui::SmallButton("Apply")) {
                if (strlen(renameBuffer) > 0 && m_engine) {
                    if (m_engine->renameObject(m_selectedObject, renameBuffer)) {
                        m_selectedObject = renameBuffer;
                        if (m_onObjectSelected) {
                            m_onObjectSelected(m_selectedObject);
                        }
                    }
                }
                renaming = false;
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Cancel")) {
                renaming = false;
            }
        }
        ImGui::Separator();

        if (m_engine && !m_selectedObject.empty()) {
            auto* obj = m_engine->getObject(m_selectedObject);
            if (!obj) {
                ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1), "Warning: Object no longer exists in scene!");
                std::cerr << "[UI] Transform section: object '" << m_selectedObject << "' not found in engine. Clearing selection." << std::endl;
                m_selectedObject.clear();
                if (m_onObjectSelected) {
                    m_onObjectSelected("");
                }
                return;
            }
        }

        bool changed = false;

        ImGui::Text("Position");
        ImGui::PushItemWidth(180.0f);
        if (ImGui::DragFloat3("##Position", &m_selectedTransform.position.x, 0.1f)) {
            changed = true;
        }

        ImGui::Spacing();

        ImGui::Text("Rotation (degrees)");
        if (ImGui::DragFloat3("##Rotation", &m_selectedTransform.rotation.x, 1.0f, -180.0f, 180.0f)) {
            changed = true;
        }

        ImGui::Spacing();

        ImGui::Text("Scale");
        if (ImGui::DragFloat3("##Scale", &m_selectedTransform.scale.x, 0.1f, 0.01f, 10.0f)) {
            changed = true;
        }
        ImGui::PopItemWidth();

        ImGui::Separator();

        float buttonHeight = std::max(35.0f, ImGui::GetFrameHeight() * 1.5f);

        if (ImGui::Button("Reset Transform", ImVec2(-1, buttonHeight))) {
            m_selectedTransform = TransformData();
            changed = true;
        }

        if (changed && m_engine && !m_selectedObject.empty()) {
            m_engine->setObjectPosition(m_selectedObject, m_selectedTransform.position);
            m_engine->setObjectRotation(m_selectedObject, m_selectedTransform.rotation);
            m_engine->setObjectScale(m_selectedObject, m_selectedTransform.scale);

            if (m_onTransformChanged) {
                m_onTransformChanged(m_selectedObject, m_selectedTransform);
            }
        }

        ImGui::Separator();

        if (ImGui::Button("Delete Object", ImVec2(-1, buttonHeight))) {
            if (m_engine) {
                m_engine->removeModel(m_selectedObject);
                m_selectedObject.clear();
                if (m_onObjectSelected) {
                    m_onObjectSelected("");
                }
            }
        }

        static bool visible = true;
        ImGui::Checkbox("Visible", &visible);
        if (m_engine && !m_selectedObject.empty()) {
            m_engine->setObjectVisible(m_selectedObject, visible);
        }
    }

    void EditorUI::drawTextureSection() {
        ImGui::Text("Selected: %s", m_selectedObject.c_str());
        ImGui::Separator();

        if (!m_currentTexturePreview.empty()) {
            ImGui::Text("Current Texture:");
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));

            std::string displayPath = m_currentTexturePreview;
            float maxWidth = ImGui::GetContentRegionAvail().x;
            ImVec2 textSize = ImGui::CalcTextSize(displayPath.c_str());

            if (textSize.x > maxWidth) {
                size_t len = displayPath.length();
                size_t cutPos = static_cast<size_t>(maxWidth / (textSize.x / len)) - 15;
                if (cutPos < len && cutPos > 30) {
                    displayPath = "..." + displayPath.substr(displayPath.length() - cutPos);
                }
            }

            ImGui::TextWrapped("%s", displayPath.c_str());
            ImGui::PopStyleColor();
        }
        else {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No texture assigned");
        }

        ImGui::Separator();

        static char texturePathBuffer[512] = "";

        float availWidth = ImGui::GetContentRegionAvail().x;
        float buttonWidth = 70.0f;
        float inputWidth = availWidth - buttonWidth - 10.0f;

        if (inputWidth < 100.0f) {
            inputWidth = availWidth;
            buttonWidth = 0.0f;
        }

        if (buttonWidth > 0.0f) {
            ImGui::PushItemWidth(inputWidth);
            ImGui::InputText("##TexturePath", texturePathBuffer, sizeof(texturePathBuffer));
            ImGui::PopItemWidth();
            ImGui::SameLine();

            if (ImGui::Button("Browse", ImVec2(buttonWidth, 0))) {
                std::cout << "[UI] Opening texture file dialog..." << std::endl;
                IGFD::FileDialogConfig config;
                config.path = "./textures";
                config.countSelectionMax = 1;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseTextureFile", "Choose Texture File",
                    ".png,.jpg,.jpeg,.bmp,.tga,.dds", config);
            }
        }
        else {
            ImGui::PushItemWidth(availWidth);
            ImGui::InputText("##TexturePath", texturePathBuffer, sizeof(texturePathBuffer));
            ImGui::PopItemWidth();
        }

        ImGui::Spacing();

        if (strlen(texturePathBuffer) > 0) {
            strcpy_s(m_texturePath, sizeof(m_texturePath), texturePathBuffer);
        }

        float buttonFullWidth = ImGui::GetContentRegionAvail().x;

        if (ImGui::Button("Apply Texture", ImVec2(buttonFullWidth, 35))) {
            if (strlen(m_texturePath) > 0) {
                std::cout << "[UI] Applying texture to '" << m_selectedObject
                    << "': " << m_texturePath << std::endl;

                if (m_onTextureLoaded) {
                    m_onTextureLoaded(m_selectedObject, m_texturePath);
                }

                m_currentTexturePreview = m_texturePath;
                strcpy_s(texturePathBuffer, sizeof(texturePathBuffer), m_texturePath);
            }
            else {
                std::cerr << "[UI] No texture file selected!" << std::endl;
            }
        }

        ImGui::Spacing();

        if (ImGui::Button("Remove Texture", ImVec2(buttonFullWidth, 30))) {
            std::cout << "[UI] Removing texture from '" << m_selectedObject << "'" << std::endl;
            memset(m_texturePath, 0, sizeof(m_texturePath));
            memset(texturePathBuffer, 0, sizeof(texturePathBuffer));
            m_currentTexturePreview.clear();

            if (m_onTextureLoaded) {
                m_onTextureLoaded(m_selectedObject, "");
            }
        }

        ImGui::Spacing();
        ImGui::TextDisabled("Supported: PNG, JPG, JPEG, BMP, TGA, DDS");

        if (ImGuiFileDialog::Instance()->Display("ChooseTextureFile")) {
            std::cout << "[UI] File dialog displayed" << std::endl;
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

                std::cout << "[UI] Selected texture file: " << filePath << std::endl;

                strcpy_s(m_texturePath, sizeof(m_texturePath), filePath.c_str());
                strcpy_s(texturePathBuffer, sizeof(texturePathBuffer), filePath.c_str());
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }

    void EditorUI::drawCameraSection() {
        float buttonHeight = std::max(35.0f, ImGui::GetFrameHeight() * 1.5f);

        if (ImGui::Button("Reset Camera", ImVec2(-1, buttonHeight))) {
            m_cameraData.position = glm::vec3(5.0f, 5.0f, 5.0f);
            m_cameraData.target = glm::vec3(0.0f, 0.0f, 0.0f);
            m_cameraData.fov = 60.0f;
            if (m_engine) {
                m_engine->setCameraPosition(m_cameraData.position);
                m_engine->setCameraTarget(m_cameraData.target);
                m_engine->setCameraFOV(m_cameraData.fov);
            }
            if (m_onCameraChanged) {
                m_onCameraChanged(m_cameraData);
            }
        }

        ImGui::Separator();

        bool changed = false;

        ImGui::Text("Field of View");
        ImGui::PushItemWidth(180.0f);
        drawFloatControl("fov", m_cameraData.fov, 20.0f, 120.0f);
        if (ImGui::IsItemDeactivatedAfterEdit()) changed = true;
        ImGui::PopItemWidth();

        if (changed && m_engine) {
            m_engine->setCameraFOV(m_cameraData.fov);
            if (m_onCameraChanged) {
                m_onCameraChanged(m_cameraData);
            }
        }
    }

    void EditorUI::drawVec3Control(const std::string& label, glm::vec3& values, float resetValue, float speed) {
        ImGui::PushID(label.c_str());

        float lineHeight = ImGui::GetTextLineHeight();
        float buttonWidth = lineHeight;
        float fieldWidth = (ImGui::GetContentRegionAvail().x - buttonWidth * 3) / 3;

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        if (ImGui::Button("X", ImVec2(buttonWidth, lineHeight))) {
            values.x = resetValue;
        }
        ImGui::PopStyleColor(2);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(fieldWidth);
        ImGui::DragFloat("##X", &values.x, speed);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.9f, 0.3f, 1.0f));
        if (ImGui::Button("Y", ImVec2(buttonWidth, lineHeight))) {
            values.y = resetValue;
        }
        ImGui::PopStyleColor(2);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(fieldWidth);
        ImGui::DragFloat("##Y", &values.y, speed);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.9f, 1.0f));
        if (ImGui::Button("Z", ImVec2(buttonWidth, lineHeight))) {
            values.z = resetValue;
        }
        ImGui::PopStyleColor(2);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(fieldWidth);
        ImGui::DragFloat("##Z", &values.z, speed);

        ImGui::PopID();
    }

    void EditorUI::drawFloatControl(const std::string& label, float& value, float min, float max, float speed) {
        ImGui::PushID(label.c_str());

        float labelWidth = ImGui::CalcTextSize(label.c_str()).x + 10.0f;
        ImGui::Text("%s:", label.c_str());
        ImGui::SameLine(labelWidth);
        ImGui::SetNextItemWidth(-1);
        ImGui::DragFloat("##Value", &value, speed, min, max, "%.2f");

        ImGui::PopID();
    }

    void EditorUI::loadObject(const std::string& name, const std::string& filepath) {
        if (m_engine) {
            std::cout << "[UI] loadObject: " << name << " from " << filepath << std::endl;
            if (m_onObjectLoaded) {
                m_onObjectLoaded(name, filepath);
            }
        }
    }

    void EditorUI::setOnObjectLoaded(std::function<void(const std::string&, const std::string&)> callback) {
        m_onObjectLoaded = callback;
    }

    void EditorUI::setOnTransformChanged(std::function<void(const std::string&, const TransformData&)> callback) {
        m_onTransformChanged = callback;
    }

    void EditorUI::setOnCameraChanged(std::function<void(const CameraData&)> callback) {
        m_onCameraChanged = callback;
    }

    void EditorUI::setOnObjectSelected(std::function<void(const std::string&)> callback) {
        m_onObjectSelected = callback;
    }

    void EditorUI::setOnTextureLoaded(std::function<void(const std::string&, const std::string&)> callback) {
        m_onTextureLoaded = callback;
    }

} // namespace UI