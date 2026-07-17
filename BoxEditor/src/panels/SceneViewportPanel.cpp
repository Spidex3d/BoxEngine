#include "panels\SceneViewportPanel.h"
#include <BoxEngine.h>
#include <imgui\ImGuiAF.h>
#include <BoxDiffs.h>
#include <miniBoxLog.h>
#include <Helpers.h>
#include <UI\EditorIcons.h>


ViewportAction SceneViewportPanel::DrawSceneViewport(BoxEngine& engine, const EditorIcons& icons)
{
   
    ViewportAction action = ViewportAction::None;
    Helpers helpers;

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
            action = ViewportAction::SetObjectMode;
            BOX_LOG_INFO("Object mode selected");
        }
        else if (m_EditMode == 2) {
            action = ViewportAction::SetEditMode;
            BOX_LOG_INFO("Edit mode selected");
        }
    }
    ImGui::SameLine();
    // ################################################## Buttons #########################################
    const EditorTexture& vertexIcon =
        icons.GetVertexIcon();

    const EditorTexture& edgeIcon =
        icons.GetEdgeIcon();

    const EditorTexture& faceIcon =
        icons.GetFaceIcon();

    // keep an int for current edit target: 0 = vertex, 1 = edge, 2 = face
    // If you already have a member, use that one instead.
    static int m_editType = 0;
    const ImVec2 editTypeSize(16, 16);
    const ImVec2 iconSize(16, 16);
    // AddVertexMode AddEdgeMode AddFaceMode
    ImGui::SameLine();
    ImGui::PushID("editTargetIcons");
    // ########################
    if (ImGui::ImageButton("##VertexTool", reinterpret_cast<ImTextureID>(static_cast<intptr_t>(vertexIcon.id)),iconSize))
    {
        m_editType = 0;
        // action = ViewportAction::VertexEditMode;
    }

    if (m_editType == 0)
    {
        const ImVec2 min = ImGui::GetItemRectMin();
        const ImVec2 max = ImGui::GetItemRectMax();

        ImGui::GetWindowDrawList()->AddRect(
            min,
            max,
            IM_COL32(50, 150, 255, 255),
            4.0f,
            0,
            2.0f
        );
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Vertex Edit");
    }
    
    // draw selection outline if active
    if (m_editType == 0) {
        ImVec2 a = ImGui::GetItemRectMin();
        ImVec2 b = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddRect(a, b, IM_COL32(50, 150, 255, 255), 4.0f, 0, 2.0f);
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Vertex Edit");
    ImGui::SameLine();
    // Edge
    if (ImGui::ImageButton("##EdgeTool", reinterpret_cast<ImTextureID>(static_cast<intptr_t>(edgeIcon.id)), iconSize))
    {
        m_editType = 1;
        // action = ViewportAction::EdgeEditMode;
    }

    if (m_editType == 1)
    {
        const ImVec2 min = ImGui::GetItemRectMin();
        const ImVec2 max = ImGui::GetItemRectMax();

        ImGui::GetWindowDrawList()->AddRect(
            min,
            max,
            IM_COL32(50, 150, 255, 255),
            4.0f,
            0,
            2.0f
        );
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Edge Edit");
    }
    
    if (m_editType == 1) {
        ImVec2 a = ImGui::GetItemRectMin();
        ImVec2 b = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddRect(a, b, IM_COL32(50, 150, 255, 255), 4.0f, 0, 2.0f);
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Edge Edit");
    ImGui::SameLine();
    // Face
    if (ImGui::ImageButton("##FaceTool", reinterpret_cast<ImTextureID>(static_cast<intptr_t>(faceIcon.id)), iconSize))
    {
        m_editType = 2;
        // action = ViewportAction::FaceEditMode;
    }

    if (m_editType == 2)
    {
        const ImVec2 min = ImGui::GetItemRectMin();
        const ImVec2 max = ImGui::GetItemRectMax();

        ImGui::GetWindowDrawList()->AddRect(
            min,
            max,
            IM_COL32(50, 150, 255, 255),
            4.0f,
            0,
            2.0f
        );
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Face Edit");
    }
    
    if (m_editType == 2) {
        ImVec2 a = ImGui::GetItemRectMin();
        ImVec2 b = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddRect(a, b, IM_COL32(50, 150, 255, 255), 4.0f, 0, 2.0f);
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Face Edit");

    ImGui::PopID();

    ImGui::SameLine();

	// ################################################## End Buttons #########################################

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
				action = ViewportAction::AddObjModel;
            }

            if (ImGui::MenuItem("Gltf Model")) {
				action = ViewportAction::AddGltfModel;
            }
            // other menu items...
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Add a new mesh")) {

            if (ImGui::MenuItem("Editable Cube")) {
                action = ViewportAction::AddEditableCube;
            }

            if (ImGui::MenuItem("Editable Plane")) {
				action = ViewportAction::AddEditablePlane;
            }

            if (ImGui::MenuItem("Editable UV Sphere")) {
				action = ViewportAction::AddEditableUVSphere;
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

    // ############################################ Framebuffer ##############################################
    const ImVec2 availableSize = ImGui::GetContentRegionAvail();

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

    ImGui::End();
    ImGui::PopStyleVar(1);
    //ImGui::PopStyleColor(4); // pop all 4 pushed colors has to match top
    //ImGui::PopID();


    return action;
}


void SceneViewportPanel::Shutdown() {
    // Cleanup code here (if needed)
   /* DestroyEditorTexture(m_vertexIcon);
    DestroyEditorTexture(m_edgeIcon);
    DestroyEditorTexture(m_faceIcon);*/
   // DestroyEditorTexture(m_scaleIcon);
}
