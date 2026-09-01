#include "lighting/Lighting.h"


void Lighting::SetEnabled(
    bool enabled)
{
    m_enabled = enabled;
}


bool Lighting::IsEnabled() const
{
    return m_enabled;
}


void Lighting::SetType(LightType type)
{
    m_type = type;
}


LightType Lighting::GetType() const
{
    return m_type;
}


void Lighting::SetDirection(const glm::vec3& direction)
{
    m_direction = direction;
}


const glm::vec3&
Lighting::GetDirection() const
{
    return m_direction;
}


void Lighting::SetPosition(const glm::vec3& position)
{
    m_position = position;
}


const glm::vec3&
Lighting::GetPosition() const
{
    return m_position;
}


void Lighting::SetColor(const glm::vec3& color)
{
    m_color = color;
}


const glm::vec3&
Lighting::GetColor() const
{
    return m_color;
}


void Lighting::SetIntensity(float intensity)
{
    m_intensity = intensity;
}


float Lighting::GetIntensity() const
{
    return m_intensity;
}