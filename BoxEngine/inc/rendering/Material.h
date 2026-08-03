#pragma once
#include <glad/glad.h>
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
	
    // Texture management
    GLuint GetBaseColorTexture() const;
    
    bool UsesBaseColorTexture() const;
    

    void SetUseBaseColorTexture(bool useTexture);
    
	// Set the base color texture and its source path used in the model save and load process
    void SetBaseColorTexture(GLuint textureID, const std::string& sourcePath);

    const std::string& GetBaseColorTexturePath() const;

private:
    glm::vec4 m_baseColor = glm::vec4(1.0f);

    float m_metallic = 0.0f;

    float m_roughness = 0.0f;

    float m_alpha = 1.0f;

    glm::vec3 m_emissionColor = glm::vec3(0.0f);

    float m_emissionStrength = 0.0f;

    // Textures 
    GLuint m_baseColorTexture = 0;

    bool m_useBaseColorTexture = false;

    //std::shared_ptr<Texture> m_baseColorTexture;
	// Store the source path of the base color texture for saving and loading purposes
    std::string m_baseColorTexturePath;


	
};