#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>


class Texture;

enum class MaterialType
{
    Standard = 0,
    Glass
};

enum class MaterialCategory
{
    Glass = 0,
    Metal,
    Plastic,
    Wood,
    StoneBrickTile,
    PlantSoil,
    Fabric,
    Misc
};

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
	
    // Texture & Material management
    GLuint GetBaseColorTexture() const;
    
    bool UsesBaseColorTexture() const;
    

    void SetUseBaseColorTexture(bool useTexture);
    
	// Set the base color texture and its source path used in the model save and load process
    void SetBaseColorTexture(GLuint textureID, const std::string& sourcePath);

    const std::string& GetBaseColorTexturePath() const;

    const std::string& GetName() const
    {
        return m_name;
    }

    void SetName(const std::string& name)
    {
        m_name = name;
    }

    // Normal map
    GLuint GetNormalTexture() const;

    bool UsesNormalTexture() const;

    void SetUseNormalTexture(
        bool useTexture
    );

    void SetNormalTexture(
        GLuint textureID,
        const std::string& sourcePath
    );

    const std::string&
        GetNormalTexturePath() const;

    float GetNormalStrength() const;

    void SetNormalStrength(
        float strength
    );

	// Material type
    MaterialType GetType() const;
    void SetType(MaterialType type);

    float GetTransmission() const;
    void SetTransmission(float transmission);

    float GetIOR() const;
    void SetIOR(float ior);

    void SetBaseColorTexturePath(const std::string& sourcePath);

    void SetNormalTexturePath(const std::string& sourcePath);


private:

    std::string m_name = "Material";

	// Material type
    MaterialType m_type = MaterialType::Standard;

    float m_transmission = 0.0f;

    float m_ior = 1.5f;

private:
    glm::vec4 m_baseColor = glm::vec4(1.0f);

    float m_metallic = 0.0f;

    float m_roughness = 0.5f;

    float m_alpha = 1.0f;

    glm::vec3 m_emissionColor = glm::vec3(0.0f);

    float m_emissionStrength = 0.0f;

    // Textures 
    GLuint m_baseColorTexture = 0;

    bool m_useBaseColorTexture = false;

	// Store the source path of the base color texture for saving and loading purposes
    std::string m_baseColorTexturePath;

    // Normal Map
    GLuint m_normalTexture = 0;

    bool m_useNormalTexture = false;

    std::string m_normalTexturePath;

    float m_normalStrength = 1.0f;

	
};