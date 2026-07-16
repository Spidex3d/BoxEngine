#include "panels\SceneViewportPanel.h"
#include <BoxEngine.h>
#include <imgui\ImGuiAF.h>
#include <miniBoxLog.h>

//void SceneViewportPanel::DrawSceneViewport()
//{
//    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
//    ImGui::Begin("Main Scene");
//
//    // Available size in UI units
//    const float window_width = ImGui::GetContentRegionAvail().x;
//    const float window_height = ImGui::GetContentRegionAvail().y;
//
//    ImVec2 pos = ImGui::GetCursorScreenPos();
//    ImGuiIO& io = ImGui::GetIO();
//
//    int desired_w = static_cast<int>(window_width * io.DisplayFramebufferScale.x);
//    int desired_h = static_cast<int>(window_height * io.DisplayFramebufferScale.y);
//
//
//    // ########################################## Top Tool bar ############################################
//        const float tbHeight = 25.0f;
//        ImGui::BeginChild("##scene_toolbar", ImVec2(ImGui::GetContentRegionAvail().x, tbHeight), false, ImGuiWindowFlags_NoDecoration);
//
//        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));
//        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 4));
//
//
//        ImGui::PushID("top_Buttons");
//
//        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // normal
//
//        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.16f, 0.70f, 0.16f, 1.0f)); // hover
//        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.50f, 0.10f, 1.0f)); // active/click
//        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.8f, 1.0f)); // active/click
//
//        ImGui::GetStyle().FrameBorderSize = 0.3f; // Add a border to the button
//        ImGui::GetStyle().FrameRounding = 6.0f; // rounded corners of buttons
//
//        
//
//        const char* items[] = { "Select Mode", ICON_FA_CUBE " Object Mode", ICON_FA_VECTOR_SQUARE " Edit Mode" };
//        ImGui::SetNextItemWidth(100.0f);
//        ImGui::SameLine();
//        if (ImGui::Combo("##combo", &m_EditMode, items, IM_ARRAYSIZE(items))) {
//            if (m_EditMode == 1) {
//                if (m_actionCallback) m_actionCallback("AddObjectMode");
//
//                BOX_LOG_INFO("Object mode selected");
//            }
//            else if (m_EditMode == 2) {
//                if (m_actionCallback) m_actionCallback("AddEditMode");
//
//                BOX_LOG_INFO("Edit mode selected");
//            }
//        }
//        ImGui::SameLine();
//
//        ImGui::PopStyleVar(2);
//        ImGui::PopStyleColor(4); // pop all 4 pushed colors has to match top
//        ImGui::PopID();
//
//        ImGui::EndChild();
//
//        // ########################################## End Top Tool bar ############################################
//      
//
//        // Detect right-click for popup menu (existing UI code)
//        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
//        {
//            ImGui::OpenPopup("RightClickMenu");
//
//        }
//
//        if (ImGui::BeginPopup("RightClickMenu"))
//        {
//
//            if (ImGui::BeginMenu("Add a model")) {
//                if (ImGui::MenuItem("Obj Model")) {
//                    // Request engine to add a Obj via action callback
//
//                }
//
//                if (ImGui::MenuItem("Gltf Model")) {
//                    // Request engine to add a Gltf via action callback
//
//                }
//                // other menu items...
//                ImGui::EndMenu();
//            }
//
//            if (ImGui::BeginMenu("Add a new mesh")) {
//
//                if (ImGui::MenuItem("Editable Cube")) {
//                    // Request engine to add a cube via action callback
//
//                    //if (m_actionCallback) m_actionCallback("AddLight:Ambient");
//
//                }
//
//                if (ImGui::MenuItem("Editable Plane")) {
//                    // Request engine to add a plane via action callback
//
//
//                }
//
//                if (ImGui::MenuItem("Editable UV Sphere")) {
//                    // Request engine to add a plane via action callback
//                    //  if (m_actionCallback) m_actionCallback("AddEditableUVSphere");
//
//                }
//
//                if (ImGui::MenuItem("Editable ICO Sphere")) {
//                    // Request engine to add a plane via action callback
//                    //if (m_actionCallback) m_actionCallback("AddEditableICOSphere");
//                }
//
//                if (ImGui::MenuItem("Editable Cylinder")) {
//                    // Request engine to add a plane via action callback
//                    //if (m_actionCallback) m_actionCallback("AddEditableCylinder");
//                }
//
//                if (ImGui::MenuItem("Editable Cone")) {
//                    // Request engine to add a plane via action callback
//                   // if (m_actionCallback) m_actionCallback("AddEditableCone");
//                }
//
//                if (ImGui::MenuItem("Editable Torus")) {
//                    // Request engine to add a plane via action callback
//                   // if (m_actionCallback) m_actionCallback("AddEditableTorus");
//                }
//                // other menu items...
//                ImGui::EndMenu();
//            }
//            // Lighting call to action callback to add a new light to the scene,
//            // the type of light will be determined by the menu item clicked and passed as a string argument to the callback
//            if (ImGui::BeginMenu("Add a new Light")) {
//                /*if (ImGui::MenuItem("Ambient")) {
//                    if (m_actionCallback) m_actionCallback("AddLight:Ambient");
//
//                }*/
//
//                if (ImGui::MenuItem("Spot")) {
//                }
//
//                if (ImGui::MenuItem("Area")) {
//                    //  if (m_actionCallback) m_actionCallback("AddLight:Area");
//                }
//                ImGui::EndMenu();
//            }
//
//
//            if (ImGui::BeginMenu("Sky")) {
//
//                if (ImGui::MenuItem("Add Sky Box")) {
//
//                }
//                if (ImGui::MenuItem("Add Sky Sphere ")) {
//                    // if (m_actionCallback) m_actionCallback("AddSkySpere");
//                }
//                ImGui::EndMenu();
//            }
//
//            if (ImGui::BeginMenu("Reset Camera")) {
//                if (ImGui::MenuItem("Reset Camera Position")) {
//                    // reset camera position
//
//                }
//                ImGui::EndMenu();
//            }
//
//            ImGui::EndPopup();
//        }
//
//        // ################################################################################################
//        const ImVec2 availableSize = ImGui::GetContentRegionAvail();
//
//        const ImGuiIO& io = ImGui::GetIO();
//
//        const int desiredWidth = static_cast<int>(
//            availableSize.x * io.DisplayFramebufferScale.x
//            );
//
//        const int desiredHeight = static_cast<int>(
//            availableSize.y * io.DisplayFramebufferScale.y
//            );
//
//        if (desiredWidth > 0 && desiredHeight > 0)
//        {
//            engine.ResizeSceneViewport(desiredWidth, desiredHeight);
//            engine.RenderScene();
//
//            const GLuint sceneTexture = engine.GetSceneTexture();
//
//            if (sceneTexture != 0)
//            {
//                ImGui::Image(
//                    reinterpret_cast<ImTextureID>(
//                        static_cast<intptr_t>(sceneTexture)
//                        ),
//                    availableSize,
//                    ImVec2(0, 1),
//                    ImVec2(1, 0)
//                );
//
//                m_sceneViewportPos = ImGui::GetItemRectMin();
//                m_sceneViewportSize = ImGui::GetItemRectSize();
//            }
//            else
//            {
//                ImGui::TextUnformatted(
//                    "Scene framebuffer is unavailable."
//                );
//            }
//        }
//        // ################################################################################################
//
//    ImGui::PopStyleVar(1);
//    //ImGui::PopStyleColor(4); // pop all 4 pushed colors has to match top
//    //ImGui::PopID();
//
//   
//
//    ImGui::End();
//
//}

