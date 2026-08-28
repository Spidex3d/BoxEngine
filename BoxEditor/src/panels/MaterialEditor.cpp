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

    if (!m_preview->Initialize(200, 200))
    {
        BOX_LOG_ERROR("MaterialPreview initialization failed");

        m_preview.reset();
        return false;
    }

	return true;
}

void MaterialEditor::Draw(
    BoxEngine& engine,
    Entity& entity,
    FaceEditController& faceEditController)
{
    // -------------------------------------------------
    // Choose which material the preview should show.
    //
    // Default to the old entity material if there
    // isn't a valid selected face.
    // -------------------------------------------------

    Material* previewMaterial = &entity.GetMaterial();


    if (faceEditController.HasSelectedFace())
    {
        const std::size_t faceIndex =
            faceEditController.GetSelectedFace();

        MeshEditing& mesh =
            entity.GetEditableMesh();

        if (faceIndex <
            mesh.GetFaceCount())
        {
            const EditFace& face =
                mesh.GetFace(faceIndex);

            if (face.materialIndex <
                entity.GetMaterialSlotCount())
            {
                previewMaterial =
                    &entity.GetMaterialSlot(
                        face.materialIndex
                    );
            }
        }
    }


    ImGui::Spacing();


    // -------------------------------------------------
    // Material Preview
    // -------------------------------------------------

    if (m_preview)
    {
        ImGui::TextUnformatted(
            "Material Preview"
        );

        m_preview->Draw(
            *previewMaterial
        );
    }
    else
    {
        ImGui::TextDisabled(
            "Preview unavailable."
        );
    }


    // -------------------------------------------------
    // Face Material controls
    // -------------------------------------------------

    DrawFaceMaterialProperties(
        engine,
        entity,
        faceEditController
    );


    ImGui::Spacing();


    // Keep these old controls for now.
    DrawMaterialProperties(
        engine,
        entity,
        *previewMaterial
    );

    DrawEmissionControls(*previewMaterial);

    DrawTextureProperties(engine, entity, *previewMaterial);
}

void MaterialEditor::DrawMaterialProperties(BoxEngine& engine, Entity& entity, Material& material)
{

    
		// put a imgui image of the material preview here.
		// display a sphere with the material applied to it.

        ImGui::Spacing();
       // ImGui::Text("Editing: %s", entity.GetName().c_str());
        ImGui::Spacing();

        glm::vec4 baseColor = material.GetBaseColor();
        if (ImGui::ColorEdit4("Base Color", &baseColor[0]))
        {
            material.SetBaseColor(baseColor);
        }
        float metallic = material.GetMetallic();
       
        if (ImGui::InputFloat("Metallic", &metallic, 0.01f, 1.0f))
        {
            metallic = std::clamp(metallic, 0.0f, 1.0f);

            material.SetMetallic(metallic);
        }
        float roughness = material.GetRoughness();
        
        if (ImGui::InputFloat("Roughness", &roughness, 0.01f, 1.0f))
        {
            roughness = std::clamp(roughness, 0.0f, 1.0f);

            material.SetRoughness(roughness);
        }
        float alpha = material.GetAlpha();
       
        if (ImGui::InputFloat("Alpha", &alpha, 0.01f, 1.0f))
        {
            alpha = std::clamp(alpha, 0.01f, 1.0f);

            material.SetAlpha(alpha);
        }

        bool useTexture = material.UsesBaseColorTexture();
		// use this to toggle the use of the base color texture in the material. TEMP
        if (ImGui::Checkbox("Use Base Color Texture", &useTexture))
        {
            material.SetUseBaseColorTexture(useTexture);
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
       
        if (ImGui::InputFloat("Emission Strength", &emissionStrength, 0.05f, 0.25f))
        {
            emissionStrength = std::clamp(emissionStrength, 0.01f, 0.500f);

            material.SetEmissionStrength(emissionStrength);
        }
	}
   
}

void MaterialEditor::DrawTextureProperties(BoxEngine& engine, Entity& entity, Material& material)
{
	// do this later, for now we will just draw the material properties and emission controls.
}

void MaterialEditor::DrawFaceMaterialProperties(BoxEngine& engine, Entity& entity, FaceEditController& faceEditController)
{
    
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


    MeshEditing& mesh = entity.GetEditableMesh();


    if (faceIndex >=
        mesh.GetFaceCount())
    {
        ImGui::TextDisabled(
            "Invalid face selection."
        );

        return;
    }
    
    EditFace& face = mesh.GetFace(faceIndex);

	ImGui::Text("Editing: %s", entity.GetName().c_str());   // Name of the entity being edited

	ImGui::Text("Selected Face: %zu", faceIndex);           // Index of the selected face

	ImGui::Text("Material Slot: %zu", face.materialIndex    // Index of the material slot assigned to the selected face
    );


    ImGui::Spacing();


    // =================================================
    // NEW MATERIAL SLOT
    // =================================================
    // [ New ] [ Open ] [ Save ] [ Save As ]
    if (ImGui::Button("Add New Material"))
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


            // -----------------------------------------
            // Make the newly-created material
            // the material for the selected face.
            // -----------------------------------------

            entity.SetFaceMaterial(
                faceIndex,
                newSlot
            );


            // -----------------------------------------
            // Rebuild so the face gets its new
            // materialIndex on the GPU.
            // -----------------------------------------

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
            BOX_LOG_WARNING("Maximum material slots reached");
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Open"))
    {

    }
	ImGui::SameLine();
    if (ImGui::Button("Save"))
    {
    }

    
    ImGui::SameLine();
    if (ImGui::Button("Save As"))
    {

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
    int selectedSlot = static_cast<int>(face.materialIndex);

    std::vector<std::string>slotNames;

    for (std::size_t i = 0;
        i < entity.GetMaterialSlotCount();
        ++i)
    {
        slotNames.push_back(entity.GetMaterialSlot(i).GetName());
    }


    std::vector<const char*>slotItems;

    for (const std::string& name : slotNames)
    {
        slotItems.push_back(name.c_str());
    }

	

	// ############################################ New Material Name Selection #######################
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

    ImGui::SeparatorText("Material Tools");
	// ############################################# Buttons for Material Slot Management ##############
    if (ImGui::Button("Load Texture"))
    {

        const std::string path = FileDialog::OpenTexture();

        if (!path.empty())
        {
            const GLuint textureID =
                engine.LoadTexture(path);

            if (textureID != 0)
            {

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
    ImGui::SameLine();
    if (ImGui::Button("Node Editor"))
    {

		// open a new panel to add a node editor for the material,
        // this will allow the user to create complex materials using a node-based system.
    }

	// ############################################# New Material List Selection #######################

    if (ImGui::ListBox(
        "Material List",
        &selectedSlot,
        slotItems.data(),
        static_cast<int>(
            slotItems.size()
            ),
        4))
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

}

void MaterialEditor::Shutdown()
{
    
    if (m_preview)
    {
        m_preview->Shutdown();
        m_preview.reset();
    }
	
    
}
