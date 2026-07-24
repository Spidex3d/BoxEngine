#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <string>

class Texture;

class Material
{
public:
    Material() = default;

    const glm::vec4& GetBaseColor() const;
    void SetBaseColor(const glm::vec4& color);

    float GetMetallic() const;
    void SetMetallic(float metallic);

    float GetRoughness() const;
    void SetRoughness(float roughness);

    float GetAlpha() const;
    void SetAlpha(float alpha);

    const glm::vec3& GetEmissionColor() const;
    void SetEmissionColor(const glm::vec3& color);

    float GetEmissionStrength() const;
    void SetEmissionStrength(float strength);

    void SetBaseColorTexture(
        const std::shared_ptr<Texture>& texture);

    std::shared_ptr<Texture>
        GetBaseColorTexture() const;

private:
    glm::vec4 m_baseColor = glm::vec4(1.0f);

    float m_metallic = 0.0f;

    float m_roughness = 0.5f;

    float m_alpha = 1.0f;

    glm::vec3 m_emissionColor = glm::vec3(0.0f);

    float m_emissionStrength = 0.0f;

    std::shared_ptr<Texture> m_baseColorTexture;
};