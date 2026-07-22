#include "BoxEngine.h"
#include "panels/SceneCollectionPanel.h"
#include <imgui/imgui.h>
#include <imgui/ImGuiAF.h>

#include <entity/Entity.h>

void SceneCollectionPanel::DrawSceneCollection(
    BoxEngine& engine)
{
    ImGui::Begin("Scene Collection");

    const auto& entities = engine.GetEntities();

    int entityToDelete = -1;

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

            const int entityID = entity->GetID();

            const bool isSelected = engine.GetSelectedEntityID() == entityID;

            ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_Leaf |
                ImGuiTreeNodeFlags_NoTreePushOnOpen |
                ImGuiTreeNodeFlags_SpanAvailWidth;

            if (isSelected)
            {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            const char* visibilityIcon =
                entity->IsVisible()
                ? ICON_FA_EYE
                : ICON_FA_EYE_SLASH;

            const std::string displayName =
                entity->GetName().empty()
                ? "Entity " +
                std::to_string(entityID)
                : entity->GetName();

            const std::string label =
                std::string(visibilityIcon) +
                " " +
                displayName;

            ImGui::TreeNodeEx(
                reinterpret_cast<void*>(
                    static_cast<intptr_t>(
                        entityID
                        )
                    ),
                flags,
                "%s",
                label.c_str()
            );

            if (ImGui::IsItemClicked())
            {
                engine.SetSelectedEntity(
                    entityID
                );
            }

            if (ImGui::BeginPopupContextItem())
            {
                ImGui::TextDisabled(
                    "%s",
                    displayName.c_str()
                );

                ImGui::Separator();

                if (ImGui::MenuItem(
                    ICON_FA_TRASH_ALT " Delete"))
                {
                    entityToDelete =
                        entityID;
                }

                const char* visibilityText =
                    entity->IsVisible()
                    ? ICON_FA_EYE_SLASH " Hide"
                    : ICON_FA_EYE " Show";

                if (ImGui::MenuItem(
                    visibilityText))
                {
                    entity->SetVisible(
                        !entity->IsVisible()
                    );
                }

                ImGui::EndPopup();
            }
        }

        ImGui::TreePop();
    }

    if (entityToDelete != -1)
    {
        engine.RemoveEntity(
            entityToDelete
        );
    }

    if (ImGui::IsWindowHovered() &&
        ImGui::IsMouseClicked(
            ImGuiMouseButton_Left) &&
        !ImGui::IsAnyItemHovered())
    {
        engine.ClearSelectedEntity();
    }

    ImGui::End();
}


