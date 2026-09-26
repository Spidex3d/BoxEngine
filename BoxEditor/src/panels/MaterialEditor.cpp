#include "panels/MaterialEditor.h"
#include <imgui\imgui.h>
#include <glm\glm.hpp>
#include <filesystem>
#include <entity\Entity.h>
#include <BoxEngine.h>
#include <FileDialog.h>
#include <material/Material.h>
#include "material/MaterialSerializer.h"
#include <material/MaterialLibrary.h>
#include <Helpers.h>
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


MaterialEditorAction MaterialEditor::GetAction()
{
    MaterialEditorAction action =
        m_action;

    m_action =
        MaterialEditorAction::None;

    return action;
}

void MaterialEditor::DrawMaterialProperties(BoxEngine& engine, Entity& entity, Material& material)
{
    // =================================================
    // SURFACE
    // =================================================

    if (ImGui::CollapsingHeader(
        "Surface",
        ImGuiTreeNodeFlags_DefaultOpen))
    {
        glm::vec4 baseColor =
            material.GetBaseColor();

        if (ImGui::ColorEdit3(
            "Base Color",
            &baseColor[0]))
        {
            material.SetBaseColor(
                baseColor
            );
        }


        float metallic =
            material.GetMetallic();

        if (ImGui::SliderFloat(
            "Metallic",
            &metallic,
            0.0f,
            1.0f))
        {
            material.SetMetallic(
                metallic
            );
        }


        float roughness =
            material.GetRoughness();

        if (ImGui::SliderFloat(
            "Roughness",
            &roughness,
            0.0f,
            1.0f))
        {
            material.SetRoughness(
                roughness
            );
        }


        float alpha =
            material.GetAlpha();

        if (ImGui::SliderFloat(
            "Opacity",
            &alpha,
            0.0f,
            1.0f))
        {
            material.SetAlpha(
                alpha
            );
        }
    }


    // =================================================
    // ADVANCED
    // =================================================

    if (ImGui::CollapsingHeader("Advanced"))
    {
        int materialType =
            static_cast<int>(
                material.GetType()
                );

        const char* materialTypes[] =
        {
            "Standard",
            "Glass"
        };

        if (ImGui::Combo(
            "Material Type",
            &materialType,
            materialTypes,
            IM_ARRAYSIZE(materialTypes)))
        {
            const MaterialType newType =
                static_cast<MaterialType>(
                    materialType
                    );

            material.SetType(
                newType
            );

            if (newType ==
                MaterialType::Glass)
            {
                material.SetMetallic(0.0f);
                material.SetRoughness(0.05f);
                material.SetAlpha(0.20f);
                material.SetTransmission(1.0f);
                material.SetIOR(1.5f);
            }
        }


        if (material.GetType() ==
            MaterialType::Glass)
        {
            float transmission =
                material.GetTransmission();

            if (ImGui::SliderFloat(
                "Transmission",
                &transmission,
                0.0f,
                1.0f))
            {
                material.SetTransmission(
                    transmission
                );
            }


            float ior =
                material.GetIOR();

            if (ImGui::SliderFloat(
                "IOR",
                &ior,
                1.0f,
                2.5f))
            {
                material.SetIOR(
                    ior
                );
            }
        }
    }
}


