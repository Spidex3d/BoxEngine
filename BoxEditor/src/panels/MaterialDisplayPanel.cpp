#include "panels/MaterialDisplayPanel.h"
#include "material/MaterialLibrary.h"
#include "material/MaterialSerializer.h"
#include <material/Material.h>
#include <preview/MaterialPreview.h>
#include <imgui/imgui.h>
#include <imgui\ImGuiAF.h>
#include <miniBoxLog.h>
#include <filesystem>
#include <vector>

#include "entity/Entity.h"
#include <BoxEngine.h>
#include "tools/FaceEditController.h"

MaterialDisplayPanel::~MaterialDisplayPanel() = default;

bool MaterialDisplayPanel::Initialize()
{

   
    
        BOX_LOG_INFO(
            "MaterialDisplayPanel initialized"
        );

        return true;
    
    /*m_materialPreview =
        std::make_unique<MaterialPreview>();

    if (!m_materialPreview->Initialize(128, 128))
    {
        BOX_LOG_ERROR("Material Display preview failed to initialize");

        m_materialPreview.reset();

        return false;
    }

    BOX_LOG_INFO("MaterialDisplayPanel initialized");

    return true;*/
}

void MaterialDisplayPanel::Open()
{
	m_isOpen = true;
}

void MaterialDisplayPanel::Close()
{
	m_isOpen = false;
}

bool MaterialDisplayPanel::IsOpen() const
{
	return m_isOpen;
}

