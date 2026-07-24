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

void Material::SetBaseColorTexture(
    const std::shared_ptr<Texture>& texture)
{
    m_baseColorTexture = texture;
}

std::shared_ptr<Texture>
Material::GetBaseColorTexture() const
{
    return m_baseColorTexture;
}


//const glm::vec4& Material::GetBaseColor() const
//{
//	if (m_baseColorTexture)
//	{
//		// If a base color texture is set, we might want to sample the texture to get the base color.
//		// However, since we don't have access to the texture sampling here, we'll just return the base color.
//	}
//
//	return m_baseColor;
//}
//
//void Material::SetBaseColor(const glm::vec4& color)
//{
//	m_baseColor = color;
//}
//
//float Material::GetMetallic() const
//{
//	return 0.0f;
//}
//
//void Material::SetMetallic(float metallic)
//{
//}
//
//float Material::GetRoughness() const
//{
//	return 0.0f;
//}
//
//void Material::SetRoughness(float roughness)
//{
//}
//
//float Material::GetAlpha() const
//{
//	return 0.0f;
//}
//
//void Material::SetAlpha(float alpha)
//{
//}
//
//const glm::vec3& Material::GetEmissionColor() const
//{
//	// TODO: insert return statement here
//
//	return glm::vec3(0.0f);
//}
//
//void Material::SetEmissionColor(const glm::vec3& color)
//{
//}
//
//float Material::GetEmissionStrength() const
//{
//	return 0.0f;
//}
//
//void Material::SetEmissionStrength(float strength)
//{
//}
//
//void Material::SetBaseColorTexture(const std::shared_ptr<Texture>& texture)
//{
//}
//
//std::shared_ptr<Texture> Material::GetBaseColorTexture() const
//{
//	return std::shared_ptr<Texture>();
//}
