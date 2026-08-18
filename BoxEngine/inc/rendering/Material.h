#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

//The roadmap
//
//I would build this in this order:
//
//Add materialIndex to EditFace.
//Let an Entity own multiple Material slots.
//Add a simple Assign Material to Selected Face function.
//Get different colours on different faces.
//Make Extrude / Inset / LoopCut / Bevel preserve face material assignments.
//Extend rendering to proper per - material draw ranges.
//Assign different textures to individual materials / faces.
//Improve UV generation / unwrap.
//Add texture painting.


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

private:

    std::string m_name = "Material";

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