void MaterialEditor::DrawEmissionControls(Material& material)
{
    if (ImGui::CollapsingHeader("Emission"))
    {
        glm::vec3 emissionColor =
            material.GetEmissionColor();

        if (ImGui::ColorEdit3(
            "Emission Color",
            &emissionColor[0]))
        {
            material.SetEmissionColor(
                emissionColor
            );
        }


        float emissionStrength =
            material.GetEmissionStrength();

        if (ImGui::SliderFloat(
            "Strength",
            &emissionStrength,
            0.0f,
            1.0f))
        {
            material.SetEmissionStrength(
                emissionStrength
            );
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

	ImGui::Text("Active  Face: %zu", faceIndex);           // Index of the selected face

	ImGui::Text("Material Slot: %zu", face.materialIndex    // Index of the material slot assigned to the selected face
    );


    ImGui::Spacing();

    
    // =================================================
    // NEW MATERIAL SLOT
    // =================================================
    // [ New ] [ Open ] [ Save ] 
    ImGui::SeparatorText("Add New Material section");

    if (ImGui::Button("Add New Material"))
    {
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

            entity.SetSelectedFacesMaterial(
                newSlot
            );

            MeshData renderMesh;

            if (mesh.BuildRenderMesh(renderMesh))
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

            // IMPORTANT:
            // Don't use any old references again this frame.
            return;
        }
        else
        {
            BOX_LOG_WARNING("Maximum material slots reached");
        }
    }
    
    // =================================================
    // CURRENT MATERIAL 
    // =================================================

    if (face.materialIndex >=
        entity.GetMaterialSlotCount())
    {
        ImGui::TextDisabled(
            "Face has invalid material slot."
        );

        return;
    }


    // Get the material AFTER AddMaterialSlot()
    // because AddMaterialSlot may reallocate the vector.
    Material& material = entity.GetMaterialSlot(face.materialIndex);


    char materialNameBuffer[128]{};

    strncpy_s(
        materialNameBuffer,
        sizeof(materialNameBuffer),
        material.GetName().c_str(),
        _TRUNCATE
    );

    if (ImGui::InputText(
        "Material Name",
        materialNameBuffer,
        sizeof(materialNameBuffer)))
    {
        material.SetName(
            materialNameBuffer
        );
    }

   
    ImGui::Spacing();
    ImGui::SeparatorText("Material Open - Save Section");

    // ImGui::SameLine();
    if (ImGui::Button("Open material"))
    {
        m_action = MaterialEditorAction::OpenMaterialLibrary;

    }
    ImGui::SameLine();

    if (ImGui::Button("Save material"))
    {
        namespace fs = std::filesystem;

        fs::path materialDirectory =
            MaterialLibrary::GetCategoryPath(
                m_selectedCategory
            );

        fs::path filePath =
            materialDirectory /
            (
                material.GetName() +
                ".mbmat"
                );

        if (MaterialSerializer::Save(
            material,
            filePath.string()))
        {
            BOX_LOG_INFO(
                "Material saved: " +
                filePath.string()
            );
        }
        else
        {
            BOX_LOG_ERROR(
                "Failed to save material: " +
                filePath.string()
            );
        }
    }


    // =================================================
    // Material Library Category
    // =================================================
	//ImGui::SameLine();
    const char* categoryNames[] =
    {
        "Glass",
        "Metal",
        "Plastic",
        "Wood",
        "Stone / Brick / Tile",
        "Plant / Soil",
        "Fabric",
        "Misc"
    };

    int categoryIndex = static_cast<int>(m_selectedCategory);

    if (ImGui::Combo(
        "Category",
        &categoryIndex,
        categoryNames,
        IM_ARRAYSIZE(categoryNames)))
    {
        m_selectedCategory =
            static_cast<MaterialCategory>(
                categoryIndex
                );
    }


    ImGui::Spacing();


    // =================================================
    // CURRENT FACE MATERIAL
    // =================================================

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

	
    ImGui::SeparatorText("Load Material Textures");
	// ############################################# Buttons for Material Slot Management ##############

    // =================================================
    // Base Color
    // =================================================

    if (ImGui::Button("Load Base Color"))
    {

        const std::string path = FileDialog::OpenTexture();

        if (!path.empty())
        {
            const GLuint textureID = engine.LoadTexture(path);

            if (textureID != 0)
            {

                material.SetBaseColorTexture(textureID, path);

                material.SetUseBaseColorTexture(true);

                BOX_LOG_INFO("Loaded base color texture: " << path);
            }
        }

    }

    // =================================================
    // NORMAL MAP
    // =================================================

    ImGui::SameLine();
    if (ImGui::Button("Load Normal Map"))
    {
        const std::string path =
            FileDialog::OpenTexture();

        if (!path.empty())
        {
            const GLuint textureID =
                engine.LoadTexture(path);

            if (textureID != 0)
            {
                material.SetNormalTexture(
                    textureID,
                    path
                );

               // material.SetUseNormalTexture(true);

                BOX_LOG_INFO("Loaded normal map: " << path);
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

    // ================================
	// Normal Selection
	// ================================

    bool useNormalMap =
        material.UsesNormalTexture();

    if (ImGui::Checkbox(
        "Use Normal Map",
        &useNormalMap))
    {
        material.SetUseNormalTexture(
            useNormalMap
        );
    }


    float normalStrength =
        material.GetNormalStrength();

    ImGui::SetNextItemWidth(
        100.0f
    );

    if (ImGui::InputFloat(
        "Normal Strength",
        &normalStrength,
        0.05f,
        0.10f,
        "%.2f"))
    {
        material.SetNormalStrength(
            normalStrength
        );
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
        entity.SetSelectedFacesMaterial(
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
    // shutdown panel if needed
    
    m_preview.reset();
    
}
