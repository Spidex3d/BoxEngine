#include "panels/MaterialBrowser.h"

#include "preview/MaterialPreview.h"
#include "BoxEngine.h"
#include <entity/Entity.h>
#include <rendering/Material.h>
#include <imgui/imgui.h>
#include <imgui/ImGuiAF.h>
#include <miniBoxLog.h>

MaterialBrowser::MaterialBrowser() = default;

MaterialBrowser::~MaterialBrowser()
{
    Shutdown();
}

bool MaterialBrowser::Initialize()
{
    BOX_LOG_INFO("MaterialBrowser::Initialize called");

    m_preview = std::make_unique<MaterialPreview>();

    if (!m_preview->Initialize(256, 256))
    {
        BOX_LOG_ERROR("MaterialPreview initialization failed");
        m_preview.reset();
        return false;
    }

    return true;
}

void MaterialBrowser::Shutdown()
{
    if (m_preview)
    {
        m_preview->Shutdown();
        m_preview.reset();
    }

    m_selectedMaterial = nullptr;
}

void MaterialBrowser::Draw(BoxEngine& engine)
{
    if (ImGui::Begin("Material Browser"))
    {
    

    if (ImGui::BeginTabBar("ObjectMaterialsTabs"))
    {
        // ####################################################
        // Materials tab
        // ####################################################
        if (ImGui::BeginTabItem("Materials"))
        {
            // set this as a base material not a textured material for now, we can add a textured material later
            std::vector<MaterialEntry> materials = CollectMaterials(engine);

            ImGui::Text("Loaded Materials: %zu", materials.size());
            ImGui::Separator();
            ImGui::Spacing();

            DrawMaterialList(engine, materials);



            ImGui::EndTabItem();
        }
        // ####################################################
        // Textures tab
        // ####################################################
        if (ImGui::BeginTabItem("Textures"))
        {
            // set this as a textured material 

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    // ###################################################################
    // Draw preview in separate window if a material is selected which cab be set to a different object in the scene.
    if (m_showPreview && m_selectedMaterial)
    {
        DrawPreviewWindow();
    }

    }
    ImGui::End();
}

std::vector<MaterialEntry> MaterialBrowser::CollectMaterials(
    BoxEngine& engine)
{
    std::vector<MaterialEntry> materials;
    const auto& entities = engine.GetEntities();

    for (const auto& entity : entities)
    {
        if (!entity)
        {
            continue;
        }

        MaterialEntry entry;
        entry.material = &entity->GetMaterial();
        entry.ownerEntityID = entity->GetID();

        // Create display name from entity
        if (!entity->GetName().empty())
        {
            entry.displayName = entity->GetName() + " Material";
        }
        else
        {
            entry.displayName =
                "Entity " +
                std::to_string(entity->GetID()) +
                " Material";
        }

        materials.push_back(entry);
    }

    return materials;
}



//void MaterialBrowser::DrawMaterialList(const std::vector<MaterialEntry>& materials)
void MaterialBrowser::DrawMaterialList(BoxEngine& engine,
    const std::vector<MaterialEntry>& materials)
{
    if (materials.empty())
    {
        ImGui::TextDisabled("No materials loaded");
        return;
    }

    // Create a child window with scrolling for the material list
    ImGui::BeginChild("MaterialListChild", ImVec2(0, 0), true,
        ImGuiWindowFlags_HorizontalScrollbar
    );

    for (size_t i = 0; i < materials.size(); ++i)
    {
        const MaterialEntry& entry = materials[i];

        ImGui::PushID(static_cast<int>(i));

        // Check if this material is selected
        bool isSelected = (m_selectedMaterial == entry.material);

        // Create selectable with material icon
		// I would like to use a material icon here, but for now, we can use a palette icon from Font Awesome
        std::string label = std::string(ICON_FA_PALETTE) + " " + entry.displayName;

        if (ImGui::Selectable(
            label.c_str(),
            isSelected,
            ImGuiSelectableFlags_None,
            ImVec2(0, 0)))
        {
            m_selectedMaterial = entry.material;
            m_selectedMaterialName = entry.displayName;
            m_showPreview = true;
        }

        // Show context menu
        if (ImGui::BeginPopupContextItem())
        {
            ImGui::TextDisabled("%s", entry.displayName.c_str());
            ImGui::Separator();
			// not sure if we need this
            if (ImGui::MenuItem(ICON_FA_EYE " Preview"))
            {
                m_selectedMaterial = entry.material;
                m_selectedMaterialName = entry.displayName;
                m_showPreview = true;

                              

            }

            if (ImGui::MenuItem(ICON_FA_SEARCH " Select Owner"))
            {
				// set the selected entity in the engine to the owner of this material
                engine.SetSelectedEntity(entry.ownerEntityID);
            }

            if (ImGui::MenuItem(ICON_FA_SEARCH " Use on Selected Entity"))
            {
				// use on the Selected entity set this material to the selected entity in the engine
                Entity* selectedEntity =
                    engine.GetSelectedEntity();

                if (selectedEntity &&
                    entry.material)
                {
                    selectedEntity->GetMaterial() =
                        *entry.material;
                }
                
            }

            ImGui::EndPopup();
        }

		// Show material properties on hover using the tooltip
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();

            const glm::vec4& baseColor = entry.material->GetBaseColor();

            ImGui::Text("Base Color: (%.2f, %.2f, %.2f, %.2f)",
                baseColor.r,
                baseColor.g,
                baseColor.b,
                baseColor.a
            );

            ImGui::Text("Metallic: %.2f",
                entry.material->GetMetallic()
            );

            ImGui::Text("Roughness: %.2f",
                entry.material->GetRoughness()
            );

            ImGui::Text("Alpha Strength: %.2f",
                entry.material->GetAlpha()
            );

            const glm::vec3& emissionColor =
				entry.material->GetEmissionColor();

            
            ImGui::Text("Emission Color: (%.2f, %.2f, %.2f)",
                emissionColor.r,
                emissionColor.g,
                emissionColor.b);

            ImGui::Text("Emission Strength: %.2f",
                entry.material->GetEmissionStrength()
            );
            

            ImGui::EndTooltip();

            




        }
        


        ImGui::PopID();
    }

    ImGui::EndChild();
}

// leave this in for now, but we can remove it later if we don't need it
void MaterialBrowser::DrawPreviewWindow()
{
    if (!m_selectedMaterial || !m_preview)
    {
        return;
    }

    ImGui::Begin("Material Selected",
        &m_showPreview,
        ImGuiWindowFlags_AlwaysAutoResize
    );

    ImGui::TextUnformatted(m_selectedMaterialName.c_str());
    ImGui::Separator();
    ImGui::Spacing();

    // Draw the preview
    m_preview->Draw(*m_selectedMaterial);

    ImGui::Spacing();

    // Display material properties
    if (ImGui::CollapsingHeader(
        "Material Properties",
        ImGuiTreeNodeFlags_DefaultOpen))
    {
        const glm::vec4& baseColor =
            m_selectedMaterial->GetBaseColor();

        ImGui::ColorEdit4(
            "Base Color",
            const_cast<float*>(&baseColor[0]),
            ImGuiColorEditFlags_NoInputs |
            ImGuiColorEditFlags_NoPicker
        );

        ImGui::Text("Metallic: %.2f",
            m_selectedMaterial->GetMetallic()
        );

        ImGui::Text("Roughness: %.2f",
            m_selectedMaterial->GetRoughness()
        );

        ImGui::Text("Alpha: %.2f",
            m_selectedMaterial->GetAlpha()
        );

        const glm::vec3& emissionColor =
            m_selectedMaterial->GetEmissionColor();

        if (m_selectedMaterial->GetEmissionStrength() > 0.0f)
        {
            ImGui::ColorEdit3(
                "Emission",
                const_cast<float*>(&emissionColor[0]),
                ImGuiColorEditFlags_NoInputs |
                ImGuiColorEditFlags_NoPicker
            );

            ImGui::Text("Emission Strength: %.2f",
                m_selectedMaterial->GetEmissionStrength()
            );
        }
    }

    ImGui::End();
}