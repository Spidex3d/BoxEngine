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

        // ============================================
        // CAPTURE SELECTED FACE
        // ============================================

        if (faceEditController.HasSelectedFace())
        {
            const std::size_t selectedFace =
                faceEditController.GetSelectedFace();

            if (selectedFace <
                entity->GetEditableMesh().GetFaceCount())
            {
                newModifier.scatter.faceIndices.push_back(
                    selectedFace
                );
            }
        }



        // Default scatter settings
        newModifier.scatter.type =
            ParticleScatterType::Grass;

        newModifier.scatter.count = 100;

        newModifier.scatter.seed = 12345;

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

    if (ImGui::Button("Use Selected Face"))
    {
        if (faceEditController.HasSelectedFace())
        {
            const std::size_t faceIndex =
                faceEditController.GetSelectedFace();

            if (faceIndex <
                entity->GetEditableMesh().GetFaceCount())
            {
                // Replace the previous selection
                scatter.faceIndices.clear();

                scatter.faceIndices.push_back(faceIndex);
            }
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Clear Faces"))
    {
        scatter.faceIndices.clear();
    }

	// ============================================ End Buttons =========================================



    ImGui::SeparatorText("Surface Scatter Settings");

    // ============================================
    // ENABLE / DISABLE
    // ============================================

    ImGui::Checkbox(
        "Enable Scatter",
        &modifier.modEnabled
    );

    ImGui::Spacing();

    // ============================================
    // PARTICLE TYPE
    // ============================================

    const char* particleTypes[] =
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
    }

    // ============================================
    // PARTICLE COUNT
    // ============================================

	ImGui::InputInt(
		"Particle Count",
		&scatter.count
	);

    // ============================================
    // RANDOM SEED
    // ============================================

    ImGui::SetNextItemWidth(150.0f);

    ImGui::InputScalar("##RandomSeed", ImGuiDataType_U32, &scatter.seed);

    ImGui::SameLine();

    if (ImGui::Button("Randomise"))
    {
        static std::random_device rd;

        static std::mt19937 randomGenerator(rd());

        std::uniform_int_distribution<std::uint32_t>
            distribution;

        std::uint32_t newSeed;

        do
        {
            newSeed = distribution(randomGenerator);
        } while (newSeed == scatter.seed);

        scatter.seed = newSeed;
    }

    ImGui::SameLine();

    ImGui::TextUnformatted("Seed");

    // ============================================
    // SCALE VARIATION
    // ============================================

    ImGui::SeparatorText("Scale Variation");

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
    );

    // Ensure maximum is never below minimum
    scatter.maxScale =
        std::max(
            scatter.minScale,
            scatter.maxScale
        );

    // ============================================
    // ROTATION
    // ============================================

    ImGui::SeparatorText("Rotation");

    ImGui::SliderFloat(
        "Random Rotation",
        &scatter.randomRotation,
        0.0f,
        360.0f
    );

    // ============================================
    // TERRAIN SLOPE
    // ============================================

    ImGui::SeparatorText("Surface Restrictions");

    ImGui::SliderFloat(
        "Maximum Slope",
        &scatter.maxSlope,
        0.0f,
        180.0f
    );

    ImGui::Spacing();



    // ============================================
    // REMOVE MODIFIER
    // ============================================

    ImGui::Separator();

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

    /*if (ImGui::Button("Remove Scatter Modifier"))
    {
        modifiers.erase(
            modifiers.begin() + selectedIndex
        );

        entity->SetSelectedModifierIndex(-1);

        ImGui::End();
        return;
    }*/

    ImGui::End();
}
