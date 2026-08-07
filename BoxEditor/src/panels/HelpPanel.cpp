#include "panels/HelpPanel.h"
#include <imgui/imgui.h>
#include <imgui/ImGuiAF.h>
#include "Helpers.h" // for color text and centering text

Helpers g_helpers; // Global instance of Helpers class

void HelpPanel::Open()
{
    m_isOpen = true;
}

void HelpPanel::Close()
{
    m_isOpen = false;
}

bool HelpPanel::IsOpen() const
{
    return m_isOpen;
}

void HelpPanel::Draw()
{
    if (!m_isOpen)
    {
        return;
    }

    ImGui::SetNextWindowSize(
        ImVec2(650.0f, 500.0f),
        ImGuiCond_FirstUseEver
    );

    if (!ImGui::Begin(
        "BoxEditor Help",
        &m_isOpen))
    {
        ImGui::End();
        return;
    }
	
    // Helper Function to center text with color
    g_helpers.CenterTextColored(COLOR_LIGHTBLUE, ICON_FA_EDIT " BoxEditor Reference");

    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::BeginTabBar(
        "##HelpTabs"))
    {
        if (ImGui::BeginTabItem(
            "Getting Started"))
        {
			// Helper Function to center text with color
            g_helpers.CenterTextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), ICON_FA_AMBULANCE " Help doc's");
            ImGui::Spacing();
            ImGui::TextWrapped(
                "BoxEditor is a small 3D modelling application. "
				"To get started right click in the viewport to add a cube. "
                " Use the Scene Collection to select objects and "
                "the Object Explorer to edit their properties."
                " To delete an object select the object in Scene Collection right click press delete."
				" Texture Tab To edit materials and textures."
            );

            ImGui::Spacing();

            ImGui::BulletText("Add objects right click in the viewport.");

            ImGui::BulletText("Select objects by clicking them in the viewport.");

            ImGui::BulletText("Use the Object Explorer for precise transforms.");

            

            ImGui::BulletText("To move the object use object mode press move x,y,z to move.");

            ImGui::BulletText("To Scale the object use object mode press scale x,y,z to scale.");

            ImGui::BulletText("To Rotate the object use object mode press rotate x,y,z to rotate.");

            ImGui::TextWrapped(
                "To edit an object use edit mode select the vertex button "
                "select a vertex press x to move on x, press y to move on y, press z to move on z. "
                " Or use the edge button then select an edge to manipulate it."
                " Press x to move on x, press y to move on y, press z to move on z."
                " The face button works the same way."
            );
			// Helper Function to center text with color
            g_helpers.CenterTextColored(COLOR_LIGHTGREEN, ICON_FA_CAMERA" Camera Controls");

			ImGui::Separator();
            if (ImGui::BeginTable(
                "##ShortcutTableCamera",
                2,
                ImGuiTableFlags_Borders |
                ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn(ICON_FA_MOUSE" Mouse");

                ImGui::TableSetupColumn("Camera Action");
                ImGui::TableHeadersRow();

                const auto AddShortcut =
                    [](const char*  key,
                        const char* action)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    //ImGui::TextUnformatted(key);
					ImGui::TextColored(COLOR_LIGHTGREEN, key);

                    ImGui::TableSetColumnIndex(1);
                   // ImGui::TextUnformatted(action);
                    ImGui::TextColored(COLOR_LIGHTGREEN, action);
                };
                AddShortcut(
                    "Middle Mouse Button Down",
                    "Orbit Camera"
                );
                AddShortcut(
                    "Middle Mouse Down + Shift",
                    "Camera Left/Right Up/Down"
                );
                AddShortcut(
                    "Middle Mouse Button Scrole",
                    "Camera Zoom In/Out"
                );

                ImGui::EndTable();
            }






            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(
            "Modes"))
        {
            ImGui::SeparatorText(
                "Select Mode"
            );

            ImGui::TextWrapped(
                "Used for selecting objects without performing "
                "an active transform."
            );

            ImGui::SeparatorText(
                "Object Mode"
            );

            ImGui::TextWrapped(
                "Used for moving, rotating and scaling complete objects."
            );

            ImGui::SeparatorText(
                "Edit Mode"
            );

            ImGui::TextWrapped(
                "Used for editing mesh vertices, edges and faces."
            );

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(ICON_FA_TOOLS" Tools"))
        {
            ImGui::TextWrapped(
                "Select an object and enter Object Mode."
            );

            ImGui::Spacing();

            ImGui::BulletText(
                "Click the Move tool."
            );

            ImGui::BulletText(
                "Press X to move along the X axis."
            );

            ImGui::BulletText(
                "Press Y to move along the Y axis."
            );

            ImGui::BulletText(
                "Press Z to move along the Z axis."
            );

            ImGui::BulletText(
                "Move the mouse to reposition the object."
            );

            ImGui::BulletText(
                "Left-click to confirm."
            );

            ImGui::BulletText(
                "Right-click "
            );

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(
            "Materials"))
        {
            ImGui::TextWrapped(
                "Select an object and open the Material Editor "
                "inside the Object Explorer."
            );

            ImGui::Spacing();

            ImGui::BulletText(
                "Base Color changes the material colour."
            );

            ImGui::BulletText(
                "Metallic controls whether the material behaves like metal."
            );

            ImGui::BulletText(
                "Roughness controls how sharp or soft highlights appear."
            );

            ImGui::BulletText(
                "Load Texture opens an image file and assigns it "
                "to the selected object's material."
            );

            ImGui::BulletText(
                "The Base Color can also tint the loaded texture."
            );

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Shortcuts"))
        {
            ImGui::SeparatorText(ICON_FA_CAMERA" Camera Controls");

            if (ImGui::BeginTable(
                "##ShortcutTableCamera",
                2,
                ImGuiTableFlags_Borders |
                ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn(ICON_FA_MOUSE" Mouse");

                ImGui::TableSetupColumn("Camera Action");
                ImGui::TableHeadersRow();

                const auto AddShortcut =
                    [](const char* key,
                        const char* action)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(key);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(action);
                };
                AddShortcut(
                    "Middle Mouse Button Down",
                    "Orbit Camera"
                );
                AddShortcut(
                    "Middle Mouse Down + Shift",
                    "Camera Left/Right Up/Down"
                );
                AddShortcut(
                    "Middle Mouse Button Scrole",
                    "Camera Zoom In/Out"
                );

                ImGui::EndTable();
            }

            ImGui::SeparatorText(ICON_FA_ARROWS_ALT" Move Controls");

            if (ImGui::BeginTable(
                "##ShortcutTableMove",
                2,
                ImGuiTableFlags_Borders |
                ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn(ICON_FA_KEYBOARD" Key");

                ImGui::TableSetupColumn(
                    "Move Action"
                );

                ImGui::TableHeadersRow();

                const auto AddShortcut =
                    [](const char* key,
                        const char* action)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(key);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(action);
                };
                AddShortcut(
                    "In Object Mode",
                    "Select Move Button"
                );

                AddShortcut(
                    "X",
                    "Move along the X axis"
                );

                AddShortcut(
                    "Y",
                    "Move along the Y axis"
                );

                AddShortcut(
                    "Z",
                    "Move along the Z axis"
                );

                AddShortcut(
                    "Left Mouse",
                    "Confirm transform"
                );

                AddShortcut(
                    "Right Mouse",
                    "Cancel transform"
                );
                
                
                ImGui::EndTable();
            }
			// next table for scale tool
			ImGui::SeparatorText(ICON_FA_EXPAND" Scale Controls");

            if (ImGui::BeginTable(
                "##ShortcutTableScale",
                2,
                ImGuiTableFlags_Borders |
                ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn(ICON_FA_KEYBOARD" Key");

                ImGui::TableSetupColumn("Scale Action");
                ImGui::TableHeadersRow();

                const auto AddShortcut =
                    [](const char* key,
                        const char* action)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(key);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(action);
                };
                AddShortcut(
                    "In Object Mode",
                    "Select Scale Button"
                );

                AddShortcut(
                    "X",
                    "Scale along the X axis"
                );
                AddShortcut(
                    "Y",
                    "Scale along the Y axis"
                );
                AddShortcut(
                    "Z",
                    "Scale along the Z axis"
                );
                AddShortcut(
                    "Left Mouse",
                    "Confirm transform"
                );
                AddShortcut(
                    "Right Mouse",
                    "Cancel transform"
                );

                ImGui::EndTable();
            }

            // next table for Rotate tool
            //ImGui::SeparatorText(ICON_FA_ARROW_ALT_CIRCLE_LEFT" Rotate Controls");
            ImGui::SeparatorText(ICON_FA_COG" Rotate Controls");

            if (ImGui::BeginTable(
                "##ShortcutTableRotate",
                2,
                ImGuiTableFlags_Borders |
                ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn(ICON_FA_KEYBOARD" Key");

                ImGui::TableSetupColumn("Scale Action");
                ImGui::TableHeadersRow();

                const auto AddShortcut =
                    [](const char* key,
                        const char* action)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(key);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(action);
                };
                AddShortcut(
                    "In Object Mode",
                    "Select Rotate Button"
                );

                AddShortcut(
                    "X",
                    "Rotate along the X axis"
                );
                AddShortcut(
                    "Y",
                    "Rotate along the Y axis"
                );
                AddShortcut(
                    "Z",
                    "Rotate along the Z axis"
                );
                AddShortcut(
                    "Left Mouse",
                    "Confirm transform"
                );
                AddShortcut(
                    "Right Mouse",
                    "Cancel transform"
                );

                ImGui::EndTable();
            }

			ImGui::SeparatorText(ICON_FA_MAP_SIGNS" Other Shortcuts");

			// Other shortcuts can be added here as needed.
            if (ImGui::BeginTable(
                "##ShortcutTableOther",
                2,
                ImGuiTableFlags_Borders |
                ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn(ICON_FA_KEYBOARD" Key");

                ImGui::TableSetupColumn("Other Action");
                ImGui::TableHeadersRow();

                const auto AddShortcut =
                    [](const char* key,
                        const char* action)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(key);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(action);
                };
                AddShortcut(
                    "Escape",
                    "Close application & go to bed"
                );

                ImGui::EndTable();
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}