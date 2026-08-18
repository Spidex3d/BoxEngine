#include "panels/MaterialEditor.h"
#include <imgui\imgui.h>
#include <glm\glm.hpp>
#include <entity\Entity.h>
#include <BoxEngine.h>
#include <FileDialog.h>
#include <rendering\Material.h>
#include <preview/MaterialPreview.h>
#include <tools/FaceEditController.h>
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

void MaterialEditor::Draw(BoxEngine& engine, Entity& entity, FaceEditController& faceEditController)
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

        DrawFaceMaterialProperties(engine, entity, faceEditController);

        ImGui::Spacing();

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

void MaterialEditor::DrawFaceMaterialProperties(BoxEngine& engine, Entity& entity, FaceEditController& faceEditController)
{
    if (!ImGui::CollapsingHeader(
        "Face Material",
        ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }


    // -------------------------------------------------
    // No face selected.
    // -------------------------------------------------

    if (!faceEditController.HasSelectedFace())
    {
        ImGui::TextDisabled(
            "Select a face in Material Mode."
        );

        return;
    }


    const std::size_t faceIndex =
        faceEditController.GetSelectedFace();


    MeshEditing& mesh =
        entity.GetEditableMesh();


    if (faceIndex >=
        mesh.GetFaceCount())
    {
        ImGui::TextDisabled(
            "Invalid face selection."
        );

        return;
    }


    EditFace& face =
        mesh.GetFace(faceIndex);


    ImGui::Text(
        "Selected Face: %zu",
        faceIndex
    );

    ImGui::Text(
        "Material Slot: %zu",
        face.materialIndex
    );


    ImGui::Spacing();


    // =================================================
    // NEW MATERIAL SLOT
    // =================================================

    if (ImGui::Button(
        "New Material"))
    {
        // Shader currently supports 8 slots.
        if (entity.GetMaterialSlotCount() < 8)
        {
            Material newMaterial;

            const std::size_t nextSlot =
                entity.GetMaterialSlotCount();

            newMaterial.SetName(
                "Material " +
                std::to_string(nextSlot)
            );

            const std::size_t newSlot =
                entity.AddMaterialSlot(
                    newMaterial
                );

            //const std::size_t newSlot =
            //    entity.AddMaterialSlot(
            //        newMaterial
            //    );

            //// Immediately assign the new slot
            //// to the selected face.
            //entity.SetFaceMaterial(
            //    faceIndex,
            //    newSlot
            //);


            // Material index is stored in
            // MeshVertex, so rebuild render mesh.
            MeshData renderMesh;

            if (mesh.BuildRenderMesh(
                renderMesh))
            {
                entity.CreateFromMeshData(
                    renderMesh
                );
            }

            BOX_LOG_INFO(
                "Created material slot "
                << newSlot
                << " for face "
                << faceIndex
            );
        }
        else
        {
            BOX_LOG_WARNING(
                "Maximum material slots reached"
            );
        }
    }


    ImGui::Spacing();


    // =================================================
    // CURRENT FACE MATERIAL
    // =================================================

    if (face.materialIndex >=
        entity.GetMaterialSlotCount())
    {
        ImGui::TextDisabled(
            "Face has invalid material slot."
        );

        return;
    }
    // #############################################
    int selectedSlot =
        static_cast<int>(
            face.materialIndex
            );

    std::vector<std::string>
        slotNames;

    for (std::size_t i = 0;
        i < entity.GetMaterialSlotCount();
        ++i)
    {
        slotNames.push_back(entity.GetMaterialSlot(i).GetName());
    }


    std::vector<const char*>
        slotItems;

    for (const std::string& name :
        slotNames)
    {
        slotItems.push_back(
            name.c_str()
        );
    }


    if (ImGui::Combo(
        "Material List",
        &selectedSlot,
        slotItems.data(),
        static_cast<int>(
            slotItems.size()
            )))
    {
        entity.SetFaceMaterial(
            faceIndex,
            static_cast<std::size_t>(
                selectedSlot
                )
        );

        MeshData renderMesh;

        if (mesh.BuildRenderMesh(
            renderMesh))
        {
            entity.CreateFromMeshData(
                renderMesh
            );
        }
    }



    // #############################################

    Material& material = entity.GetMaterialSlot(face.materialIndex);

    char materialNameBuffer[128]{};

    strncpy_s(
        materialNameBuffer,
        sizeof(materialNameBuffer),
        material.GetName().c_str(),
        _TRUNCATE
    );

    if (ImGui::InputText(
        "Material ",
        materialNameBuffer,
        sizeof(materialNameBuffer)))
    {
        material.SetName(
            materialNameBuffer
        );
    }

    glm::vec4 color =
        material.GetBaseColor();


    if (ImGui::ColorEdit4(
        "Face Color",
        &color[0]))
    {
        material.SetBaseColor(
            color
        );
    }

}

void MaterialEditor::Shutdown()
{
    
    if (m_preview)
    {
        m_preview->Shutdown();
        m_preview.reset();
    }
	
    
}
