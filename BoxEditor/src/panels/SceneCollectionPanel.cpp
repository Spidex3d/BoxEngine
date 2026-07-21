#include "BoxEngine.h"
#include "panels/SceneCollectionPanel.h"
#include <imgui/imgui.h>
#include <imgui/ImGuiAF.h>

#include <entity/Entity.h>

void SceneCollectionPanel::DrawSceneCollection(
    BoxEngine& engine)
{
    ImGui::Begin("Scene Collection");

    const auto& entities =
        engine.GetEntities();

    if (ImGui::TreeNodeEx(
        "Scene",
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_SpanAvailWidth))
    {
        for (const auto& entity : entities)
        {
            if (!entity)
            {
                continue;
            }

            /*const bool isSelected =
                m_selectedEntity == entity.get();*/

            const bool isSelected =
                engine.GetSelectedEntityID() ==
                entity->GetID();

            ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_Leaf |
                ImGuiTreeNodeFlags_NoTreePushOnOpen |
                ImGuiTreeNodeFlags_SpanAvailWidth;

            if (isSelected)
            {
                flags |=
                    ImGuiTreeNodeFlags_Selected;
            }

            ImGui::TreeNodeEx(
                reinterpret_cast<void*>(
                    static_cast<intptr_t>(
                        entity->GetID())),
                flags,
                "%s",
                entity->GetName().c_str()
            );

            if (ImGui::IsItemClicked())
            {
                /*m_selectedEntity =
                    entity.get();*/

                engine.SetSelectedEntity(
                    entity->GetID()
                );
            }
        }

        ImGui::TreePop();
    }
	// clicking on the empty space of the Scene Collection panel will clear the selected entity
    if (ImGui::IsWindowHovered() &&
        ImGui::IsMouseClicked(
            ImGuiMouseButton_Left) &&
        !ImGui::IsAnyItemHovered())
    {
        engine.ClearSelectedEntity();
    }

    ImGui::End();
}


