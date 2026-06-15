#include "Scene.hpp"
#include <algorithm>
#include <iostream>
#include <glm/gtc/quaternion.hpp>

namespace Renderer {

    void Scene::addObject(std::shared_ptr<Core::SceneObject> object) {
        if (!object) return;

        const std::string& name = object->getName();
        const std::string& uniqueId = object->getUniqueId();

        if (m_uniqueIdToObject.find(uniqueId) != m_uniqueIdToObject.end()) {
            std::cerr << "[Scene] Warning: Object with uniqueId '" << uniqueId << "' already exists!" << std::endl;
            return;
        }

        if (m_nameToObject.find(name) != m_nameToObject.end()) {
            std::cout << "[Scene] Note: Multiple objects with name '" << name << "' (uniqueId: " << uniqueId << ")" << std::endl;
        }

        m_nameToObject[name] = object;
        m_uniqueIdToObject[uniqueId] = object;
        m_objects.push_back(object);

        std::cout << "[Scene] Added object: " << name << " (id: " << uniqueId << "), total objects: " << m_objects.size() << std::endl;
    }

    void Scene::addObject(const std::string& name, std::shared_ptr<Resources::Model> model) {
        auto object = std::make_shared<Core::SceneObject>(name, model);
        addObject(object);
    }

    bool Scene::removeObject(const std::string& name) {
        auto it = m_nameToObject.find(name);
        if (it == m_nameToObject.end()) {
            std::cerr << "[Scene] Cannot remove object '" << name << "' - not found!" << std::endl;
            return false;
        }

        auto object = it->second;
        const std::string& uniqueId = object->getUniqueId();

        m_nameToObject.erase(it);
        m_uniqueIdToObject.erase(uniqueId);

        auto vecIt = std::find(m_objects.begin(), m_objects.end(), object);
        if (vecIt != m_objects.end()) {
            m_objects.erase(vecIt);
        }

        std::cout << "[Scene] Removed object: " << name << " (id: " << uniqueId << ")" << std::endl;
        return true;
    }

    bool Scene::removeObjectByUniqueId(const std::string& uniqueId) {
        auto it = m_uniqueIdToObject.find(uniqueId);
        if (it == m_uniqueIdToObject.end()) {
            std::cerr << "[Scene] Cannot remove object with uniqueId '" << uniqueId << "' - not found!" << std::endl;
            return false;
        }

        auto object = it->second;
        const std::string& name = object->getName();

        auto nameIt = m_nameToObject.find(name);
        if (nameIt != m_nameToObject.end() && nameIt->second == object) {
            m_nameToObject.erase(nameIt);
            for (const auto& obj : m_objects) {
                if (obj != object && obj->getName() == name) {
                    m_nameToObject[name] = obj;
                    break;
                }
            }
        }

        m_uniqueIdToObject.erase(it);

        auto vecIt = std::find(m_objects.begin(), m_objects.end(), object);
        if (vecIt != m_objects.end()) {
            m_objects.erase(vecIt);
        }

        std::cout << "[Scene] Removed object: " << name << " (id: " << uniqueId << ")" << std::endl;
        return true;
    }

    bool Scene::renameObject(const std::string& oldName, const std::string& newName) {
        if (oldName == newName) {
            std::cout << "[Scene] Rename skipped: old name equals new name" << std::endl;
            return true;
        }

        auto it = m_nameToObject.find(oldName);
        if (it == m_nameToObject.end()) {
            std::cerr << "[Scene] Cannot rename '" << oldName << "' - not found!" << std::endl;
            return false;
        }

        if (m_nameToObject.find(newName) != m_nameToObject.end()) {
            std::cerr << "[Scene] Cannot rename to '" << newName << "' - name already exists!" << std::endl;
            return false;
        }

        auto object = it->second;

        m_nameToObject.erase(it);

        object->setName(newName);

        m_nameToObject[newName] = object;

        std::cout << "[Scene] Renamed object from '" << oldName << "' to '" << newName << "'" << std::endl;
        return true;
    }

    Core::SceneObject* Scene::getObject(const std::string& name) {
        auto it = m_nameToObject.find(name);
        if (it == m_nameToObject.end()) {
            return nullptr;
        }
        return it->second.get();
    }

    const Core::SceneObject* Scene::getObject(const std::string& name) const {
        auto it = m_nameToObject.find(name);
        if (it == m_nameToObject.end()) {
            return nullptr;
        }
        return it->second.get();
    }

    Core::SceneObject* Scene::getObjectByUniqueId(const std::string& uniqueId) {
        auto it = m_uniqueIdToObject.find(uniqueId);
        if (it == m_uniqueIdToObject.end()) {
            return nullptr;
        }
        return it->second.get();
    }

    const Core::SceneObject* Scene::getObjectByUniqueId(const std::string& uniqueId) const {
        auto it = m_uniqueIdToObject.find(uniqueId);
        if (it == m_uniqueIdToObject.end()) {
            return nullptr;
        }
        return it->second.get();
    }

