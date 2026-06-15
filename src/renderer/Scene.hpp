#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "Camera.hpp"
#include "Light.hpp"
#include "../resources/Model.hpp"
#include "../core/SceneObject.hpp"
#include "../utils/Math.hpp"

namespace Renderer {

    struct Frustum {
        Utils::Math::Vec4 planes[6];
    };

    class Scene {
    public:
        Scene() = default;
        ~Scene() = default;

        void addObject(std::shared_ptr<Core::SceneObject> object);
        void addObject(const std::string& name, std::shared_ptr<Resources::Model> model);
        bool removeObject(const std::string& name);
        bool removeObjectByUniqueId(const std::string& uniqueId);
        bool renameObject(const std::string& oldName, const std::string& newName);

        Core::SceneObject* getObject(const std::string& name);
        const Core::SceneObject* getObject(const std::string& name) const;

        Core::SceneObject* getObjectByUniqueId(const std::string& uniqueId);
        const Core::SceneObject* getObjectByUniqueId(const std::string& uniqueId) const;

        std::vector<Core::SceneObject*> getObjectsByName(const std::string& name);
        std::vector<const Core::SceneObject*> getObjectsByName(const std::string& name) const;

        const std::vector<std::shared_ptr<Core::SceneObject>>& getObjects() const { return m_objects; }
        void clear();

        void setCamera(const Camera& camera);
        Camera& getCamera() { return m_camera; }
        const Camera& getCamera() const { return m_camera; }
        void setCameraPosition(const Utils::Math::Vec3& position);
        void setCameraTarget(const Utils::Math::Vec3& target);

        void addLight(const Light& light);
        void addLight(Light&& light);
        void removeLight(size_t index);
        void clearLights();
        const std::vector<Light>& getLights() const { return m_lights; }
        std::vector<Light>& getLights() { return m_lights; }
        void setAmbientColor(const Utils::Math::Vec3& color) { m_ambientColor = color; }
        const Utils::Math::Vec3& getAmbientColor() const { return m_ambientColor; }

        void update(float deltaTime);

        bool isObjectVisible(const Core::SceneObject& object) const;
        std::vector<Core::SceneObject*> getVisibleObjects();
        std::vector<const Core::SceneObject*> getVisibleObjects() const;
        void setFrustumCullingEnabled(bool enabled) { m_frustumCullingEnabled = enabled; }
        bool isFrustumCullingEnabled() const { return m_frustumCullingEnabled; }

    private:
        void updateFrustum();

        std::vector<std::shared_ptr<Core::SceneObject>> m_objects;
        std::unordered_map<std::string, std::shared_ptr<Core::SceneObject>> m_nameToObject;
        std::unordered_map<std::string, std::shared_ptr<Core::SceneObject>> m_uniqueIdToObject;

        Camera m_camera;
        std::vector<Light> m_lights;
        Utils::Math::Vec3 m_ambientColor = Utils::Math::Vec3(0.1f);

        bool m_frustumCullingEnabled = false;
        mutable Frustum m_frustum;
        mutable bool m_frustumDirty = true;
    };

}