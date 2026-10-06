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

	bool joinSelected = false; // Flag to indicate if the "Join Selected" option was chosen

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

           
            bool isSelected = false;

            const auto& selectedIDs =
                engine.GetSelectedEntityIDs();

            for (const int selectedID : selectedIDs)
            {
                if (selectedID == entityID)
                {
                    isSelected = true;
                    break;
                }
            }
            //######################## new bit

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
                const bool shiftHeld =
                    ImGui::GetIO().KeyShift;

                if (!shiftHeld)
                {
                    engine.ClearSelectedEntities();

                    engine.AddSelectedEntity(
                        entityID
                    );
                }
                else
                {
                    engine.AddSelectedEntity(
                        entityID
                    );
                }
            }

			// ######################### new bit

            if (ImGui::BeginPopupContextItem())
            {
                ImGui::TextDisabled(
                    "%s",
                    displayName.c_str()
                );
				// --------------------- Join Entities ---------------------
                const auto& selectedIDs = engine.GetSelectedEntityIDs();

                const bool canJoin = selectedIDs.size() == 2;

                if (ImGui::MenuItem(
                    "Join Selected",
                    nullptr,
                    false,
                    canJoin))
                {
                    joinSelected = true;
                }
                
				// ---------------------End Join Entities ---------------------

                ImGui::Separator();
                // -------------------------------------------------
                // Delete
                // -------------------------------------------------
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
	// ##########################################################################################
	// Join Selected Entities
	// ##########################################################################################
    if (joinSelected)
    {
        const auto& selectedIDs =
            engine.GetSelectedEntityIDs();

        if (selectedIDs.size() == 2)
        {
            Entity* first =
                engine.GetEntityByID(
                    selectedIDs[0]
                );

            Entity* second =
                engine.GetEntityByID(
                    selectedIDs[1]
                );

            if (first && second)
            {
                engine.JoinEntities(
                    first,
                    second
                );
            }
        }
    }

	// Delete the entity after the loop to avoid modifying the vector while iterating

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
       // engine.ClearSelectedEntity();
          engine.ClearSelectedEntities();
    }

    ImGui::End();
}


