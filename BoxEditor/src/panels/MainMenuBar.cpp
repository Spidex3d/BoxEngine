#include "panels\MainMenuBar.h"
#include <imgui/imgui.h>
#include <imgui/ImGuiAF.h>
#include <miniBoxLog.h>

MenuAction MainMenuBar::DrawMainMenu()
{
    MenuAction action = MenuAction::None;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene"))
            {
                action = MenuAction::NewScene;
            }

            if (ImGui::MenuItem("Open Scene"))
            {
            }

            if (ImGui::MenuItem("Save Scene"))
            {
            }

            if (ImGui::MenuItem("Save Scene As"))
            {
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Export .mbx"))
            {
                action = MenuAction::Exportmbx;
            }
            if (ImGui::MenuItem("Import .mbx"))
            {
                action = MenuAction::Importmbx;
            }
            if (ImGui::MenuItem("Import .obj"))
            {
            }
            if (ImGui::MenuItem("Export .obj"))
            {
            }
			// we can add gltf later if we want to support it.
            ImGui::Separator();

            if (ImGui::MenuItem(
                ICON_FA_SIGN_OUT_ALT " Exit"))
            {
                action = MenuAction::Exit;
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

            ImGui::Separator();

            if (ImGui::MenuItem("Preferences"))
            {
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Render State"))
        {
            if (ImGui::MenuItem("Render Image"))
            {
            }

            if (ImGui::MenuItem("Render Solid"))
            {
            }

            if (ImGui::MenuItem("Render Wireframe"))
            {
            }

            ImGui::EndMenu();
        }
		// Not sure if we need this menu,we can do this with the right click context menu in viewport.
        if (ImGui::BeginMenu("Add Object"))
        {
            if (ImGui::MenuItem("Add Cube"))
            {
                action = MenuAction::AddCube;
            }

            if (ImGui::MenuItem("Add Plane"))
            {
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Add Icosphere"))
            {
            }

            if (ImGui::MenuItem("Add Sphere"))
            {
				action = MenuAction::AddSphere;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Add Cylinder"))
            {
            }

            if (ImGui::MenuItem("Add Cone"))
            {
            }

            if (ImGui::MenuItem("Add Torus"))
            {
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Modifiers"))
        {
            if (ImGui::MenuItem("Extrude"))
            {
                
            }
            if (ImGui::MenuItem("Inset"))
            {
               
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Edge loop"))
            {

            }
            if (ImGui::MenuItem("Bevel"))
            {

            }
            if (ImGui::MenuItem("Solidify"))
            {

            }
            ImGui::Separator();
            if (ImGui::MenuItem("Scatter"))
            {

            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("Help"))
            {
				action = MenuAction::Help;
                
            }
            if (ImGui::MenuItem("About"))
            {
				action = MenuAction::About;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    return action;
}