#include "Rendering/Material.h"

#include <algorithm>

const glm::vec4& Material::GetBaseColor() const
{
    return m_baseColor;
}

void Material::SetBaseColor(
    const glm::vec4& color)
{
    m_baseColor = color;
}

float Material::GetMetallic() const
{
    return m_metallic;
}

void Material::SetMetallic(float metallic)
{
    m_metallic =
        std::clamp(metallic, 0.0f, 1.0f);
}

float Material::GetRoughness() const
{
    return m_roughness;
}

void Material::SetRoughness(float roughness)
{
    m_roughness =
        std::clamp(roughness, 0.0f, 1.0f);
}

float Material::GetAlpha() const
{
    return m_alpha;
}

void Material::SetAlpha(float alpha)
{
    m_alpha =
        std::clamp(alpha, 0.0f, 1.0f);
}

const glm::vec3&
Material::GetEmissionColor() const
{
    return m_emissionColor;
}

void Material::SetEmissionColor(
    const glm::vec3& color)
{
    m_emissionColor = color;
}

float Material::GetEmissionStrength() const
{
    return m_emissionStrength;
}

void Material::SetEmissionStrength(
    float strength)
{
    m_emissionStrength =
        std::max(strength, 0.0f);
}
// Texture management
GLuint Material::GetBaseColorTexture() const
{
    return m_baseColorTexture;
}

bool Material::UsesBaseColorTexture() const
{
    return
        m_useBaseColorTexture &&
        m_baseColorTexture != 0;
}

void Material::SetUseBaseColorTexture(
    bool useTexture)
{
    m_useBaseColorTexture =
        useTexture;
}

void Material::SetBaseColorTexture(GLuint textureID, const std::string& sourcePath)
{
    m_baseColorTexture = textureID;

    m_baseColorTexturePath = sourcePath;

    m_useBaseColorTexture = textureID != 0;
}

const std::string& Material::GetBaseColorTexturePath() const
{
    return m_baseColorTexturePath;
}


