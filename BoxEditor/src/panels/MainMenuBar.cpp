#include "panels\MainMenuBar.h"
#include <imgui/imgui.h>
#include <imgui/ImGuiAF.h>
#include <miniBoxLog.h>
//#include <panels\SceneViewportPanel.h>

MenuAction MainMenuBar::DrawMainMenu()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene"))
            {
                ImGui::EndMenu();
                ImGui::EndMainMenuBar();
                return MenuAction::NewScene;
            }
            if (ImGui::MenuItem("Open Scene"))
            {

            }
            if (ImGui::MenuItem("Save"))
            {

            }
            if (ImGui::MenuItem("Save As"))
            {

            }
			ImGui::Separator();
            if (ImGui::MenuItem("Import .obj"))
            {
				// implement import functionality here, e.g., open a file dialog to select a file to import into the scene.
            }
            if (ImGui::MenuItem("Export .mbx"))
            {
				// Export the current scene or selected objects to a file format, e.g., OBJ or a custom format MBX.
            }

			ImGui::Separator();
            if (ImGui::MenuItem(ICON_FA_SIGN_OUT_ALT" Exit"))
            {
                ImGui::EndMenu();
                ImGui::EndMainMenuBar();
                return MenuAction::Exit;

            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo"))
            {
            }
            if (ImGui::MenuItem("Redo"))
            {

            }
            if (ImGui::MenuItem("Test_01"))
            {

            }
            if (ImGui::MenuItem("Test_02"))
            {

            }

            ImGui::Separator();
            if (ImGui::MenuItem("Preferences"))
            {
				// open preferences dialog set grid size, snap to grid, screen color etc. 

            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Render"))
        {
            if (ImGui::MenuItem("Render Image"))
            {
				// Render the current scene to an image file, e.g., PNG or JPEG.
                // This would involve capturing the framebuffer and saving it to disk.
            }
            if (ImGui::MenuItem("Render Animation"))
            {

            }
            ImGui::Separator();
            if (ImGui::MenuItem("Test_01"))
            {

            }
            if (ImGui::MenuItem("Test_02"))
            {

            }

            if (ImGui::MenuItem("Test_03"))
            {
                

            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
    return MenuAction::None;
}

VeiwPortMenuAction MainMenuBar::DrawViewportMenu()
{

    //const float tbHeight = 25.0f;
    //ImGui::BeginChild("##scene_toolbar", ImVec2(ImGui::GetContentRegionAvail().x, tbHeight), false, ImGuiWindowFlags_NoDecoration);

    //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));
    //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 4));


    //ImGui::PushID("top_Buttons");

    //ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // normal

    //ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.16f, 0.70f, 0.16f, 1.0f)); // hover
    //ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.50f, 0.10f, 1.0f)); // active/click
    //ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.8f, 1.0f)); // active/click

    //ImGui::GetStyle().FrameBorderSize = 0.3f; // Add a border to the button
    //ImGui::GetStyle().FrameRounding = 6.0f; // rounded corners of buttons



    //const char* items[] = { "Select Mode", ICON_FA_CUBE " Object Mode", ICON_FA_VECTOR_SQUARE " Edit Mode" };
    //ImGui::SetNextItemWidth(100.0f);
    //ImGui::SameLine();
    //if (ImGui::Combo("##combo", &m_EditMode, items, IM_ARRAYSIZE(items))) {
    //    if (m_EditMode == 1) {
    //        if (m_actionCallback) m_actionCallback("AddObjectMode");

    //        BOX_LOG_INFO("Object mode selected");
    //    }
    //    else if (m_EditMode == 2) {
    //        if (m_actionCallback) m_actionCallback("AddEditMode");

    //        BOX_LOG_INFO("Edit mode selected");
    //    }
    //}
    //ImGui::SameLine();

    //ImGui::PopStyleVar(2);
    //ImGui::PopStyleColor(4); // pop all 4 pushed colors has to match top
    //ImGui::PopID();

    //ImGui::EndChild();

    return VeiwPortMenuAction();
}
