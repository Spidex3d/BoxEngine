#include "panels/MaterialEditor.h"
#include <imgui\imgui.h>
#include <glm\glm.hpp>
#include <filesystem>
#include <fstream>
#include <utility>
#include <Windows.h>
#include <commdlg.h>
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

namespace
{
    struct BoxTexturesFile
    {
        std::string name;
        std::string albedo;
        std::string normal;
        std::string roughness;
        std::string ao;
        std::string displacement;
        int version = 0;
    };

    std::string TrimMaterialValue(const std::string& value)
    {
        const auto first = value.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return {};
        const auto last = value.find_last_not_of(" \t\r\n");
        return value.substr(first, last - first + 1);
    }

    bool ReadBoxTexturesFile(const std::filesystem::path& filePath,
        BoxTexturesFile& result)
    {
        std::ifstream input(filePath);
        if (!input) return false;

        BoxTexturesFile parsed;
        std::string line;
        while (std::getline(input, line))
        {
            line = TrimMaterialValue(line);
            if (line.empty() || line[0] == '#' || line[0] == ';') continue;

            const auto equals = line.find('=');
            if (equals == std::string::npos) continue;

            const std::string key = TrimMaterialValue(line.substr(0, equals));
            const std::string value = TrimMaterialValue(line.substr(equals + 1));

            if (key == "name")         parsed.name = value;
            else if (key == "albedo")       parsed.albedo = value;
            else if (key == "normal")       parsed.normal = value;
            else if (key == "roughness")    parsed.roughness = value;
            else if (key == "ao")           parsed.ao = value;
            else if (key == "displacement") parsed.displacement = value;
            else if (key == "version")
            {
                try { parsed.version = std::stoi(value); }
                catch (...) { return false; }
            }
        }

        if (parsed.version != 1 || parsed.name.empty() || parsed.albedo.empty())
            return false;

        result = std::move(parsed);
        return true;
    }

    std::filesystem::path ChooseBoxTexturesFile()
    {
        char fileName[MAX_PATH] = {};
        OPENFILENAMEA dialog = {};
        dialog.lStructSize = sizeof(dialog);
        dialog.lpstrFile = fileName;
        dialog.nMaxFile = MAX_PATH;
        dialog.lpstrFilter = "BoxTextures Material (*.mat)\0*.mat\0All Files (*.*)\0*.*\0";
        dialog.nFilterIndex = 1;
        dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
        return GetOpenFileNameA(&dialog) ? std::filesystem::path(fileName)
            : std::filesystem::path{};
    }
}


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
    MaterialEditorAction action = m_action;

    m_action = MaterialEditorAction::None;

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

    if (ImGui::Button("Load BoxTextures Material"))
    {
        const std::filesystem::path matPath = ChooseBoxTexturesFile();
        if (!matPath.empty())
        {
            BoxTexturesFile imported;
            if (!ReadBoxTexturesFile(matPath, imported))
            {
                BOX_LOG_ERROR("Invalid BoxTextures .mat file: " << matPath.string());
            }
            else
            {
                // Filenames in .mat are relative to that file, not the working directory.
                const auto loadMap = [&](const std::string& filename) -> GLuint
                {
                    if (filename.empty()) return 0;
                    const auto path = (matPath.parent_path() / filename).lexically_normal();
                    if (!std::filesystem::is_regular_file(path))
                    {
                        BOX_LOG_WARNING("Missing BoxTextures map: " << path.string());
                        return 0;
                    }
                    return engine.LoadTexture(path.string());
                };

                // Load before changing the current material, so a failed albedo
                // leaves the existing material untouched.
                const GLuint albedoID = loadMap(imported.albedo);
                if (albedoID == 0)
                {
                    BOX_LOG_ERROR("Could not load BoxTextures albedo: " << imported.albedo);
                }
                else
                {
                    material.SetName(imported.name);
                    const auto albedoPath = (matPath.parent_path() / imported.albedo).lexically_normal();
                    material.SetBaseColorTexture(albedoID, albedoPath.string());

                    if (!imported.normal.empty())
                    {
                        const auto normalPath = (matPath.parent_path() / imported.normal).lexically_normal();
                        const GLuint normalID = loadMap(imported.normal);
                        material.SetNormalTexture(normalID, normalID ? normalPath.string() : "");
                    }
                    else material.SetNormalTexture(0, "");

                    if (!imported.roughness.empty())
                    {
                        const auto roughnessPath = (matPath.parent_path() / imported.roughness).lexically_normal();
                        const GLuint roughnessID = loadMap(imported.roughness);
                        material.SetRoughnessTexture(roughnessID, roughnessID ? roughnessPath.string() : "");
                        material.SetUseRoughnessTexture(roughnessID != 0);
                    }
                    else
                    {
                        material.SetRoughnessTexture(0, "");
                        material.SetUseRoughnessTexture(false);
                    }

                    // Ambient Occlusion map: store now, shader binding comes next.
                    if (!imported.ao.empty())
                    {
                        const auto aoPath = (matPath.parent_path() / imported.ao).lexically_normal();
                        const GLuint aoID = loadMap(imported.ao);
                        material.SetAOTexture(aoID, aoID ? aoPath.string() : "");
                    }
                    else material.SetAOTexture(0, "");

                    BOX_LOG_INFO("Imported BoxTextures material: " << imported.name);
                    if (!imported.displacement.empty())
                        BOX_LOG_INFO("Displacement entry parsed; renderer support is not yet implemented");
                }
            }
        }
    }

    // =================================================
    // Base Color Buttons for Material Slot Management
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
    // ------------------------------------------------
    // add a button to load a roughness map, this will be used to control the roughness of the material, we will implement this next.
    // ------------------------------------------------
    if (ImGui::Button("Load Roughness Map"))
    {
        const std::string path =
            FileDialog::OpenTexture();

        if (!path.empty())
        {
            const GLuint textureID =
                engine.LoadTexture(path);

            if (textureID != 0)
            {
                material.SetRoughnessTexture(
                    textureID,
                    path
                );

                material.SetUseRoughnessTexture(
                    true
                );

                BOX_LOG_INFO(
                    "Loaded roughness map: "
                    << path
                );
            }
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Map UVs"))
    {
        m_action = MaterialEditorAction::OpenUVPanel;

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

    // roughness map selection

    bool useRoughnessMap =
        material.UsesRoughnessTexture();

    if (ImGui::Checkbox(
        "Use Roughness Map",
        &useRoughnessMap))
    {
        material.SetUseRoughnessTexture(
            useRoughnessMap
        );
    }

    // Ambient Occlusion controls for the selected material slot.
    bool useAOMap = material.UsesAOTexture();
    if (ImGui::Checkbox("Use AO Map", &useAOMap))
    {
        material.SetUseAOTexture(useAOMap);
    }

    float aoStrength = material.GetAOStrength();
    if (ImGui::SliderFloat("AO Strength", &aoStrength, 0.0f, 1.0f, "%.2f"))
    {
        material.SetAOStrength(aoStrength);
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

