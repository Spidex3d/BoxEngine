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

            if (ImGui::MenuItem("Save"))
            {
            }

            if (ImGui::MenuItem("Save As"))
            {
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Import .obj"))
            {
            }

            if (ImGui::MenuItem("Export .mbx"))
            {
            }

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

        if (ImGui::BeginMenu("Render"))
        {
            if (ImGui::MenuItem("Render Image"))
            {
            }

            if (ImGui::MenuItem("Render Animation"))
            {
            }

            ImGui::EndMenu();
        }

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

        if (ImGui::BeginMenu("Tools"))
        {
            if (ImGui::MenuItem("Move"))
            {
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    return action;
}