void MaterialDisplayPanel::Draw(
    BoxEngine& engine,
    Entity& entity,
    FaceEditController& faceEditController)
{
    if (!m_isOpen)
    {
        return;
    }

    ImGui::SetNextWindowSize(
        ImVec2(650.0f, 500.0f),
        ImGuiCond_FirstUseEver
    );

    if (!ImGui::Begin(
        "Material Display",
        &m_isOpen,
        ImGuiWindowFlags_NoCollapse))
    {
        ImGui::End();
        return;
    }


    // =================================================
    // TOP TOOLBAR
    // =================================================

    const float tbHeight = 28.0f;

    ImGui::BeginChild(
        "##scene_toolbar",
        ImVec2(
            ImGui::GetContentRegionAvail().x,
            tbHeight
        ),
        false,
        ImGuiWindowFlags_NoDecoration
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_ItemSpacing,
        ImVec2(8, 6)
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_FramePadding,
        ImVec2(6, 4)
    );

    ImGui::PushID("top_Buttons");

    ImGui::PushStyleColor(
        ImGuiCol_Button,
        ImVec4(0.0f, 0.0f, 0.0f, 0.0f)
    );

    ImGui::PushStyleColor(
        ImGuiCol_ButtonHovered,
        ImVec4(0.16f, 0.70f, 0.16f, 1.0f)
    );

    ImGui::PushStyleColor(
        ImGuiCol_ButtonActive,
        ImVec4(0.10f, 0.50f, 0.10f, 1.0f)
    );

    ImGui::PushStyleColor(
        ImGuiCol_Border,
        ImVec4(0.0f, 0.0f, 0.8f, 1.0f)
    );


    const char* items[] =
    {
        "Select Category",
        ICON_FA_FOLDER_OPEN " Glass",
        ICON_FA_FOLDER_OPEN " Metal",
        ICON_FA_FOLDER_OPEN " Plastic",
        ICON_FA_FOLDER_OPEN " Wood",
        ICON_FA_FOLDER_OPEN " Stone Brick Tile",
        ICON_FA_FOLDER_OPEN " Plant Soil",
        ICON_FA_FOLDER_OPEN " Fabric",
        ICON_FA_FOLDER_OPEN " Misc"
    };


    ImGui::SetNextItemWidth(180.0f);

    ImGui::Combo(
        "##MaterialCategory",
        &m_directorySelected,
        items,
        IM_ARRAYSIZE(items)
    );


    ImGui::PopStyleColor(4);
    ImGui::PopID();
    ImGui::PopStyleVar(2);

    ImGui::EndChild();


    // =================================================
    // GET MATERIALS FOR SELECTED CATEGORY
    // =================================================

    std::vector<std::filesystem::path>
        materials;


    switch (m_directorySelected)
    {
    case 1:

        materials =
            MaterialLibrary::GetMaterials(
                MaterialCategory::Glass
            );

        break;


    case 2:

        materials =
            MaterialLibrary::GetMaterials(
                MaterialCategory::Metal
            );

        break;


    case 3:

        materials =
            MaterialLibrary::GetMaterials(
                MaterialCategory::Plastic
            );

        break;


    case 4:

        materials =
            MaterialLibrary::GetMaterials(
                MaterialCategory::Wood
            );

        break;


    case 5:

        materials =
            MaterialLibrary::GetMaterials(
                MaterialCategory::StoneBrickTile
            );

        break;


    case 6:

        materials =
            MaterialLibrary::GetMaterials(
                MaterialCategory::PlantSoil
            );

        break;


    case 7:

        materials =
            MaterialLibrary::GetMaterials(
                MaterialCategory::Fabric
            );

        break;


    case 8:

        materials =
            MaterialLibrary::GetMaterials(
                MaterialCategory::Misc
            );

        break;


    default:

        break;
    }


    ImGui::Text(
        "Found %d materials",
        static_cast<int>(
            materials.size()
            )
    );

    ImGui::Separator();


    // =================================================
    // MATERIAL GRID
    // =================================================

    const float tileWidth = 120.0f;
    const float spacing = 10.0f;

    const float availableWidth =
        ImGui::GetContentRegionAvail().x;


    int columnCount =
        static_cast<int>(
            availableWidth /
            (tileWidth + spacing)
            );


    if (columnCount < 1)
    {
        columnCount = 1;
    }


    if (ImGui::BeginTable(
        "##MaterialGrid",
        columnCount,
        ImGuiTableFlags_SizingStretchSame))
    {
        for (const auto& materialPath :
            materials)
        {
            const std::string materialName =
                materialPath
                .stem()
                .string();


            const std::string previewKey =
                materialPath.string();


            ImGui::TableNextColumn();

            ImGui::PushID(
                previewKey.c_str()
            );


            // =================================================
            // LOAD MATERIAL FOR PREVIEW
            // =================================================

            Material previewMaterial;


            if (MaterialSerializer::Load(
                materialPath.string(),
                previewMaterial))
            {
                // ---------------------------------------------
                // Restore base-color texture.
                // ---------------------------------------------

                const std::string baseColorPath =
                    previewMaterial
                    .GetBaseColorTexturePath();


                if (!baseColorPath.empty())
                {
                    const GLuint textureID =
                        GetOrLoadTexture(
                            engine,
                            baseColorPath
                        );


                    if (textureID != 0)
                    {
                        previewMaterial
                            .SetBaseColorTexture(
                                textureID,
                                baseColorPath
                            );
                    }
                }


                // ---------------------------------------------
                // Restore normal texture for preview.
                // ---------------------------------------------

                const std::string normalPath =
                    previewMaterial
                    .GetNormalTexturePath();


                if (!normalPath.empty())
                {
                    const GLuint textureID =
                        GetOrLoadTexture(
                            engine,
                            normalPath
                        );


                    if (textureID != 0)
                    {
                        previewMaterial
                            .SetNormalTexture(
                                textureID,
                                normalPath
                            );
                    }
                }


                // =================================================
                // GET OR CREATE UNIQUE MATERIAL PREVIEW
                //
                // Every .mbmat file gets its own MaterialPreview
                // and therefore its own framebuffer texture.
                // =================================================

                auto previewIt =
                    m_materialPreviews.find(
                        previewKey
                    );


                if (previewIt ==
                    m_materialPreviews.end())
                {
                    auto newPreview =
                        std::make_unique<
                        MaterialPreview
                        >();


                    if (newPreview->Initialize(
                        128,
                        128))
                    {
                        previewIt =
                            m_materialPreviews.emplace(
                                previewKey,
                                std::move(
                                    newPreview
                                )
                            ).first;
                    }
                }


                // =================================================
                // DRAW MATERIAL IMAGE BUTTON
                // =================================================

                bool materialClicked = false;


                if (previewIt !=
                    m_materialPreviews.end())
                {
                    materialClicked =
                        previewIt
                        ->second
                        ->DrawButton(
                            previewMaterial,
                            ImVec2(
                                100.0f,
                                100.0f
                            )
                        );
                }


                // =================================================
                // APPLY MATERIAL WHEN CLICKED
                // =================================================

                if (materialClicked)
                {
                    // ---------------------------------------------
                    // Make sure a face is selected.
                    // ---------------------------------------------

                    if (!faceEditController
                        .HasSelectedFace())
                    {
                        BOX_LOG_WARNING(
                            "No face selected for material"
                        );
                    }
                    else if (
                        entity
                        .GetMaterialSlotCount()
                        >= 8)
                    {
                        BOX_LOG_WARNING(
                            "Maximum material slots reached"
                        );
                    }
                    else
                    {
                        Material loadedMaterial;


                        if (MaterialSerializer::Load(
                            materialPath.string(),
                            loadedMaterial))
                        {
                            // =====================================
                            // BASE COLOR TEXTURE
                            // =====================================

                            const std::string
                                loadedBaseColorPath =
                                loadedMaterial
                                .GetBaseColorTexturePath();


                            if (!loadedBaseColorPath.empty())
                            {
                                const GLuint textureID =
                                    GetOrLoadTexture(
                                        engine,
                                        loadedBaseColorPath
                                    );


                                if (textureID != 0)
                                {
                                    loadedMaterial
                                        .SetBaseColorTexture(
                                            textureID,
                                            loadedBaseColorPath
                                        );
                                }
                            }


                            // =====================================
                            // NORMAL TEXTURE
                            // =====================================

                            const std::string
                                loadedNormalPath =
                                loadedMaterial
                                .GetNormalTexturePath();


                            if (!loadedNormalPath.empty())
                            {
                                const GLuint textureID =
                                    GetOrLoadTexture(
                                        engine,
                                        loadedNormalPath
                                    );


                                if (textureID != 0)
                                {
                                    loadedMaterial
                                        .SetNormalTexture(
                                            textureID,
                                            loadedNormalPath
                                        );
                                }
                            }


                            // =====================================
                            // CREATE MATERIAL SLOT
                            // =====================================

                            const std::size_t newSlot =
                                entity.AddMaterialSlot(
                                    loadedMaterial
                                );


                            // =====================================
                            // ASSIGN TO SELECTED FACE(S)
                            // =====================================

                            entity
                                .SetSelectedFacesMaterial(
                                    newSlot
                                );


                            // =====================================
                            // REBUILD RENDER MESH
                            // =====================================

                            MeshEditing& mesh =
                                entity
                                .GetEditableMesh();


                            MeshData renderMesh;


                            if (mesh.BuildRenderMesh(
                                renderMesh))
                            {
                                entity
                                    .CreateFromMeshData(
                                        renderMesh
                                    );
                            }


                            BOX_LOG_INFO(
                                "Loaded material "
                                + materialName
                                + " into slot "
                                + std::to_string(
                                    newSlot
                                )
                            );
                        }
                        else
                        {
                            BOX_LOG_ERROR(
                                "Failed to load material: "
                                + materialPath.string()
                            );
                        }
                    }
                }
            }


            // =================================================
            // MATERIAL NAME
            // =================================================

            ImGui::TextWrapped(
                "%s",
                materialName.c_str()
            );


            ImGui::PopID();
        }


        ImGui::EndTable();
    }


    ImGui::End();
}


