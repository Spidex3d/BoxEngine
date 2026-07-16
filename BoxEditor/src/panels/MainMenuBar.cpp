#include "panels\MainMenuBar.h"
#include <imgui/imgui.h>
#include <imgui/ImGuiAF.h>




MenuAction MainMenuBar::DrawMainMenu()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene"))
            {
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
            if (ImGui::MenuItem("Exit"))
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