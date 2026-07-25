#include "panels/MaterialEditor.h"
#include <imgui\imgui.h>
#include <glm\glm.hpp>
#include <entity\Entity.h>
#include <rendering\Material.h>
#include <preview/MaterialPreview.h>
#include <miniBoxLog.h>

MaterialEditor::MaterialEditor() = default;


MaterialEditor::~MaterialEditor()
{
    
    Shutdown();
}

bool MaterialEditor::Initialize()
{

    BOX_LOG_INFO(
        "MaterialEditor::Initialize called"
    );

    m_preview = std::make_unique<MaterialPreview>();

    if (!m_preview->Initialize(
        200,
        200))
    {
        BOX_LOG_ERROR(
            "MaterialPreview initialization failed"
        );

        m_preview.reset();
        return false;
    }

	return true;
}



void MaterialEditor::Draw(Entity& entity)
{
       

    if (ImGui::CollapsingHeader(
        "Material Editor"))
    {
      
        Material& material = entity.GetMaterial();

        ImGui::Spacing();

       

        if (m_preview)
        {
            ImGui::TextUnformatted(
                "Material Preview"
            );
            m_preview->Draw(material);
        }
        else
        {
            ImGui::TextDisabled(
                "Preview unavailable."
            );
        }

        DrawMaterialProperties(entity);
        DrawEmissionControls(material);
        DrawTextureProperties(entity);
    }

}

void MaterialEditor::DrawMaterialProperties(Entity& entity)
{

    if (ImGui::CollapsingHeader("Material Properties")) // ImGuiTreeNodeFlags_DefaultOpen
    {
		// put a imgui image of the material preview here.
		// display a sphere with the material applied to it.

        ImGui::Spacing();
        ImGui::Text("Textures for: %s", entity.GetName().c_str());
        ImGui::Spacing();

        Material& material = entity.GetMaterial();

      
        glm::vec4 baseColor = material.GetBaseColor();
        if (ImGui::ColorEdit4("Base Color", &baseColor[0]))
        {
            material.SetBaseColor(baseColor);
        }
        float metallic = material.GetMetallic();
        if (ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f))
        {
            material.SetMetallic(metallic);
        }
        float roughness = material.GetRoughness();
        if (ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f))
        {
            material.SetRoughness(roughness);
        }
        float alpha = material.GetAlpha();
        if (ImGui::SliderFloat("Alpha", &alpha, 0.0f, 1.0f))
        {
            material.SetAlpha(alpha);
        }
        
	}
    
}



void MaterialEditor::DrawEmissionControls(Material& material)
{
    ImGui::Spacing();
    if (ImGui::CollapsingHeader("Emission Controls"))
    {
        glm::vec3 emissionColor = material.GetEmissionColor();
        if (ImGui::ColorEdit3("Emission Color", &emissionColor[0]))
        {
            material.SetEmissionColor(emissionColor);
        }
        float emissionStrength = material.GetEmissionStrength();
        if (ImGui::SliderFloat("Emission Strength", &emissionStrength, 0.0f, 10.0f))
        {
            material.SetEmissionStrength(emissionStrength);
        }
	}
   
}

void MaterialEditor::DrawTextureProperties(Entity& entity)
{
	// do this later, for now we will just draw the material properties and emission controls.
}

void MaterialEditor::Shutdown()
{
    
    if (m_preview)
    {
        m_preview->Shutdown();
        m_preview.reset();
    }
	
    
}