//void MaterialDisplayPanel::Draw(BoxEngine& engine, Entity& entity, FaceEditController& faceEditController)
//{
//    if (!m_isOpen)
//    {
//        return;
//    }
//
//    ImGui::SetNextWindowSize(ImVec2(650.0f, 500.0f), ImGuiCond_FirstUseEver);
//
//    if (!ImGui::Begin(
//        "Material Display",
//        &m_isOpen,
//        ImGuiWindowFlags_NoCollapse))
//    {
//        ImGui::End();
//        return;
//    }
//
//    // Available size in UI units
//    const float window_width = ImGui::GetContentRegionAvail().x;
//    const float window_height = ImGui::GetContentRegionAvail().y;
//
//    ImVec2 pos = ImGui::GetCursorScreenPos();
//    ImGuiIO& io = ImGui::GetIO();
//
//    int desired_w = static_cast<int>(window_width * io.DisplayFramebufferScale.x);
//    int desired_h = static_cast<int>(window_height * io.DisplayFramebufferScale.y);
//
//
//
//    // ########################################## Top Tool bar ############################################
//    const float tbHeight = 28.0f;
//    ImGui::BeginChild("##scene_toolbar", ImVec2(ImGui::GetContentRegionAvail().x, tbHeight), false, ImGuiWindowFlags_NoDecoration);
//
//    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));
//    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 4));
//
//    ImGui::PushID("top_Buttons");
//
//    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // normal
//    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.16f, 0.70f, 0.16f, 1.0f)); // hover
//    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.50f, 0.10f, 1.0f)); // active/click
//    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.8f, 1.0f)); // active/click
//
//    ImGui::GetStyle().FrameBorderSize = 0.3f; // Add a border to the button
//    ImGui::GetStyle().FrameRounding = 6.0f; // rounded corners of buttons
//
//    // We can add more UI elements here to display material properties, textures, etc.
//    const char* items[] = { "Select Category",
//    ICON_FA_FOLDER_OPEN " Glass",
//    ICON_FA_FOLDER_OPEN " Metal",
//    ICON_FA_FOLDER_OPEN " Plastic",
//    ICON_FA_FOLDER_OPEN " Wood",
//    ICON_FA_FOLDER_OPEN " Stone Brick Tile",
//    ICON_FA_FOLDER_OPEN " Plant Soil",
//    ICON_FA_FOLDER_OPEN " Fabric",
//    ICON_FA_FOLDER_OPEN " Misc" };
//    ImGui::SetNextItemWidth(100.0f);
//    ImGui::SameLine();
//
//    ImGui::SetNextItemWidth(180.0f);
//
//    ImGui::Combo("##MaterialCategory", &m_directorySelected, items, IM_ARRAYSIZE(items));
//
//    ImGui::PopStyleVar(2);
//    ImGui::PopStyleColor(4); // pop all 4 pushed colors has to match top
//    ImGui::PopID();
//    ImGui::EndChild();
//
//
//    // Draw a Table for the buttons 
//
//    std::vector<std::filesystem::path> materials;
//
//    switch (m_directorySelected)
//    {
//    case 1:
//        materials =
//            MaterialLibrary::GetMaterials(
//                MaterialCategory::Glass
//            );
//        break;
//
//    case 2:
//        materials =
//            MaterialLibrary::GetMaterials(
//                MaterialCategory::Metal
//            );
//        break;
//
//    case 3:
//        materials =
//            MaterialLibrary::GetMaterials(
//                MaterialCategory::Plastic
//            );
//        break;
//
//    case 4:
//        materials =
//            MaterialLibrary::GetMaterials(
//                MaterialCategory::Wood
//            );
//        break;
//
//    case 5:
//        materials =
//            MaterialLibrary::GetMaterials(
//                MaterialCategory::StoneBrickTile
//            );
//        break;
//
//    case 6:
//        materials =
//            MaterialLibrary::GetMaterials(
//                MaterialCategory::PlantSoil
//            );
//        break;
//
//    case 7:
//        materials =
//            MaterialLibrary::GetMaterials(
//                MaterialCategory::Fabric
//            );
//        break;
//
//    case 8:
//        materials =
//            MaterialLibrary::GetMaterials(
//                MaterialCategory::Misc
//            );
//        break;
//
//    default:
//        break;
//    }
//
//    ImGui::Text(
//        "Found %d materials",
//        static_cast<int>(
//            materials.size()
//            )
//    );
//
//    ImGui::Separator();
//
//    const float tileWidth = 120.0f;
//    const float tileHeight = 100.0f;
//    const float spacing = 10.0f;
//
//    const float availableWidth = ImGui::GetContentRegionAvail().x;
//
//    int columns = static_cast<int>(availableWidth / (tileWidth + spacing));
//
//    if (columns < 1)
//    {
//        columns = 1;
//    }
//
//
//	// columnCount = screen width / (button width + spacing)
//    int columnCount = columns;
//
//    if (ImGui::BeginTable("##MaterialGrid", columnCount, ImGuiTableFlags_SizingStretchSame))
//    {
//        for (const auto& materialPath : materials)
//        {
//            const std::string materialName =
//                materialPath
//                .stem()
//                .string();
//
//            ImGui::TableNextColumn();
//
//            ImGui::PushID(
//                materialPath
//                .string()
//                .c_str()
//            );
//
//            Material previewMaterial;
//
//            if (MaterialSerializer::Load(
//                materialPath.string(),
//                previewMaterial))
//            {
//                // Restore preview base-color texture.
//                const std::string baseColorPath = previewMaterial.GetBaseColorTexturePath();
//
//                if (!baseColorPath.empty())
//                {
//                    //const GLuint textureID = engine.LoadTexture(baseColorPath);
//
//                    const GLuint textureID = GetOrLoadTexture(engine, baseColorPath);
//
//                    if (textureID != 0)
//                    {
//                        previewMaterial.SetBaseColorTexture(textureID, baseColorPath);
//                    }
//                }
//
//
//                // Draw clickable sphere.
//                if (m_materialPreview && m_materialPreview->DrawButton(
//                        previewMaterial, ImVec2(100.0f, 100.0f)))
//
//                    
//
//                {
//                    // -----------------------------------------
//                    // Make sure a face is selected.
//                    // -----------------------------------------
//
//                    if (!faceEditController.HasSelectedFace())
//                    {
//                        BOX_LOG_WARNING(
//                            "No face selected for material"
//                        );
//
//                        continue;
//                    }
//
//
//                    // -----------------------------------------
//                    // Maximum of 8 material slots for now.
//                    // -----------------------------------------
//
//                    if (entity.GetMaterialSlotCount() >= 8)
//                    {
//                        BOX_LOG_WARNING(
//                            "Maximum material slots reached"
//                        );
//
//                        continue;
//                    }
//
//
//                    // -----------------------------------------
//                    // Load the saved material into a temporary
//                    // Material object.
//                    // -----------------------------------------
//
//                    Material loadedMaterial;
//
//                    if (!MaterialSerializer::Load(
//                        materialPath.string(),
//                        loadedMaterial))
//                    {
//                        BOX_LOG_ERROR(
//                            "Failed to load material: " +
//                            materialPath.string()
//                        );
//
//                        continue;
//                    }
//
//                    // =================================================
//                    // Restore Base Color Texture
//                    // =================================================
//
//                    const std::string baseColorPath = loadedMaterial.GetBaseColorTexturePath();
//
//                    if (!baseColorPath.empty())
//                    {
//                        //const GLuint textureID = engine.LoadTexture(baseColorPath);
//                        const GLuint textureID = GetOrLoadTexture(engine, baseColorPath);
//
//                        if (textureID != 0)
//                        {
//                            loadedMaterial.SetBaseColorTexture(
//                                textureID,
//                                baseColorPath
//                            );
//
//                            BOX_LOG_INFO(
//                                "Loaded base color texture: " +
//                                baseColorPath
//                            );
//                        }
//                    }
//
//
//                    // =================================================
//                    // Restore Normal Texture
//                    // =================================================
//
//                    const std::string normalPath = loadedMaterial.GetNormalTexturePath();
//
//
//                    if (!normalPath.empty())
//                    {
//                        //const GLuint textureID = engine.LoadTexture(normalPath);
//                        const GLuint textureID = GetOrLoadTexture(engine, normalPath);
//
//                        if (textureID != 0)
//                        {
//                            loadedMaterial.SetNormalTexture(
//                                textureID,
//                                normalPath
//                            );
//
//                            BOX_LOG_INFO(
//                                "Loaded normal texture: " +
//                                normalPath
//                            );
//                        }
//                    }
//
//
//                    // -----------------------------------------
//                    // Add it as a NEW material slot.
//                    // -----------------------------------------
//
//                    const std::size_t newSlot =
//                        entity.AddMaterialSlot(
//                            loadedMaterial
//                        );
//
//
//                    // -----------------------------------------
//                    // Assign the new slot to the currently
//                    // selected face(s).
//                    // -----------------------------------------
//
//                    entity.SetSelectedFacesMaterial(
//                        newSlot
//                    );
//
//
//                    // -----------------------------------------
//                    // Rebuild the render mesh so the new
//                    // material index reaches the GPU.
//                    // -----------------------------------------
//
//                    MeshEditing& mesh =
//                        entity.GetEditableMesh();
//
//                    MeshData renderMesh;
//
//                    if (mesh.BuildRenderMesh(
//                        renderMesh))
//                    {
//                        entity.CreateFromMeshData(
//                            renderMesh
//                        );
//                    }
//
//
//                    BOX_LOG_INFO(
//                        "Loaded material "
//                        + materialName
//                        + " into slot "
//                        + std::to_string(newSlot)
//                    );
//                }
//
//            }
//
//            ImGui::TextWrapped("%s", materialName.c_str());
//
//            ImGui::PopID();
//
//        }
//
//        ImGui::EndTable();
//
//
//        //++materialNumber;
//
//    }
//        ImGui::End();
//}

GLuint MaterialDisplayPanel::GetOrLoadTexture(BoxEngine& engine, const std::string& path)
{
    if (path.empty() || path == "none")
    {
        return 0;
    }

    // Have we already loaded it?
    auto it = m_textureCache.find(path);

    if (it != m_textureCache.end())
    {
        return it->second;
    }

    // First time we've seen this texture.
    const GLuint textureID = engine.LoadTexture(path);

    if (textureID != 0)
    {
        m_textureCache[path] = textureID;
    }

    return textureID;
}
