#include "panels/UVPanel.h"
#include <imgui/imgui.h>
#include <BoxEngine.h>
#include <entity/Entity.h>
#include <mesh/UV/UVMapping.h>
#include <mesh/MeshEditing.h>
#include <mesh/MeshData.h>
#include <miniBoxLog.h>

UVPanel::~UVPanel() = default;

void UVPanel::Open()
{
    m_isOpen = true;
}

void UVPanel::Close()
{
    m_isOpen = false;
}

bool UVPanel::IsOpen() const
{
    return m_isOpen;
}

void UVPanel::Draw(BoxEngine& engine)
{
    if (!m_isOpen)
    {
        return;
    }

    // Draw the UV editor UI here
    ImGui::SetNextWindowSize(
        ImVec2(650.0f, 500.0f),
        ImGuiCond_FirstUseEver
    );

    if (!ImGui::Begin(
        "BoxEditor UV Panel",
        &m_isOpen))
    {
        ImGui::End();
        return;
    }

    Entity* selectedEntity =
        engine.GetSelectedEntity();

    if (!selectedEntity)
    {
        ImGui::TextDisabled(
            "No object selected."
        );

        ImGui::End();
        return;
    }

    ImGui::Text(
        "Object: %s",
        selectedEntity->GetName().c_str()
    );

    ImGui::Separator();

    const char* projectionTypes[] =
    {
        "Box",
        "Spherical"
    };

    ImGui::SetNextItemWidth(180.0f);

    ImGui::Combo(
        "Projection",
        &m_projectionType,
        projectionTypes,
        IM_ARRAYSIZE(projectionTypes)
    );

	ImGui::SeparatorText("UV Mapping");

    if (ImGui::Button(
        "Generate UV",
        ImVec2(140.0f, 32.0f)))
    {
        MeshEditing& editableMesh =
            selectedEntity->GetEditableMesh();

        bool generated = false;

        switch (m_projectionType)
        {
        case 0:
            generated =
                UVMapping::Box(editableMesh);
            break;

        case 1:
            generated =
                UVMapping::Spherical(editableMesh);
            break;
        }

        if (generated)
        {
            MeshData renderMesh;

            if (editableMesh.BuildRenderMesh(renderMesh))
            {
                selectedEntity->CreateFromMeshData(
                    renderMesh
                );

                BOX_LOG_INFO("UV coordinates generated for object: " + selectedEntity->GetName());
            }
        }
    }

		
   


    ImGui::End();
}
