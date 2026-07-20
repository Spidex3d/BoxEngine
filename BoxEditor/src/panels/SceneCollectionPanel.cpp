#include "panels/SceneCollectionPanel.h"
#include <imgui/imgui.h>
#include <imgui/ImGuiAF.h>

//void SceneCollectionPanel::DrawSceneCollection()
//{
//    ImGui::Begin("Scene Collection window");
//    ImGui::Text("Cube Editor");
//    ImGui::End();
//}

#include <BoxEngine.h>
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

            const bool isSelected =
                m_selectedEntity == entity.get();

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
                m_selectedEntity =
                    entity.get();
            }
        }

        ImGui::TreePop();
    }

    ImGui::End();
}

Entity* SceneCollectionPanel::GetSelectedEntity() const
{
    return m_selectedEntity;
}
