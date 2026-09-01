#pragma once
#include <glm/glm.hpp>

enum class LightType
{
    Directional,
    Point,
    Spot
};

class Lighting
{
public:

    Lighting() = default;

    void SetEnabled(bool enabled);
    bool IsEnabled() const;

    void SetType(LightType type);
    LightType GetType() const;

    void SetDirection(const glm::vec3& direction);

    const glm::vec3& GetDirection() const;

    void SetPosition(const glm::vec3& position);

    const glm::vec3& GetPosition() const;

    void SetColor(const glm::vec3& color);

    const glm::vec3& GetColor() const;

    void SetIntensity(float intensity);
    float GetIntensity() const;


private:

    bool m_enabled = true;

    LightType m_type = LightType::Directional;

    glm::vec3 m_direction =
        glm::vec3(
            -0.4f,
            -1.0f,
            -0.3f
        );

    glm::vec3 m_position =
        glm::vec3(
            0.0f,
            3.0f,
            0.0f
        );

    glm::vec3 m_color = glm::vec3(1.0f);

    float m_intensity = 1.0f;
};