    std::vector<Core::SceneObject*> Scene::getObjectsByName(const std::string& name) {
        std::vector<Core::SceneObject*> result;
        for (auto& object : m_objects) {
            if (object && object->getName() == name) {
                result.push_back(object.get());
            }
        }
        return result;
    }

    std::vector<const Core::SceneObject*> Scene::getObjectsByName(const std::string& name) const {
        std::vector<const Core::SceneObject*> result;
        for (const auto& object : m_objects) {
            if (object && object->getName() == name) {
                result.push_back(object.get());
            }
        }
        return result;
    }

    void Scene::clear() {
        m_objects.clear();
        m_nameToObject.clear();
        m_uniqueIdToObject.clear();
        m_lights.clear();
        m_ambientColor = Utils::Math::Vec3(0.1f);
        m_frustumDirty = true;
        std::cout << "[Scene] Cleared all objects" << std::endl;
    }

    void Scene::setCamera(const Camera& camera) {
        m_camera = camera;
        m_frustumDirty = true;
    }

    void Scene::setCameraPosition(const Utils::Math::Vec3& position) {
        m_camera.setPosition(position);
        m_frustumDirty = true;
    }

    void Scene::setCameraTarget(const Utils::Math::Vec3& target) {
        Utils::Math::Vec3 direction = glm::normalize(target - m_camera.getPosition());

        float yaw = glm::degrees(atan2(direction.z, direction.x));
        float pitch = glm::degrees(asin(direction.y));

        m_camera.setYaw(yaw);
        m_camera.setPitch(pitch);
        m_frustumDirty = true;
    }

    void Scene::addLight(const Light& light) {
        m_lights.push_back(light);
    }

    void Scene::addLight(Light&& light) {
        m_lights.push_back(std::move(light));
    }

    void Scene::removeLight(size_t index) {
        if (index < m_lights.size()) {
            m_lights.erase(m_lights.begin() + static_cast<ptrdiff_t>(index));
        }
    }

    void Scene::clearLights() {
        m_lights.clear();
    }

    void Scene::update(float deltaTime) {
        if (m_frustumDirty) {
            updateFrustum();
        }
        (void)deltaTime;
    }

    void Scene::updateFrustum() {
        Utils::Math::Mat4 viewMatrix = m_camera.getViewMatrix();
        Utils::Math::Mat4 projMatrix = m_camera.getProjectionMatrix();
        Utils::Math::Mat4 viewProj = projMatrix * viewMatrix;

        Utils::Math::Vec4 row0(viewProj[0][0], viewProj[1][0], viewProj[2][0], viewProj[3][0]);
        Utils::Math::Vec4 row1(viewProj[0][1], viewProj[1][1], viewProj[2][1], viewProj[3][1]);
        Utils::Math::Vec4 row2(viewProj[0][2], viewProj[1][2], viewProj[2][2], viewProj[3][2]);
        Utils::Math::Vec4 row3(viewProj[0][3], viewProj[1][3], viewProj[2][3], viewProj[3][3]);

        auto normalizePlane = [](Utils::Math::Vec4& plane) {
            float length = glm::length(Utils::Math::Vec3(plane.x, plane.y, plane.z));
            if (length > Utils::Math::EPSILON) {
                plane /= length;
            }
            };

        m_frustum.planes[0] = row3 + row0;
        normalizePlane(m_frustum.planes[0]);

        m_frustum.planes[1] = row3 - row0;
        normalizePlane(m_frustum.planes[1]);

        m_frustum.planes[2] = row3 + row1;
        normalizePlane(m_frustum.planes[2]);

        m_frustum.planes[3] = row3 - row1;
        normalizePlane(m_frustum.planes[3]);

        m_frustum.planes[4] = row3 + row2;
        normalizePlane(m_frustum.planes[4]);

        m_frustum.planes[5] = row3 - row2;
        normalizePlane(m_frustum.planes[5]);

        m_frustumDirty = false;
    }

    bool Scene::isObjectVisible(const Core::SceneObject& object) const {
        if (!m_frustumCullingEnabled) {
            return true;
        }

        if (!object.getModel() || !object.isVisible()) {
            return false;
        }

        Utils::Math::Vec3 worldPos = object.getPosition();
        for (int i = 0; i < 6; ++i) {
            const auto& plane = m_frustum.planes[i];
            float distance = plane.x * worldPos.x + plane.y * worldPos.y + plane.z * worldPos.z + plane.w;
            if (distance < 0) {
                return false;
            }
        }

        return true;
    }

    std::vector<Core::SceneObject*> Scene::getVisibleObjects() {
        std::vector<Core::SceneObject*> visible;
        visible.reserve(m_objects.size());

        for (auto& object : m_objects) {
            if (object && isObjectVisible(*object)) {
                visible.push_back(object.get());
            }
        }

        return visible;
    }

    std::vector<const Core::SceneObject*> Scene::getVisibleObjects() const {
        std::vector<const Core::SceneObject*> visible;
        visible.reserve(m_objects.size());

        for (const auto& object : m_objects) {
            if (object && isObjectVisible(*object)) {
                visible.push_back(object.get());
            }
        }

        return visible;
    }

}