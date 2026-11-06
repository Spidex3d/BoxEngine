#include "panels/ParticleSystemPanel.h"
#include <imgui/ImGuiAF.h>
#include <imgui/imgui.h>
#include <entity/Entity.h>
#include <mesh/modifiers/ModifierData.h>

#include <mesh/ParticleSystem/ParticleScatter.h>
#include <mesh/MeshEditing.h>
#include <tools/FaceEditController.h>

#include <random>
#include <cstdint>
#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
ParticleSystemPanel::~ParticleSystemPanel()
{}

bool ParticleSystemPanel::Initialize()
{
	return true;
}

void ParticleSystemPanel::Open()
{
	m_isOpen = true;
}

void ParticleSystemPanel::Close()
{
	m_isOpen = false;
}

bool ParticleSystemPanel::IsOpen() const
{
	return m_isOpen;
}

void ParticleSystemPanel::Draw(Entity* entity, FaceEditController& faceEditController)
{
    if (!m_isOpen)
        return;

    if (!ImGui::Begin("Particle System", &m_isOpen))
    {
        ImGui::End();
        return;
    }

    ImGui::SeparatorText("Particle System");

    // ============================================
    // CHECK SELECTED OBJECT ParticleScatter
    // ============================================

    if (!entity)
    {
        ImGui::TextDisabled("No object selected.");

        ImGui::TextWrapped(
            "Select an object to add or edit "
            "particle modifiers."
        );

        ImGui::End();
        return;
    }

    ImGui::Text(
        "Selected Object: %s",
        entity->GetName().c_str()
    );

    ImGui::Spacing();

    // ============================================
    // GET ENTITY MODIFIERS
    // ============================================

    std::vector<ModifierData>& modifiers =
        entity->GetModifiers();

    // ============================================
    // ADD SURFACE SCATTER
    // ============================================

    ImGui::SeparatorText("Particle Modifiers");

    if (ImGui::Button("Add Surface Scatter"))
    {
        ModifierData newModifier;

        newModifier.type =
            ModifierType::Scatter;

        newModifier.modEnabled = true;


        // =====================================================
        // CAPTURE ALL SELECTED FACES
        // =====================================================

        const std::vector<std::size_t>& selectedFaces =
            entity->GetSelectedFaces();

        const std::size_t totalFaces =
            entity->GetEditableMesh().GetFaceCount();

        for (std::size_t faceIndex : selectedFaces)
        {
            // Ignore invalid face indices.
            if (faceIndex >= totalFaces)
                continue;

            // Store this face permanently in the modifier.
            newModifier.scatter.faceIndices.push_back(
                faceIndex
            );
        }

        // Avoid duplicate face indices.
        std::sort(
            newModifier.scatter.faceIndices.begin(),
            newModifier.scatter.faceIndices.end()
        );

        newModifier.scatter.faceIndices.erase(
            std::unique(
                newModifier.scatter.faceIndices.begin(),
                newModifier.scatter.faceIndices.end()
            ),
            newModifier.scatter.faceIndices.end()
        );



        // Default scatter settings
        newModifier.scatter.type =
            ParticleScatterType::Grass;

        newModifier.scatter.count = 100;

        newModifier.scatter.seed = 12345;

        // =====================================================
        // DEFAULT GRASS MIXTURE
        // =====================================================

        // Grass 1 - Fine Grass
        ScatterEntry fineGrass;

        fineGrass.name = "Fine Grass";
        fineGrass.type = ParticleScatterType::Grass;
        fineGrass.assetID = 0;

        fineGrass.density = 1.0f;

        fineGrass.minScale = 0.6f;
        fineGrass.maxScale = 0.9f;

        fineGrass.seedOffset = 1001;
        // ####
        fineGrass.bladeCount = 35;
        fineGrass.bladeSpread = 0.045f;

        fineGrass.minBladeHeight = 0.91f;
        fineGrass.maxBladeHeight = 2.11f;

        fineGrass.minBladeWidth = 6.21f;
        fineGrass.maxBladeWidth = 9.71f;

        fineGrass.minBladeTilt = 0.0f;
        fineGrass.maxBladeTilt = 0.0f;


        // Grass 2 - Meadow Grass
        ScatterEntry meadowGrass;

        meadowGrass.name = "Meadow Grass";
        meadowGrass.type = ParticleScatterType::Grass;
        meadowGrass.assetID = 1;

        meadowGrass.density = 1.0f;

        meadowGrass.minScale = 0.9f;
        meadowGrass.maxScale = 1.2f;

        meadowGrass.seedOffset = 2002;
        // ####
        meadowGrass.bladeCount = 25;
        meadowGrass.bladeSpread = 0.045f;

        meadowGrass.minBladeHeight = 0.60f;
        meadowGrass.maxBladeHeight = 1.2f;

        meadowGrass.minBladeWidth = 5.208f;
        meadowGrass.maxBladeWidth = 6.21f;

        meadowGrass.minBladeTilt = 0.0f;
        meadowGrass.maxBladeTilt = 0.0f;

        // Grass 3 - Wild Grass
        ScatterEntry wildGrass;

        wildGrass.name = "Wild Grass";
        wildGrass.type = ParticleScatterType::Grass;
        wildGrass.assetID = 2;

        wildGrass.density = 1.0f;

        wildGrass.minScale = 1.3f;
        wildGrass.maxScale = 1.8f;

        wildGrass.seedOffset = 3003;
        //####
        wildGrass.bladeCount = 7;
        wildGrass.bladeSpread = 0.045f;

        wildGrass.minBladeHeight = 0.9f;
        wildGrass.maxBladeHeight = 1.4f;

        wildGrass.minBladeWidth = 0.9f;
        wildGrass.maxBladeWidth = 1.2f;

        wildGrass.minBladeTilt = 0.0f;
        wildGrass.maxBladeTilt = 0.0f;


        // Add all three grass types to the mixture
        newModifier.scatter.entries.push_back(fineGrass);
        newModifier.scatter.entries.push_back(meadowGrass);
        newModifier.scatter.entries.push_back(wildGrass);




        newModifier.scatter.minScale = 0.8f;
        newModifier.scatter.maxScale = 1.2f;

        newModifier.scatter.randomRotation = 360.0f;

        newModifier.scatter.maxSlope = 45.0f;

        modifiers.push_back(newModifier);

        // Automatically select the new modifier
        entity->SetSelectedModifierIndex(
            static_cast<int>(modifiers.size()) - 1
        );
    }

    ImGui::Spacing();

    // ============================================
    // DISPLAY SCATTER MODIFIERS
    // ============================================

    int selectedIndex =
        entity->GetSelectedModifierIndex();

    for (std::size_t i = 0; i < modifiers.size(); ++i)
    {
        ModifierData& modifier = modifiers[i];

        // Only show Scatter modifiers in this panel
        if (modifier.type != ModifierType::Scatter)
            continue;

        ImGui::PushID(static_cast<int>(i));

        const std::string label =
            "Surface Scatter " +
            std::to_string(i + 1);

        const bool selected =
            selectedIndex == static_cast<int>(i);

        if (ImGui::Selectable(label.c_str(), selected))
        {
            selectedIndex = static_cast<int>(i);

            entity->SetSelectedModifierIndex(
                selectedIndex
            );
        }

        ImGui::PopID();
    }

    ImGui::Spacing();

    // ============================================
    // SELECTED MODIFIER SETTINGS
    // ============================================

    selectedIndex =
        entity->GetSelectedModifierIndex();

    if (selectedIndex < 0 ||
        selectedIndex >= static_cast<int>(modifiers.size()) ||
        modifiers[selectedIndex].type != ModifierType::Scatter)
    {
        ImGui::TextDisabled(
            "Select a Surface Scatter modifier."
        );

        ImGui::End();
        return;
    }

    ModifierData& modifier =
        modifiers[selectedIndex];

    ParticleScatterData& scatter = modifier.scatter;

    // ============================================
    // TARGET FACES
    // ============================================

    ImGui::SeparatorText("Target Faces");

    ImGui::Text(
        "Assigned Faces: %d",
        static_cast<int>(scatter.faceIndices.size())
    );

    // =====================================================
    // USE CURRENT MULTI-FACE SELECTION
    // =====================================================

    if (ImGui::Button("Use Selected Faces"))
    {
        const std::vector<std::size_t>& selectedFaces =
            entity->GetSelectedFaces();

        const std::size_t totalFaces =
            entity->GetEditableMesh().GetFaceCount();

        if (!selectedFaces.empty())
        {
            scatter.faceIndices.clear();

            for (std::size_t faceIndex : selectedFaces)
            {
                if (faceIndex < totalFaces)
                {
                    scatter.faceIndices.push_back(
                        faceIndex
                    );
                }
            }

            std::sort(
                scatter.faceIndices.begin(),
                scatter.faceIndices.end()
            );

            scatter.faceIndices.erase(
                std::unique(
                    scatter.faceIndices.begin(),
                    scatter.faceIndices.end()
                ),
                scatter.faceIndices.end()
            );

            // Remove old cached distributions.
            ParticleScatter::InvalidateCache(
                entity->GetID(),
                static_cast<std::size_t>(selectedIndex)
            );
        }
    }

   

    ImGui::SameLine();

    if (ImGui::Button("Clear Faces"))
    {
        scatter.faceIndices.clear();
    }

    ImGui::SameLine();
    // ============================================
    // REMOVE MODIFIER
    // ============================================

    if (ImGui::Button("Remove Scatter Modifier"))
    {
        // Clear existing cached points for this object.
        // Modifier indices may change after erase().
        ParticleScatter::ClearEntityCache(entity->GetID());

        // Remove the selected modifier.
        modifiers.erase(
            modifiers.begin() + selectedIndex
        );

        // Nothing selected after removal.
        entity->SetSelectedModifierIndex(-1);

        ImGui::End();
        return;
    }

	// ============================================ End Buttons =========================================



    ImGui::SeparatorText("Surface Scatter Settings");

    // ============================================
    // ENABLE / DISABLE
    // ============================================

    
    ImGui::Spacing();

    // ============================================
    // PARTICLE TYPE
    // ============================================

    /*const char* particleTypes[] =
    {
        "Grass",
        "Rocks",
        "Trees",
        "Custom Mesh"
    };

    int particleType = static_cast<int>(scatter.type);

    if (ImGui::Combo(
        "Particle Type",
        &particleType,
        particleTypes,
        IM_ARRAYSIZE(particleTypes)))
    {
        scatter.type =
            static_cast<ParticleScatterType>(
                particleType
                );
    }*/

    

    // ============================================
    // SCALE VARIATION
    // ============================================

   /* ImGui::SeparatorText("Scale Variation");

    ImGui::SliderFloat(
        "Minimum Scale",
        &scatter.minScale,
        0.1f,
        5.0f
    );

    ImGui::SliderFloat(
        "Maximum Scale",
        &scatter.maxScale,
        0.1f,
        5.0f
    );*/

    // Ensure maximum is never below minimum
    /*scatter.maxScale =
        std::max(
            scatter.minScale,
            scatter.maxScale
        );*/

    // ============================================
    // ROTATION
    // ============================================

    /*ImGui::SeparatorText("Rotation");

    ImGui::SliderFloat(
        "Random Rotation",
        &scatter.randomRotation,
        0.0f,
        360.0f
    );*/

    // ============================================
    // TERRAIN SLOPE
    // ============================================

    /*ImGui::SeparatorText("Surface Restrictions");

    ImGui::SliderFloat(
        "Maximum Slope",
        &scatter.maxSlope,
        0.0f,
        180.0f
    );

    ImGui::Spacing();*/



    

    ImGui::Separator();

    

    // =====================================================
// GRASS PARTICLE LIBRARY
// =====================================================

    if (ImGui::CollapsingHeader(
        "Grass",
        ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::TextDisabled(
            "Select and configure grass varieties."
        );

        ImGui::Spacing();

        // ================================================
        // DISPLAY GRASS ENTRIES
        // ================================================

        for (std::size_t i = 0;
            i < scatter.entries.size();
            ++i)
        {
            ScatterEntry& entry = scatter.entries[i];

            if (entry.type != ParticleScatterType::Grass)
                continue;

            ImGui::PushID(static_cast<int>(i));

            ImGui::SeparatorText(entry.name.c_str());

            // --------------------------------------------
            // Enable individual grass type
            // --------------------------------------------

            ImGui::Checkbox(
                "Enabled",
                &entry.enabled
            );

            // --------------------------------------------
            // Density per square metre
            // --------------------------------------------

            ImGui::SetNextItemWidth(120.0f);

            ImGui::InputFloat(
                "Density / m2",
                &entry.density,
                1.0f,
                10.0f,
                "%.1f"
            );

            entry.density = std::max(
                0.0f,
                entry.density
            );

            // --------------------------------------------
            // Scale variation
            // --------------------------------------------

            ImGui::SetNextItemWidth(120.0f);

            ImGui::InputFloat(
                "Minimum Scale",
                &entry.minScale,
                0.1f,
                0.5f,
                "%.2f"
            );

            entry.minScale = std::max(
                0.01f,
                entry.minScale
            );

            ImGui::SetNextItemWidth(120.0f);

            ImGui::InputFloat(
                "Maximum Scale",
                &entry.maxScale,
                0.1f,
                0.5f,
                "%.2f"
            );

            entry.maxScale = std::max(
                entry.minScale,
                entry.maxScale
            );

            // --------------------------------------------
            // Rotation variation
            // --------------------------------------------

            ImGui::SetNextItemWidth(120.0f);

            ImGui::InputFloat(
                "Minimum Rotation",
                &entry.minRotation,
                1.0f,
                10.0f,
                "%.1f"
            );

            ImGui::SetNextItemWidth(120.0f);

            ImGui::InputFloat(
                "Maximum Rotation",
                &entry.maxRotation,
                1.0f,
                10.0f,
                "%.1f"
            );

            entry.minRotation =
                std::clamp(
                    entry.minRotation,
                    0.0f,
                    360.0f
                );

            entry.maxRotation =
                std::clamp(
                    entry.maxRotation,
                    entry.minRotation,
                    360.0f
                );

           

            // =====================================================
            // GRASS GROWTH DIRECTION
            // =====================================================

            ImGui::SetNextItemWidth(120.0f);

            ImGui::InputFloat(
                "Surface Alignment",
                &entry.surfaceAlignment,
                0.1f,
                0.5f,
                "%.2f"
            );

            // Restrict between vertical and surface normal.
            entry.surfaceAlignment =
                std::clamp(
                    entry.surfaceAlignment,
                    0.0f,
                    1.0f
                );

            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip(
                    "0.0 = Vertical\n"
                    "0.5 = Partially follows surface\n"
                    "1.0 = Follows surface normal"
                );
            }

			// =====================================================
			// GRASS CLUMP SETTINGS
			// =====================================================
            // =====================================================
            // INDIVIDUAL GRASS BLADE SETTINGS
            // =====================================================

            ImGui::SeparatorText("Blade Settings");

            // ---------------------------------------------
            // Blade Count
            // ---------------------------------------------

            ImGui::SetNextItemWidth(120.0f);

            ImGui::InputInt(
                "Blade Count",
                &entry.bladeCount
            );

            entry.bladeCount =
                std::clamp(
                    entry.bladeCount,
                    1,
                    100
                );

            // ---------------------------------------------
            // Blade Spread
            // ---------------------------------------------

            ImGui::SetNextItemWidth(120.0f);

            ImGui::InputFloat(
                "Blade Spread",
                &entry.bladeSpread,
                0.005f,
                0.01f,
                "%.3f"
            );

            entry.bladeSpread =
                std::max(
                    0.0f,
                    entry.bladeSpread
                );

            // ---------------------------------------------
            // Height Variation
            // ---------------------------------------------

            ImGui::SetNextItemWidth(120.0f);

            ImGui::InputFloat(
                "Minimum Blade Height",
                &entry.minBladeHeight,
                0.1f,
                0.5f,
                "%.2f"
            );

            ImGui::SetNextItemWidth(120.0f);

            ImGui::InputFloat(
                "Maximum Blade Height",
                &entry.maxBladeHeight,
                0.1f,
                0.5f,
                "%.2f"
            );

            entry.minBladeHeight =
                std::max(0.01f, entry.minBladeHeight);

            entry.maxBladeHeight =
                std::max(
                    entry.minBladeHeight,
                    entry.maxBladeHeight
                );

            // ---------------------------------------------
            // Width Variation
            // ---------------------------------------------

            ImGui::SetNextItemWidth(120.0f);

            ImGui::InputFloat(
                "Minimum Blade Width",
                &entry.minBladeWidth,
                0.1f,
                0.5f,
                "%.2f"
            );

            ImGui::SetNextItemWidth(120.0f);

            ImGui::InputFloat(
                "Maximum Blade Width",
                &entry.maxBladeWidth,
                0.1f,
                0.5f,
                "%.2f"
            );

            entry.minBladeWidth =
                std::max(0.01f, entry.minBladeWidth);

            entry.maxBladeWidth =
                std::max(
                    entry.minBladeWidth,
                    entry.maxBladeWidth
                );

            // ---------------------------------------------
            // Blade Tilt
            // ---------------------------------------------

            ImGui::SetNextItemWidth(120.0f);

            ImGui::InputFloat(
                "Minimum Blade Tilt",
                &entry.minBladeTilt,
                1.0f,
                5.0f,
                "%.1f"
            );

            ImGui::SetNextItemWidth(120.0f);

            ImGui::InputFloat(
                "Maximum Blade Tilt",
                &entry.maxBladeTilt,
                1.0f,
                5.0f,
                "%.1f"
            );

            entry.minBladeTilt =
                std::clamp(
                    entry.minBladeTilt,
                    0.0f,
                    90.0f
                );

            entry.maxBladeTilt =
                std::clamp(
                    entry.maxBladeTilt,
                    entry.minBladeTilt,
                    90.0f
                );




            ImGui::Spacing();

            ImGui::PopID();
        }

        // ================================================
        // DISPLAY TOTAL GRASS DENSITY
        // ================================================

        float totalDensity = 0.0f;

        for (const ScatterEntry& entry : scatter.entries)
        {
            if (entry.enabled &&
                entry.type == ParticleScatterType::Grass)
            {
                totalDensity += entry.density;
            }
        }

        ImGui::Separator();

        ImGui::Text(
            "Total Grass Density: %.1f clumps/m2",
            totalDensity
        );
    }

    if (ImGui::CollapsingHeader("Plants & Flowers"))
    {
        // Plant asset thumbnails
    }

    if (ImGui::CollapsingHeader("Rocks"))
    {
        // Rock asset thumbnails
    }

    if (ImGui::CollapsingHeader("Trees"))
    {
        // Tree asset thumbnails
    }

    ImGui::End();
}