void SceneViewportPanel::DrawSceneViewport(BoxEngine& engine)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
    ImGui::Begin("Main Scene");

    // Available size in UI units
    const float window_width = ImGui::GetContentRegionAvail().x;
    const float window_height = ImGui::GetContentRegionAvail().y;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGuiIO& io = ImGui::GetIO();

    int desired_w = static_cast<int>(window_width * io.DisplayFramebufferScale.x);
    int desired_h = static_cast<int>(window_height * io.DisplayFramebufferScale.y);


    // ########################################## Top Tool bar ############################################
    const float tbHeight = 25.0f;
    ImGui::BeginChild("##scene_toolbar", ImVec2(ImGui::GetContentRegionAvail().x, tbHeight), false, ImGuiWindowFlags_NoDecoration);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 4));


    ImGui::PushID("top_Buttons");

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // normal

    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.16f, 0.70f, 0.16f, 1.0f)); // hover
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.50f, 0.10f, 1.0f)); // active/click
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.8f, 1.0f)); // active/click

    ImGui::GetStyle().FrameBorderSize = 0.3f; // Add a border to the button
    ImGui::GetStyle().FrameRounding = 6.0f; // rounded corners of buttons



    const char* items[] = { "Select Mode", ICON_FA_CUBE " Object Mode", ICON_FA_VECTOR_SQUARE " Edit Mode" };
    ImGui::SetNextItemWidth(100.0f);
    ImGui::SameLine();
    if (ImGui::Combo("##combo", &m_EditMode, items, IM_ARRAYSIZE(items))) {
        if (m_EditMode == 1) {
            if (m_actionCallback) m_actionCallback("AddObjectMode");

            BOX_LOG_INFO("Object mode selected");
        }
        else if (m_EditMode == 2) {
            if (m_actionCallback) m_actionCallback("AddEditMode");

            BOX_LOG_INFO("Edit mode selected");
        }
    }
    ImGui::SameLine();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4); // pop all 4 pushed colors has to match top
    ImGui::PopID();

    ImGui::EndChild();

    // ########################################## End Top Tool bar ############################################


    // Detect right-click for popup menu (existing UI code)
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        ImGui::OpenPopup("RightClickMenu");

    }

    if (ImGui::BeginPopup("RightClickMenu"))
    {

        if (ImGui::BeginMenu("Add a model")) {
            if (ImGui::MenuItem("Obj Model")) {
                // Request engine to add a Obj via action callback

            }

            if (ImGui::MenuItem("Gltf Model")) {
                // Request engine to add a Gltf via action callback

            }
            // other menu items...
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Add a new mesh")) {

            if (ImGui::MenuItem("Editable Cube")) {
                // Request engine to add a cube via action callback

                //if (m_actionCallback) m_actionCallback("AddLight:Ambient");

            }

            if (ImGui::MenuItem("Editable Plane")) {
                // Request engine to add a plane via action callback


            }

            if (ImGui::MenuItem("Editable UV Sphere")) {
                // Request engine to add a plane via action callback
                //  if (m_actionCallback) m_actionCallback("AddEditableUVSphere");

            }

            if (ImGui::MenuItem("Editable ICO Sphere")) {
                // Request engine to add a plane via action callback
                //if (m_actionCallback) m_actionCallback("AddEditableICOSphere");
            }

            if (ImGui::MenuItem("Editable Cylinder")) {
                // Request engine to add a plane via action callback
                //if (m_actionCallback) m_actionCallback("AddEditableCylinder");
            }

            if (ImGui::MenuItem("Editable Cone")) {
                // Request engine to add a plane via action callback
               // if (m_actionCallback) m_actionCallback("AddEditableCone");
            }

            if (ImGui::MenuItem("Editable Torus")) {
                // Request engine to add a plane via action callback
               // if (m_actionCallback) m_actionCallback("AddEditableTorus");
            }
            // other menu items...
            ImGui::EndMenu();
        }
        // Lighting call to action callback to add a new light to the scene,
        // the type of light will be determined by the menu item clicked and passed as a string argument to the callback
        if (ImGui::BeginMenu("Add a new Light")) {
            /*if (ImGui::MenuItem("Ambient")) {
                if (m_actionCallback) m_actionCallback("AddLight:Ambient");

            }*/

            if (ImGui::MenuItem("Spot")) {
            }

            if (ImGui::MenuItem("Area")) {
                //  if (m_actionCallback) m_actionCallback("AddLight:Area");
            }
            ImGui::EndMenu();
        }


        if (ImGui::BeginMenu("Sky")) {

            if (ImGui::MenuItem("Add Sky Box")) {

            }
            if (ImGui::MenuItem("Add Sky Sphere ")) {
                // if (m_actionCallback) m_actionCallback("AddSkySpere");
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Reset Camera")) {
            if (ImGui::MenuItem("Reset Camera Position")) {
                // reset camera position

            }
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }

    // ################################################################################################
    const ImVec2 availableSize = ImGui::GetContentRegionAvail();

    //const ImGuiIO& io = ImGui::GetIO();

    const int desiredWidth = static_cast<int>(
        availableSize.x * io.DisplayFramebufferScale.x
        );

    const int desiredHeight = static_cast<int>(
        availableSize.y * io.DisplayFramebufferScale.y
        );

    if (desiredWidth > 0 && desiredHeight > 0)
    {
        engine.ResizeSceneViewport(desiredWidth, desiredHeight);
        engine.RenderScene();

        const GLuint sceneTexture = engine.GetSceneTexture();

        if (sceneTexture != 0)
        {
            ImGui::Image(
                reinterpret_cast<ImTextureID>(
                    static_cast<intptr_t>(sceneTexture)
                    ),
                availableSize,
                ImVec2(0, 1),
                ImVec2(1, 0)
            );

            m_sceneViewportPos = ImGui::GetItemRectMin();
            m_sceneViewportSize = ImGui::GetItemRectSize();
        }
        else
        {
            ImGui::TextUnformatted(
                "Scene framebuffer is unavailable."
            );
        }
    }
    // ################################################################################################

    ImGui::PopStyleVar(1);
    //ImGui::PopStyleColor(4); // pop all 4 pushed colors has to match top
    //ImGui::PopID();



    ImGui::End();

}
