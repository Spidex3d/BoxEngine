#include "panels/MaterialDisplayPanel.h"
#include "material/MaterialLibrary.h"
#include "material/MaterialSerializer.h"
#include <material/Material.h>
#include <preview/MaterialPreview.h>
#include <imgui/imgui.h>
#include <imgui\ImGuiAF.h>
#include <miniBoxLog.h>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <vector>
#include <Helpers.h>

#include "entity/Entity.h"
#include <BoxEngine.h>
#include "tools/FaceEditController.h"

namespace
{
    std::string TrimBoxValue(const std::string& s)
    {
        const auto first = s.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return {};
        const auto last = s.find_last_not_of(" \t\r\n");
        return s.substr(first, last - first + 1);
    }

    // Both formats finish as the same engine Material object.
    bool LoadDisplayMaterial(MaterialDisplayPanel& panel, BoxEngine& engine,
        const std::filesystem::path& path, Material& material)
    {
        if (path.extension() == ".mat")
        {
            std::ifstream input(path);
            if (!input) return false;

            std::string name, albedo, normal, roughness, ao, line;
            int version = 0;
            while (std::getline(input, line))
            {
                line = TrimBoxValue(line);
                if (line.empty() || line[0] == '#' || line[0] == ';') continue;
                const auto equals = line.find('=');
                if (equals == std::string::npos) continue;
                const auto key = TrimBoxValue(line.substr(0, equals));
                const auto value = TrimBoxValue(line.substr(equals + 1));
                if (key == "name") name = value;
                else if (key == "albedo") albedo = value;
                else if (key == "normal") normal = value;
                else if (key == "roughness") roughness = value;
                else if (key == "ao") ao = value;
                else if (key == "version")
                {
                    try { version = std::stoi(value); }
                    catch (...) { return false; }
                }
                // Displacement is reserved for later engine support.
            }
            if (version != 1 || name.empty() || albedo.empty()) return false;

            const auto absoluteMap = [&](const std::string& filename)
            {
                return (path.parent_path() / filename).lexically_normal().string();
            };
            const std::string basePath = absoluteMap(albedo);
            const GLuint baseID = panel.GetOrLoadTexture(engine, basePath);
            if (baseID == 0) return false;

            material.SetName(name);
            material.SetBaseColorTexture(baseID, basePath);

            if (!normal.empty())
            {
                const auto normalPath = absoluteMap(normal);
                const GLuint normalID = panel.GetOrLoadTexture(engine, normalPath);
                if (normalID) material.SetNormalTexture(normalID, normalPath);
            }
            if (!roughness.empty())
            {
                const auto roughnessPath = absoluteMap(roughness);
                const GLuint roughnessID = panel.GetOrLoadTexture(engine, roughnessPath);
                if (roughnessID)
                {
                    material.SetRoughnessTexture(roughnessID, roughnessPath);
                    material.SetUseRoughnessTexture(true);
                }
            }
            if (!ao.empty())
            {
                const auto aoPath = absoluteMap(ao);
                const GLuint aoID = panel.GetOrLoadTexture(engine, aoPath);
                if (aoID) material.SetAOTexture(aoID, aoPath);
            }
            return true;
        }

        if (!MaterialSerializer::Load(path.string(), material)) return false;

        const auto loadMap = [&](const std::string& filename) -> GLuint
        {
            return filename.empty() ? 0 : panel.GetOrLoadTexture(engine, filename);
        };
        const auto basePath = material.GetBaseColorTexturePath();
        const auto normalPath = material.GetNormalTexturePath();
        const auto roughnessPath = material.GetRoughnessTexturePath();
        const auto aoPath = material.GetAOTexturePath();
        const bool aoEnabled = material.IsAOTextureEnabled();
        if (const GLuint id = loadMap(basePath)) material.SetBaseColorTexture(id, basePath);
        if (const GLuint id = loadMap(normalPath)) material.SetNormalTexture(id, normalPath);
        if (const GLuint id = loadMap(roughnessPath))
        {
            material.SetRoughnessTexture(id, roughnessPath);
            material.SetUseRoughnessTexture(true);
        }
        if (const GLuint id = loadMap(aoPath))
        {
            material.SetAOTexture(id, aoPath);
            material.SetUseAOTexture(aoEnabled);
        }
        return true;
    }
}

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


    ImGui::SetNextItemWidth(165.0f);
    ImGui::Combo("##MaterialSource", &m_materialSource,
        "BoxEditor (.mbmat)\0BoxTextures (.mat)\0");
    ImGui::SameLine();
    if (m_materialSource == 0)
    {
        ImGui::SetNextItemWidth(180.0f);
        ImGui::Combo("##MaterialCategory", &m_directorySelected,
            items, IM_ARRAYSIZE(items));
    }
    else
    {
        const char* pbrCategories[] =
        {
            "All PBR Materials",
            ICON_FA_FOLDER_OPEN " Bricks",
            ICON_FA_FOLDER_OPEN " Dirt",
            ICON_FA_FOLDER_OPEN " Rocks",
            ICON_FA_FOLDER_OPEN " Grass",
            ICON_FA_FOLDER_OPEN " Pavements"
        };

        ImGui::SetNextItemWidth(185.0f);
        ImGui::Combo("##PBRCategory", &m_pbrCategorySelected,
            pbrCategories, IM_ARRAYSIZE(pbrCategories));
    }


    ImGui::PopStyleColor(4);
    ImGui::PopID();
    ImGui::PopStyleVar(2);

    ImGui::EndChild();


    // =================================================
    // GET MATERIALS FOR SELECTED CATEGORY
    // =================================================

    std::vector<std::filesystem::path>
        materials;


    if (m_materialSource == 1)
    {
        Helpers helpers;
        const std::filesystem::path pbrFolder = helpers.GetAssetPath("assets/PBR");
        const char* folderNames[] =
        {
            "", "Bricks", "Dirt", "Rocks", "Grass", "Pavements"
        };

        // "All" searches PBR; a chosen category searches only its folder.
        const std::filesystem::path searchFolder =
            m_pbrCategorySelected == 0
            ? pbrFolder
            : pbrFolder / folderNames[m_pbrCategorySelected];

        std::error_code ec;
        if (std::filesystem::is_directory(searchFolder, ec))
        {
            std::filesystem::recursive_directory_iterator it(
                searchFolder, std::filesystem::directory_options::skip_permission_denied, ec);
            const std::filesystem::recursive_directory_iterator end;
            for (; it != end; it.increment(ec))
            {
                if (ec) { ec.clear(); continue; }
                if (it->is_regular_file(ec) && it->path().extension() == ".mat")
                    materials.push_back(it->path());
            }
            std::sort(materials.begin(), materials.end());
        }
    }
    else switch (m_directorySelected)
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


            if (LoadDisplayMaterial(*this, engine, materialPath, previewMaterial))
            {
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
                        Material loadedMaterial = previewMaterial;
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





//#include "panels/MaterialDisplayPanel.h"
//#include "material/MaterialLibrary.h"
//#include "material/MaterialSerializer.h"
//#include <material/Material.h>
//#include <preview/MaterialPreview.h>
//#include <imgui/imgui.h>
//#include <imgui\ImGuiAF.h>
//#include <miniBoxLog.h>
//#include <filesystem>
//#include <fstream>
//#include <algorithm>
//#include <vector>
//#include <Helpers.h>
//
//#include "entity/Entity.h"
//#include <BoxEngine.h>
//#include "tools/FaceEditController.h"
//
//namespace
//{
//    std::string TrimBoxValue(const std::string& s)
//    {
//        const auto first = s.find_first_not_of(" \t\r\n");
//        if (first == std::string::npos) return {};
//        const auto last = s.find_last_not_of(" \t\r\n");
//        return s.substr(first, last - first + 1);
//    }
//
//    // Both formats finish as the same engine Material object.
//    bool LoadDisplayMaterial(MaterialDisplayPanel& panel, BoxEngine& engine,
//        const std::filesystem::path& path, Material& material)
//    {
//        if (path.extension() == ".mat")
//        {
//            std::ifstream input(path);
//            if (!input) return false;
//
//            std::string name, albedo, normal, roughness, ao, line;
//            int version = 0;
//            while (std::getline(input, line))
//            {
//                line = TrimBoxValue(line);
//                if (line.empty() || line[0] == '#' || line[0] == ';') continue;
//                const auto equals = line.find('=');
//                if (equals == std::string::npos) continue;
//                const auto key = TrimBoxValue(line.substr(0, equals));
//                const auto value = TrimBoxValue(line.substr(equals + 1));
//                if (key == "name") name = value;
//                else if (key == "albedo") albedo = value;
//                else if (key == "normal") normal = value;
//                else if (key == "roughness") roughness = value;
//                else if (key == "ao") ao = value;
//                else if (key == "version")
//                {
//                    try { version = std::stoi(value); }
//                    catch (...) { return false; }
//                }
//                // Displacement is reserved for later engine support.
//            }
//            if (version != 1 || name.empty() || albedo.empty()) return false;
//
//            const auto absoluteMap = [&](const std::string& filename)
//            {
//                return (path.parent_path() / filename).lexically_normal().string();
//            };
//            const std::string basePath = absoluteMap(albedo);
//            const GLuint baseID = panel.GetOrLoadTexture(engine, basePath);
//            if (baseID == 0) return false;
//
//            material.SetName(name);
//            material.SetBaseColorTexture(baseID, basePath);
//
//            if (!normal.empty())
//            {
//                const auto normalPath = absoluteMap(normal);
//                const GLuint normalID = panel.GetOrLoadTexture(engine, normalPath);
//                if (normalID) material.SetNormalTexture(normalID, normalPath);
//            }
//            if (!roughness.empty())
//            {
//                const auto roughnessPath = absoluteMap(roughness);
//                const GLuint roughnessID = panel.GetOrLoadTexture(engine, roughnessPath);
//                if (roughnessID)
//                {
//                    material.SetRoughnessTexture(roughnessID, roughnessPath);
//                    material.SetUseRoughnessTexture(true);
//                }
//            }
//            if (!ao.empty())
//            {
//                const auto aoPath = absoluteMap(ao);
//                const GLuint aoID = panel.GetOrLoadTexture(engine, aoPath);
//                if (aoID) material.SetAOTexture(aoID, aoPath);
//            }
//            return true;
//        }
//
//        if (!MaterialSerializer::Load(path.string(), material)) return false;
//
//        const auto loadMap = [&](const std::string& filename) -> GLuint
//        {
//            return filename.empty() ? 0 : panel.GetOrLoadTexture(engine, filename);
//        };
//        const auto basePath = material.GetBaseColorTexturePath();
//        const auto normalPath = material.GetNormalTexturePath();
//        const auto roughnessPath = material.GetRoughnessTexturePath();
//        if (const GLuint id = loadMap(basePath)) material.SetBaseColorTexture(id, basePath);
//        if (const GLuint id = loadMap(normalPath)) material.SetNormalTexture(id, normalPath);
//        if (const GLuint id = loadMap(roughnessPath))
//        {
//            material.SetRoughnessTexture(id, roughnessPath);
//            material.SetUseRoughnessTexture(true);
//        }
//        return true;
//    }
//}
//
//MaterialDisplayPanel::~MaterialDisplayPanel() = default;
//
//bool MaterialDisplayPanel::Initialize()
//{
//
//
//
//    BOX_LOG_INFO(
//        "MaterialDisplayPanel initialized"
//    );
//
//    return true;
//
//    /*m_materialPreview =
//        std::make_unique<MaterialPreview>();
//
//    if (!m_materialPreview->Initialize(128, 128))
//    {
//        BOX_LOG_ERROR("Material Display preview failed to initialize");
//
//        m_materialPreview.reset();
//
//        return false;
//    }
//
//    BOX_LOG_INFO("MaterialDisplayPanel initialized");
//
//    return true;*/
//}
//
//void MaterialDisplayPanel::Open()
//{
//    m_isOpen = true;
//}
//
//void MaterialDisplayPanel::Close()
//{
//    m_isOpen = false;
//}
//
//bool MaterialDisplayPanel::IsOpen() const
//{
//    return m_isOpen;
//}
//
//void MaterialDisplayPanel::Draw(
//    BoxEngine& engine,
//    Entity& entity,
//    FaceEditController& faceEditController)
//{
//    if (!m_isOpen)
//    {
//        return;
//    }
//
//    ImGui::SetNextWindowSize(
//        ImVec2(650.0f, 500.0f),
//        ImGuiCond_FirstUseEver
//    );
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
//
//    // =================================================
//    // TOP TOOLBAR
//    // =================================================
//
//    const float tbHeight = 28.0f;
//
//    ImGui::BeginChild(
//        "##scene_toolbar",
//        ImVec2(
//            ImGui::GetContentRegionAvail().x,
//            tbHeight
//        ),
//        false,
//        ImGuiWindowFlags_NoDecoration
//    );
//
//    ImGui::PushStyleVar(
//        ImGuiStyleVar_ItemSpacing,
//        ImVec2(8, 6)
//    );
//
//    ImGui::PushStyleVar(
//        ImGuiStyleVar_FramePadding,
//        ImVec2(6, 4)
//    );
//
//    ImGui::PushID("top_Buttons");
//
//    ImGui::PushStyleColor(
//        ImGuiCol_Button,
//        ImVec4(0.0f, 0.0f, 0.0f, 0.0f)
//    );
//
//    ImGui::PushStyleColor(
//        ImGuiCol_ButtonHovered,
//        ImVec4(0.16f, 0.70f, 0.16f, 1.0f)
//    );
//
//    ImGui::PushStyleColor(
//        ImGuiCol_ButtonActive,
//        ImVec4(0.10f, 0.50f, 0.10f, 1.0f)
//    );
//
//    ImGui::PushStyleColor(
//        ImGuiCol_Border,
//        ImVec4(0.0f, 0.0f, 0.8f, 1.0f)
//    );
//
//
//    const char* items[] =
//    {
//        "Select Category",
//        ICON_FA_FOLDER_OPEN " Glass",
//        ICON_FA_FOLDER_OPEN " Metal",
//        ICON_FA_FOLDER_OPEN " Plastic",
//        ICON_FA_FOLDER_OPEN " Wood",
//        ICON_FA_FOLDER_OPEN " Stone Brick Tile",
//        ICON_FA_FOLDER_OPEN " Plant Soil",
//        ICON_FA_FOLDER_OPEN " Fabric",
//        ICON_FA_FOLDER_OPEN " Misc"
//    };
//
//
//    ImGui::SetNextItemWidth(165.0f);
//    ImGui::Combo("##MaterialSource", &m_materialSource,
//        "BoxEditor (.mbmat)\0BoxTextures (.mat)\0");
//    ImGui::SameLine();
//    if (m_materialSource == 0)
//    {
//        ImGui::SetNextItemWidth(180.0f);
//        ImGui::Combo("##MaterialCategory", &m_directorySelected,
//            items, IM_ARRAYSIZE(items));
//    }
//    else
//    {
//        const char* pbrCategories[] =
//        {
//            "All PBR Materials",
//            ICON_FA_FOLDER_OPEN " Bricks",
//            ICON_FA_FOLDER_OPEN " Dirt",
//            ICON_FA_FOLDER_OPEN " Rocks",
//            ICON_FA_FOLDER_OPEN " Grass",
//            ICON_FA_FOLDER_OPEN " Pavements"
//        };
//
//        ImGui::SetNextItemWidth(185.0f);
//        ImGui::Combo("##PBRCategory", &m_pbrCategorySelected,
//            pbrCategories, IM_ARRAYSIZE(pbrCategories));
//    }
//
//
//    ImGui::PopStyleColor(4);
//    ImGui::PopID();
//    ImGui::PopStyleVar(2);
//
//    ImGui::EndChild();
//
//
//    // =================================================
//    // GET MATERIALS FOR SELECTED CATEGORY
//    // =================================================
//
//    std::vector<std::filesystem::path>
//        materials;
//
//
//    if (m_materialSource == 1)
//    {
//        Helpers helpers;
//        const std::filesystem::path pbrFolder = helpers.GetAssetPath("assets/PBR");
//        const char* folderNames[] =
//        {
//            "", "Bricks", "Dirt", "Rocks", "Grass", "Pavements"
//        };
//
//        // "All" searches PBR; a chosen category searches only its folder.
//        const std::filesystem::path searchFolder =
//            m_pbrCategorySelected == 0
//            ? pbrFolder
//            : pbrFolder / folderNames[m_pbrCategorySelected];
//
//        std::error_code ec;
//        if (std::filesystem::is_directory(searchFolder, ec))
//        {
//            std::filesystem::recursive_directory_iterator it(
//                searchFolder, std::filesystem::directory_options::skip_permission_denied, ec);
//            const std::filesystem::recursive_directory_iterator end;
//            for (; it != end; it.increment(ec))
//            {
//                if (ec) { ec.clear(); continue; }
//                if (it->is_regular_file(ec) && it->path().extension() == ".mat")
//                    materials.push_back(it->path());
//            }
//            std::sort(materials.begin(), materials.end());
//        }
//    }
//    else switch (m_directorySelected)
//    {
//    case 1:
//
//        materials =
//            MaterialLibrary::GetMaterials(
//                MaterialCategory::Glass
//            );
//
//        break;
//
//
//    case 2:
//
//        materials =
//            MaterialLibrary::GetMaterials(
//                MaterialCategory::Metal
//            );
//
//        break;
//
//
//    case 3:
//
//        materials =
//            MaterialLibrary::GetMaterials(
//                MaterialCategory::Plastic
//            );
//
//        break;
//
//
//    case 4:
//
//        materials =
//            MaterialLibrary::GetMaterials(
//                MaterialCategory::Wood
//            );
//
//        break;
//
//
//    case 5:
//
//        materials =
//            MaterialLibrary::GetMaterials(
//                MaterialCategory::StoneBrickTile
//            );
//
//        break;
//
//
//    case 6:
//
//        materials =
//            MaterialLibrary::GetMaterials(
//                MaterialCategory::PlantSoil
//            );
//
//        break;
//
//
//    case 7:
//
//        materials =
//            MaterialLibrary::GetMaterials(
//                MaterialCategory::Fabric
//            );
//
//        break;
//
//
//    case 8:
//
//        materials =
//            MaterialLibrary::GetMaterials(
//                MaterialCategory::Misc
//            );
//
//        break;
//
//
//    default:
//
//        break;
//    }
//
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
//
//    // =================================================
//    // MATERIAL GRID
//    // =================================================
//
//    const float tileWidth = 120.0f;
//    const float spacing = 10.0f;
//
//    const float availableWidth =
//        ImGui::GetContentRegionAvail().x;
//
//
//    int columnCount =
//        static_cast<int>(
//            availableWidth /
//            (tileWidth + spacing)
//            );
//
//
//    if (columnCount < 1)
//    {
//        columnCount = 1;
//    }
//
//
//    if (ImGui::BeginTable(
//        "##MaterialGrid",
//        columnCount,
//        ImGuiTableFlags_SizingStretchSame))
//    {
//        for (const auto& materialPath :
//            materials)
//        {
//            const std::string materialName =
//                materialPath
//                .stem()
//                .string();
//
//
//            const std::string previewKey =
//                materialPath.string();
//
//
//            ImGui::TableNextColumn();
//
//            ImGui::PushID(
//                previewKey.c_str()
//            );
//
//
//            // =================================================
//            // LOAD MATERIAL FOR PREVIEW
//            // =================================================
//
//            Material previewMaterial;
//
//
//            if (LoadDisplayMaterial(*this, engine, materialPath, previewMaterial))
//            {
//                // =================================================
//                // GET OR CREATE UNIQUE MATERIAL PREVIEW
//                //
//                // Every .mbmat file gets its own MaterialPreview
//                // and therefore its own framebuffer texture.
//                // =================================================
//
//                auto previewIt =
//                    m_materialPreviews.find(
//                        previewKey
//                    );
//
//
//                if (previewIt ==
//                    m_materialPreviews.end())
//                {
//                    auto newPreview =
//                        std::make_unique<
//                        MaterialPreview
//                        >();
//
//
//                    if (newPreview->Initialize(
//                        128,
//                        128))
//                    {
//                        previewIt =
//                            m_materialPreviews.emplace(
//                                previewKey,
//                                std::move(
//                                    newPreview
//                                )
//                            ).first;
//                    }
//                }
//
//
//                // =================================================
//                // DRAW MATERIAL IMAGE BUTTON
//                // =================================================
//
//                bool materialClicked = false;
//
//
//                if (previewIt !=
//                    m_materialPreviews.end())
//                {
//                    materialClicked =
//                        previewIt
//                        ->second
//                        ->DrawButton(
//                            previewMaterial,
//                            ImVec2(
//                                100.0f,
//                                100.0f
//                            )
//                        );
//                }
//
//
//                // =================================================
//                // APPLY MATERIAL WHEN CLICKED
//                // =================================================
//
//                if (materialClicked)
//                {
//                    // ---------------------------------------------
//                    // Make sure a face is selected.
//                    // ---------------------------------------------
//
//                    if (!faceEditController
//                        .HasSelectedFace())
//                    {
//                        BOX_LOG_WARNING(
//                            "No face selected for material"
//                        );
//                    }
//                    else if (
//                        entity
//                        .GetMaterialSlotCount()
//                        >= 8)
//                    {
//                        BOX_LOG_WARNING(
//                            "Maximum material slots reached"
//                        );
//                    }
//                    else
//                    {
//                        Material loadedMaterial = previewMaterial;
//                        // =====================================
//                        // CREATE MATERIAL SLOT
//                        // =====================================
//
//                        const std::size_t newSlot =
//                            entity.AddMaterialSlot(
//                                loadedMaterial
//                            );
//
//
//                        // =====================================
//                        // ASSIGN TO SELECTED FACE(S)
//                        // =====================================
//
//                        entity
//                            .SetSelectedFacesMaterial(
//                                newSlot
//                            );
//
//
//                        // =====================================
//                        // REBUILD RENDER MESH
//                        // =====================================
//
//                        MeshEditing& mesh =
//                            entity
//                            .GetEditableMesh();
//
//
//                        MeshData renderMesh;
//
//
//                        if (mesh.BuildRenderMesh(
//                            renderMesh))
//                        {
//                            entity
//                                .CreateFromMeshData(
//                                    renderMesh
//                                );
//                        }
//
//
//                        BOX_LOG_INFO(
//                            "Loaded material "
//                            + materialName
//                            + " into slot "
//                            + std::to_string(
//                                newSlot
//                            )
//                        );
//                    }
//                }
//            }
//
//
//            // =================================================
//            // MATERIAL NAME
//            // =================================================
//
//            ImGui::TextWrapped(
//                "%s",
//                materialName.c_str()
//            );
//
//
//            ImGui::PopID();
//        }
//
//
//        ImGui::EndTable();
//    }
//
//
//    ImGui::End();
//}
//
//
//
//GLuint MaterialDisplayPanel::GetOrLoadTexture(BoxEngine& engine, const std::string& path)
//{
//    if (path.empty() || path == "none")
//    {
//        return 0;
//    }
//
//    // Have we already loaded it?
//    auto it = m_textureCache.find(path);
//
//    if (it != m_textureCache.end())
//    {
//        return it->second;
//    }
//
//    // First time we've seen this texture.
//    const GLuint textureID = engine.LoadTexture(path);
//
//    if (textureID != 0)
//    {
//        m_textureCache[path] = textureID;
//    }
//
//    return textureID;
//}
//
