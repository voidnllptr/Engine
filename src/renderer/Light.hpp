#pragma once

#include <glm/glm.hpp>

namespace Renderer {

    enum class LightType {
        Directional,
        Point,
        Spot
    };

    struct Light {
        glm::vec3 position = glm::vec3(0.0f, 5.0f, 0.0f);
        glm::vec3 color = glm::vec3(1.0f);
        float intensity = 1.0f;

        glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);

        float constant = 1.0f;
        float linear = 0.09f;
        float quadratic = 0.032f;

        float innerCutoff = 12.0f;
        float outerCutoff = 17.0f;

        LightType type = LightType::Point;

        static Light createDirectional(const glm::vec3& direction, const glm::vec3& color, float intensity = 1.0f) {
            Light light;
            light.type = LightType::Directional;
            light.direction = glm::normalize(direction);
            light.color = color;
            light.intensity = intensity;
            return light;
        }

        static Light createPoint(const glm::vec3& position, const glm::vec3& color,
            float intensity = 1.0f, float constant = 1.0f,
            float linear = 0.09f, float quadratic = 0.032f) {
            Light light;
            light.type = LightType::Point;
            light.position = position;
            light.color = color;
            light.intensity = intensity;
            light.constant = constant;
            light.linear = linear;
            light.quadratic = quadratic;
            return light;
        }

        static Light createSpot(const glm::vec3& position, const glm::vec3& direction,
            const glm::vec3& color, float intensity = 1.0f,
            float innerCutoff = 12.0f, float outerCutoff = 17.0f) {
            Light light;
            light.type = LightType::Spot;
            light.position = position;
            light.direction = glm::normalize(direction);
            light.color = color;
            light.intensity = intensity;
            light.innerCutoff = innerCutoff;
            light.outerCutoff = outerCutoff;
            return light;
        }

        float getInnerCutoffRad() const { return glm::radians(innerCutoff); }
        float getOuterCutoffRad() const { return glm::radians(outerCutoff); }

        float getAttenuation(float distance) const {
            return 1.0f / (constant + linear * distance + quadratic * distance * distance);
        }
    };

}