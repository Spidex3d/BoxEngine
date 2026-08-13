#include "panels\SceneViewportPanel.h"
#include <BoxEngine.h>
#include <entity/Entity.h> // added for tool
#include <imgui\ImGuiAF.h>
#include <BoxDiffs.h>
#include <miniBoxLog.h>
#include <Helpers.h>
#include <UI\EditorIcons.h>
#include <camera/Camera.h>
#include <tools\VertexEditController.h>
#include <mesh\modifiers\FaceInset.h>



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
    const float tbHeight = 28.0f;
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
        else
        {
            if (m_transformTools.IsTransforming())
            {
                m_transformTools.CancelTransform(engine);
            }

            action = ViewportAction::SetSelectMode;

            BOX_LOG_INFO(
                "Select mode selected"
            );
        }

    }
    ImGui::SameLine();
	// ###########################################################################################################
    // ################################################## Editor Buttons #########################################
	// ###########################################################################################################
    const EditorTexture& vertexIcon = icons.GetVertexIcon();
    const EditorTexture& edgeIcon = icons.GetEdgeIcon();
    const EditorTexture& faceIcon = icons.GetFaceIcon();

    // keep an int for current edit target: 0 = vertex, 1 = edge, 2 = face
    // If you already have a member, use that one instead.
    
    const ImVec2 editTypeSize(18, 18);
    const ImVec2 iconSize(18, 18);
    // AddVertexMode AddEdgeMode AddFaceMode
    ImGui::SameLine();

	const bool EditModeActive = m_EditMode == 2; // set it editing mode active if the combo box is set to edit mode

    if (!EditModeActive)
    {
        ImGui::BeginDisabled();
    }

    ImGui::PushID("editTargetIcons");
    // ########################
    if (ImGui::ImageButton("##VertexTool", reinterpret_cast<ImTextureID>(static_cast<intptr_t>(vertexIcon.id)),iconSize))
    {
        m_editType = 0;
        // action = ViewportAction::VertexEditMode;
    }

    if (m_editType == 0)
    {
        TransformToolBarColors();
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Vertex Edit");
    }
    
    ImGui::SameLine();
    // Edge
    if (ImGui::ImageButton("##EdgeTool", reinterpret_cast<ImTextureID>(static_cast<intptr_t>(edgeIcon.id)), iconSize))
    {
        m_editType = 1;
        // action = ViewportAction::EdgeEditMode;
    }

    if (m_editType == 1)
    {
        TransformToolBarColors();
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Edge Edit");
    }
    
    ImGui::SameLine();
    // Face
    if (ImGui::ImageButton("##FaceTool", reinterpret_cast<ImTextureID>(static_cast<intptr_t>(faceIcon.id)), iconSize))
    {
        m_editType = 2;
        // action = ViewportAction::FaceEditMode;
    }

    if (m_editType == 2)
    {
        TransformToolBarColors();
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Face Edit");
    }
   
    if (!EditModeActive)
    {
        ImGui::EndDisabled();
    }

    ImGui::PopID();
    ImGui::SameLine();

	// ################################################## End Edit Buttons #########################################
	
    // #############################################################################################################
	// ############################################### Transform Tools buttons #####################################
    // #############################################################################################################
	const EditorTexture& moveIcon = icons.GetMoveToolIcon();
	const EditorTexture& scaleIcon = icons.GetScaleToolIcon();
	const EditorTexture& rotateIcon = icons.GetRotateToolIcon();
	const EditorTexture& localIcon = icons.GetLocalToolIcon();
	const EditorTexture& snapIcon = icons.GetSnapToolIcon();
	const EditorTexture& dropIcon = icons.GetDropToolIcon();
    ImGui::PushID("TransformIcons");

	const bool ObjectModeActive = m_EditMode == 1; // set it object mode active if the combo box is set to object mode

    if (!ObjectModeActive)
    {
        ImGui::BeginDisabled();
    }

    // Move
    ImGui::SameLine();
    if (ImGui::ImageButton("##MoveTool", reinterpret_cast<ImTextureID>(static_cast<intptr_t>(moveIcon.id)), iconSize))
    {
        m_transformTools.SetActiveTool(TransformToolType::Move);
        
    }
    if (m_transformTools.GetActiveTool() == TransformToolType::Move)
    {
        TransformToolBarColors();
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Move Tool");
    }
	// Scale
	ImGui::SameLine();
    if (ImGui::ImageButton("##ScaleTool", reinterpret_cast<ImTextureID>(static_cast<intptr_t>(scaleIcon.id)), iconSize))
    {
        m_transformTools.SetActiveTool(TransformToolType::Scale);
    }
    if (m_transformTools.GetActiveTool() == TransformToolType::Scale)
    {
        TransformToolBarColors();
       
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Scale Tool");
    }
	// Rotate
    ImGui::SameLine();
    if (ImGui::ImageButton("##RotateTool", reinterpret_cast<ImTextureID>(static_cast<intptr_t>(rotateIcon.id)), iconSize))
    {
		m_transformTools.SetActiveTool(TransformToolType::Rotate);
    }
    if (m_transformTools.GetActiveTool() == TransformToolType::Rotate)
    {
        TransformToolBarColors();
        
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Rotate Tool");
    }
	// Local
    ImGui::SameLine();
    if (ImGui::ImageButton("##LocalTool", reinterpret_cast<ImTextureID>(static_cast<intptr_t>(localIcon.id)), iconSize))
    {
		m_transformTools.SetActiveTool(TransformToolType::Local);
    }
    if (m_transformTools.GetActiveTool() == TransformToolType::Local)
    {
        TransformToolBarColors();
        
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Local Tool");
    }
	// Snap
    ImGui::SameLine();
    if (ImGui::ImageButton("##SnapTool", reinterpret_cast<ImTextureID>(static_cast<intptr_t>(snapIcon.id)), iconSize))
    {
		m_transformTools.SetActiveTool(TransformToolType::Snap);
    }
    if (m_transformTools.GetActiveTool() == TransformToolType::Snap)
    {
        TransformToolBarColors();
        
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Snap Tool");
    }
	// Drop the object to the ground plane / grid
    ImGui::SameLine();
    if (ImGui::ImageButton("##DropTool", reinterpret_cast<ImTextureID>(static_cast<intptr_t>(dropIcon.id)), iconSize))
    {
        m_transformTools.DropToGrid(engine);
    }
    if (m_transformTools.GetActiveTool() == TransformToolType::Drop)
    {
        TransformToolBarColors();

    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Drop Tool");
    }

    if (!ObjectModeActive)
    {
        ImGui::EndDisabled();
    }

	ImGui::PopID();

    
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4); // pop all 4 pushed colors has to match top
    ImGui::PopID();

    ImGui::EndChild();

    // ########################################################################################################
    // ########################################## End Top Tool bar ############################################
    // ########################################################################################################

    // Detect right-click for popup menu (existing UI code)
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        ImGui::OpenPopup("RightClickMenu");

    }

    if (ImGui::BeginPopup("RightClickMenu"))
    {

        if (ImGui::BeginMenu("Add a model")) {
            if (ImGui::MenuItem("mbx Model")) {
                action = ViewportAction::AddMbxModel;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Obj Model")) {
				action = ViewportAction::AddObjModel;
            }
            ImGui::Separator();
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

            if (ImGui::MenuItem("Editable Sphere")) {
				action = ViewportAction::AddEditableSphere;
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
            // Pyramid
            if (ImGui::MenuItem("Editable Pyramid")) {
                // Request engine to add a plane via action callback
				action = ViewportAction::AddEditablePyramid;
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

            if (ImGui::MenuItem("Ambient")) {
                //if (m_actionCallback) m_actionCallback("AddLight:Ambient");

            }

            if (ImGui::MenuItem("Spot")) {
            }

            if (ImGui::MenuItem("Area")) {
                //  if (m_actionCallback) m_actionCallback("AddLight:Area");
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Modifiers")) {

            if (ImGui::MenuItem("Extrude"))
            {

                Entity* selectedEntity =
                    engine.GetSelectedEntity();

                if (!selectedEntity)
                {
                    BOX_LOG_WARNING(
                        "Extrude: no entity selected"
                    );
                }
                else if (
                    !m_faceEditController.HasSelectedFace())
                {
                    BOX_LOG_WARNING(
                        "Extrude: no face selected"
                    );
                }
                else
                {
                    m_faceEditController.BeginExtrude(
                        *selectedEntity
                    );
                }


            }
                   

            if (ImGui::MenuItem("Inset"))
            {

                Entity* selectedEntity = engine.GetSelectedEntity();

                if (!selectedEntity)
                {
                    BOX_LOG_WARNING(
                        "Inset: no entity selected"
                    );
                }
                else if (
                    !m_faceEditController.HasSelectedFace())
                {
                    BOX_LOG_WARNING(
                        "Inset: no face selected"
                    );
                }
                else
                {
                    m_faceEditController.BeginInset(*selectedEntity);
                }

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

    // #######################################################################################################
    // ############################################ Framebuffer ##############################################
    // #######################################################################################################
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

        const ImVec2 imageTopLeft = ImGui::GetCursorScreenPos(); // picking

		// draw the scene texture to the ImGui window from the framebuffer, if the texture is valid
        if (sceneTexture != 0)
        {
            ImGui::Image(
                reinterpret_cast<ImTextureID>(static_cast<intptr_t>(sceneTexture)),
                availableSize, ImVec2(0, 1), ImVec2(1, 0));

            m_sceneViewportPos = ImGui::GetItemRectMin();
            m_sceneViewportSize = ImGui::GetItemRectSize();

            // ############################################################################################
            // #################################### Transforme tools ######################################
            // ############################################################################################
            const bool viewportHovered = ImGui::IsItemHovered();
            const bool objectModeActive = m_EditMode == 1;
            m_transformTools.HandleInput(engine, viewportHovered, objectModeActive);

			const bool wasTransforming = m_transformTools.IsTransforming();
            // ############################################################################################
            // #################################### Editing tools #########################################
            // ############################################################################################
            const bool editModeActive = m_EditMode == 2;
			const bool vertexModeActive = editModeActive && m_editType == 0;    // combine edit mode and vertex edit type

			const bool edgeModeActive = editModeActive && m_editType == 1;      // combine edit mode and edge edit type
			
            const bool faceModeActive = editModeActive && m_editType == 2;      // combine edit mode and face edit type
                       

            // vertex
            m_vertexEditController.HandleInput(engine, viewportHovered, vertexModeActive, m_sceneViewportPos, m_sceneViewportSize);
            m_vertexEditController.DrawVertices(engine, m_sceneViewportPos, m_sceneViewportSize, vertexModeActive);
            // edges
            m_edgeEditController.HandleInput(engine, viewportHovered, edgeModeActive, m_sceneViewportPos, m_sceneViewportSize);
            m_edgeEditController.DrawEdge(engine, m_sceneViewportPos, m_sceneViewportSize, edgeModeActive);
			// faces
			m_faceEditController.HandleInput(engine, viewportHovered, faceModeActive, m_sceneViewportPos, m_sceneViewportSize);
			m_faceEditController.DrawFace(engine, m_sceneViewportPos, m_sceneViewportSize, faceModeActive);

            // ############################################################################################
            // ################################# Modifiers ############################################
            // ############################################################################################


            // ############################################################################################
            // ################################# Mouse Picking ############################################
            // ############################################################################################
            // This prevents the tool confirmation click from performing another picking operation.
            
            const bool viewportClicked = !wasTransforming && viewportHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);

            if (viewportClicked)
            {
                const ImVec2 mousePosition =
                    ImGui::GetMousePos();

                const float mouseX =
                    mousePosition.x -
                    m_sceneViewportPos.x;

                const float mouseY =
                    mousePosition.y -
                    m_sceneViewportPos.y;

                Camera& camera = engine.GetCamera();

                const glm::mat4 view = camera.GetViewMatrix();

                const float aspectRatio =
                    m_sceneViewportSize.y > 0.0f
                    ? m_sceneViewportSize.x / m_sceneViewportSize.y
                    : 1.0f;

                const glm::mat4 projection =
                    camera.GetProjectionMatrix(
                        aspectRatio
                    );

                const glm::vec3 rayDirection =
                    CreateMouseRay(
                        mouseX,
                        mouseY,
                        m_sceneViewportSize.x,
                        m_sceneViewportSize.y,
                        view,
                        projection
                    );

                engine.PickEntity(camera.Position, rayDirection
                );
            }
		    // ############################################# Mouse Picking end ############################################
			
        }
   
    }

    ImGui::End();
    ImGui::PopStyleVar(1);
   
    return action;
}


glm::vec3 SceneViewportPanel::CreateMouseRay(float mouseX, float mouseY, float viewportWidth, float viewportHeight,
    const glm::mat4& view, const glm::mat4& projection) const
{
    if (viewportWidth <= 0.0f ||
        viewportHeight <= 0.0f)
    {
        return glm::vec3(0.0f);
    }

    const float normalisedX =
        (2.0f * mouseX) /
        viewportWidth -
        1.0f;

    const float normalisedY =
        1.0f -
        (2.0f * mouseY) /
        viewportHeight;

    const glm::vec4 rayClip(
        normalisedX,
        normalisedY,
        -1.0f,
        1.0f
    );

    glm::vec4 rayEye =
        glm::inverse(projection) *
        rayClip;

    rayEye =
        glm::vec4(
            rayEye.x,
            rayEye.y,
            -1.0f,
            0.0f
        );

    const glm::vec3 rayWorld =
        glm::normalize(
            glm::vec3(
                glm::inverse(view) *
                rayEye
            )
        );

    return rayWorld;
}

void SceneViewportPanel::Shutdown() {
    // Cleanup code here (if needed)
   /* DestroyEditorTexture(m_vertexIcon);
    DestroyEditorTexture(m_edgeIcon);
    DestroyEditorTexture(m_faceIcon);*/
   // DestroyEditorTexture(m_scaleIcon);
}
