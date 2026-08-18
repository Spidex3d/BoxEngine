#include "panels/MaterialEditor.h"
#include <imgui\imgui.h>
#include <glm\glm.hpp>
#include <entity\Entity.h>
#include <BoxEngine.h>
#include <FileDialog.h>
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

void MaterialEditor::Draw(BoxEngine& engine, Entity& entity)
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

        DrawMaterialProperties(engine, entity);
        DrawEmissionControls(material);
        DrawTextureProperties(entity);
    }

}


void MaterialEditor::DrawMaterialProperties(BoxEngine& engine, Entity& entity)
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

        bool useTexture = material.UsesBaseColorTexture();
		// use this to toggle the use of the base color texture in the material. TEMP
        if (ImGui::Checkbox("Use Base Color Texture", &useTexture))
        {
            material.SetUseBaseColorTexture(useTexture);
        }
        
        if (ImGui::Button("Load Texture"))
        {
                       
            const std::string path = FileDialog::OpenTexture();
            
            if (!path.empty())
            {
                const GLuint textureID =
                    engine.LoadTexture(path);

                if (textureID != 0)
                {
                    /*material.SetBaseColorTexture(
                        textureID
                    );*/ 
                    material.SetBaseColorTexture(
                        textureID,
                        path
                    );

                    material.SetUseBaseColorTexture(
                        true
                    );
                }
            }
                        
        }
		ImGui::SameLine();
        if (ImGui::Button("Map UVs"))
        {

			// open a panel to map the UVs of the selected entity, this will be a simple UV mapping tool that allows
            // the user to select a texture and map it to the entity's mesh.
            // This will be a simple implementation for now, but can be expanded later.
			// at some point we will need to unwrap the mesh and allow the user to manually adjust the UVs,
            // but for now we will just use a simple planar mapping.
			// we need to make this intuitive and easy to use, Blender seems very complex for UV mapping,
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

void MaterialEditor::DrawFaceMaterialProperties(BoxEngine& engine, Entity& entity)
{


}

void MaterialEditor::Shutdown()
{
    
    if (m_preview)
    {
        m_preview->Shutdown();
        m_preview.reset();
    }
	
    
}
