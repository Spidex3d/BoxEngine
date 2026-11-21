#include <tools\FaceEditController.h>
#include <BoxEngine.h> // Add this include to resolve incomplete type error
#include <entity/Entity.h>
#include <imgui/imgui.h>
#include <miniBoxLog.h>
#include <camera/Camera.h>
#include <limits>
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <mesh/modifiers/FaceExtrude.h>
#include <mesh/modifiers/FaceInset.h>
#include <mesh/modifiers/RoundInset.h>
#include <mesh/modifiers/FaceRemove.h>
#include <mesh/modifiers/ModifierData.h>


// ============================================
// MIRROR FACE MATCHING
// BoxEditor - Symmetry Editing
// ============================================

namespace
{
    std::size_t FindMirroredFace(
        const MeshEditing& mesh,
        std::size_t sourceFaceIndex,
        int mirrorAxis,
        float tolerance)
    {
        constexpr std::size_t InvalidIndex =
            static_cast<std::size_t>(-1);

        if (sourceFaceIndex >= mesh.GetFaceCount())
            return InvalidIndex;

        const EditFace& sourceFace =
            mesh.GetFace(sourceFaceIndex);

        if (sourceFace.vertices.size() < 3)
            return InvalidIndex;

        const float toleranceSquared =
            tolerance * tolerance;

        // Check the other faces.
        for (std::size_t candidateIndex = 0;
            candidateIndex < mesh.GetFaceCount();
            ++candidateIndex)
        {
            if (candidateIndex == sourceFaceIndex)
                continue;

            const EditFace& candidate =
                mesh.GetFace(candidateIndex);

            // Mirrored faces must have the same
            // number of vertices.
            if (candidate.vertices.size() !=
                sourceFace.vertices.size())
            {
                continue;
            }

            std::vector<bool> matched(
                candidate.vertices.size(), false
            );

            bool allMatched = true;

            // ====================================
            // MATCH EACH MIRRORED VERTEX
            // ====================================

            for (std::size_t sourceVertexIndex :
            sourceFace.vertices)
            {
                if (sourceVertexIndex >=
                    mesh.GetVertexCount())
                {
                    allMatched = false;
                    break;
                }

                glm::vec3 mirroredPosition =
                    mesh.GetVertex(
                        sourceVertexIndex
                    ).position;

                // Reflect across the chosen
                // local coordinate axis.
                mirroredPosition[mirrorAxis] =
                    -mirroredPosition[mirrorAxis];

                bool foundVertex = false;

                for (std::size_t j = 0;
                    j < candidate.vertices.size();
                    ++j)
                {
                    if (matched[j])
                        continue;

                    const std::size_t vertexIndex =
                        candidate.vertices[j];

                    if (vertexIndex >= mesh.GetVertexCount())
                        continue;

                    const glm::vec3 candidatePosition =
                        mesh.GetVertex(vertexIndex).position;

                    const glm::vec3 difference =
                        candidatePosition - mirroredPosition;

                    if (glm::dot(difference, difference)
                        <= toleranceSquared)
                    {
                        matched[j] = true;
                        foundVertex = true;
                        break;
                    }
                }

                if (!foundVertex)
                {
                    allMatched = false;
                    break;
                }
            }

            if (allMatched)
                return candidateIndex;
        }

        return InvalidIndex;
    }
}


void FaceEditController::HandleInput(BoxEngine& engine, bool viewportHovered,
    bool faceModeActive, const ImVec2& viewportPosition, const ImVec2& viewportSize)
{
    Entity* entity = engine.GetSelectedEntity();

    // -------------------------------------------------
    // Face mode is not active
    // -------------------------------------------------
    if (!faceModeActive)
    {
        if (m_isMoving && entity)
        {
            EditCancelMove(*entity);
        }

        return;
    }

    if (!viewportHovered)
    {
        return;
    }

    if (!entity)
    {
        return;
    }

    // =================================================
    // SELECT ALL FACES
    // =================================================

    if (!m_isMoving &&
        !m_isExtruding &&
        !m_isInsetting)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_A, false))
        {
            entity->SelectAllFaces();

            BOX_LOG_INFO("Selected all faces");
        }
    }


    // =================================================
    // ACTIVE EXTRUSION
    // =================================================
    if (m_isExtruding)
    {
        // Choose extrusion axis.
        if (ImGui::IsKeyPressed(
            ImGuiKey_X,
            false))
        {
            m_extrudeAxis =
                ExtrudeAxis::X;

            m_extrudeStartMouse =
                ImGui::GetMousePos();

            BOX_LOG_INFO(
                "Extrude axis X"
            );
        }
        else if (ImGui::IsKeyPressed(
            ImGuiKey_Y,
            false))
        {
            m_extrudeAxis =
                ExtrudeAxis::Y;

            m_extrudeStartMouse =
                ImGui::GetMousePos();

            BOX_LOG_INFO(
                "Extrude axis Y"
            );
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_Z, false))
        {
            m_extrudeAxis = ExtrudeAxis::Z;

            m_extrudeStartMouse =
                ImGui::GetMousePos();

            BOX_LOG_INFO(
                "Extrude axis Z"
            );
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_N, false))
        {
            m_extrudeAxis = ExtrudeAxis::Normal;

            m_extrudeStartMouse = ImGui::GetMousePos();

            BOX_LOG_INFO("Extrude along face normals");
        }
        



        // ---------------------------------------------
        // Update extrusion after an axis is selected.
        // ---------------------------------------------
        if (m_extrudeAxis !=
            ExtrudeAxis::None)
        {
            const ImVec2 currentMouse =
                ImGui::GetMousePos();

            const float deltaX =
                currentMouse.x -
                m_extrudeStartMouse.x;

            const float deltaY =
                currentMouse.y -
                m_extrudeStartMouse.y;

            /*
             * X and Z use horizontal mouse movement.
             * Y uses vertical mouse movement.
             */
            switch (m_extrudeAxis)
            {
            case ExtrudeAxis::X:
                m_extrudeAmount =
                    deltaX *
                    m_moveSensitivity;
                break;

            case ExtrudeAxis::Y:
                m_extrudeAmount =
                    -deltaY *
                    m_moveSensitivity;
                break;

            case ExtrudeAxis::Z:
                m_extrudeAmount =
                    deltaX *
                    m_moveSensitivity;
                break;

            case ExtrudeAxis::Normal:
                m_extrudeAmount =
                    deltaX * m_moveSensitivity;
                break;

            case ExtrudeAxis::None:
            default:
                break;
            }


            // -----------------------------------------
            // Restore the mesh to the exact state it
            // had before THIS extrusion.
            // -----------------------------------------
            MeshEditing& editableMesh =
                entity->GetEditableMesh();

            editableMesh =
                m_meshBeforeExtrude;


            // -----------------------------------------
            // Work out extrusion direction.
            // -----------------------------------------
            glm::vec3 direction(0.0f);

            switch (m_extrudeAxis)
            {
            case ExtrudeAxis::X:
                direction =
                    glm::vec3(
                        1.0f,
                        0.0f,
                        0.0f
                    );
                break;

            case ExtrudeAxis::Y:
                direction =
                    glm::vec3(
                        0.0f,
                        1.0f,
                        0.0f
                    );
                break;

            case ExtrudeAxis::Z:
                direction =
                    glm::vec3(
                        0.0f,
                        0.0f,
                        1.0f
                    );
                break;

            case ExtrudeAxis::None:
            default:
                break;
            }


            // -----------------------------------------
            // Apply THIS extrusion to the restored
            // pre-extrusion mesh.
            // -----------------------------------------
            FaceExtrude extrude;

            UpdateExtrudeMesh(*entity);

            
        }


        // ---------------------------------------------
        // Confirm extrusion
        // ---------------------------------------------

        if (ImGui::IsMouseClicked(
            ImGuiMouseButton_Left))
        {
            ConfirmExtrude(*entity);

            return;
        }

        // ---------------------------------------------
        // Cancel extrusion
        // ---------------------------------------------
        
        if (ImGui::IsKeyPressed(
            ImGuiKey_Escape,
            false))
        {
            CancelExtrude(
                *entity
            );

            return;
        }

        return;
    }
    // =================================================
    // ACTIVE ANGLE EXTRUDE
    // =================================================

    if (m_isAngleExtruding)
    {
        bool changed = false;


        // -------------------------------------------------
        // Mouse controls bend radius
        // -------------------------------------------------

        const ImVec2 currentMouse =
            ImGui::GetMousePos();

        const float deltaX =
            currentMouse.x -
            m_angleExtrudeStartMouse.x;


        float newDistance =
            0.5f +
            deltaX *
            m_moveSensitivity;


        // AngleExtrude itself also protects against
        // geometry collapse, but don't allow zero here.
        newDistance =
            std::max(
                newDistance,
                0.01f
            );


        if (std::abs(
            newDistance -
            m_angleExtrudeDistance) >
            0.0001f)
        {
            m_angleExtrudeDistance =
                newDistance;

            changed =
                true;
        }

        // -------------------------------------------------
        // Numpad controls segments
        // 1 - 9  = normal
        // 10 - 16 = press 1 then 0 - 6
        // -------------------------------------------------

        int digit = -1;

        if (ImGui::IsKeyPressed(ImGuiKey_Keypad0, false))
            digit = 0;

        if (ImGui::IsKeyPressed(ImGuiKey_Keypad1, false))
            digit = 1;

        if (ImGui::IsKeyPressed(ImGuiKey_Keypad2, false))
            digit = 2;

        if (ImGui::IsKeyPressed(ImGuiKey_Keypad3, false))
            digit = 3;

        if (ImGui::IsKeyPressed(ImGuiKey_Keypad4, false))
            digit = 4;

        if (ImGui::IsKeyPressed(ImGuiKey_Keypad5, false))
            digit = 5;

        if (ImGui::IsKeyPressed(ImGuiKey_Keypad6, false))
            digit = 6;

        if (ImGui::IsKeyPressed(ImGuiKey_Keypad7, false))
            digit = 7;

        if (ImGui::IsKeyPressed(ImGuiKey_Keypad8, false))
            digit = 8;

        if (ImGui::IsKeyPressed(ImGuiKey_Keypad9, false))
            digit = 9;


        if (digit >= 0)
        {
            // -------------------------------------------------
            // If previous digit was 1, allow 10 - 16
            // -------------------------------------------------

            if (m_angleExtrudeSegmentInput == 1 &&
                digit <= 6)
            {
                m_angleExtrudeSegments =
                    10 + digit;

                m_angleExtrudeSegmentInput = 0;
            }
            else
            {
                // Normal single digit 1 - 9
                if (digit >= 1)
                {
                    m_angleExtrudeSegments =
                        digit;

                    m_angleExtrudeSegmentInput =
                        digit;
                }
            }

            changed = true;
        }


        // -------------------------------------------------
        // T = Tangent bend plane
        // -------------------------------------------------

        if (ImGui::IsKeyPressed(
            ImGuiKey_T,
            false))
        {
            m_angleExtrudeAxis =
                AngleExtrudeRotationAxis::Tangent;

            changed =
                true;

            BOX_LOG_INFO(
                "Angle Extrude axis: Tangent"
            );
        }


        // -------------------------------------------------
        // B = Bitangent bend plane
        // -------------------------------------------------

        if (ImGui::IsKeyPressed(
            ImGuiKey_B,
            false))
        {
            m_angleExtrudeAxis =
                AngleExtrudeRotationAxis::Bitangent;

            changed =
                true;

            BOX_LOG_INFO(
                "Angle Extrude axis: Bitangent"
            );
        }


        // -------------------------------------------------
        // F = flip direction
        // -------------------------------------------------

        if (ImGui::IsKeyPressed(
            ImGuiKey_F,
            false))
        {
            if (m_angleExtrudeDirection ==
                AngleExtrudeDirection::Positive)
            {
                m_angleExtrudeDirection =
                    AngleExtrudeDirection::Negative;
            }
            else
            {
                m_angleExtrudeDirection =
                    AngleExtrudeDirection::Positive;
            }

            changed =
                true;

            BOX_LOG_INFO(
                "Angle Extrude direction flipped"
            );
        }


        // -------------------------------------------------
        // Rebuild preview
        // -------------------------------------------------

        if (changed)
        {
            UpdateAngleExtrudeMesh(
                *entity
            );
        }


        // -------------------------------------------------
        // Escape = cancel
        // -------------------------------------------------

        if (ImGui::IsKeyPressed(
            ImGuiKey_Escape,
            false))
        {
            CancelAngleExtrude(
                *entity
            );

            return;
        }


        // -------------------------------------------------
        // Left click = confirm
        // -------------------------------------------------

        if (ImGui::IsMouseClicked(
            ImGuiMouseButton_Left))
        {
            ConfirmAngleExtrude(
                *entity
            );

            return;
        }


        // VERY IMPORTANT:
        // Don't let normal face picking run while
        // Angle Extrude owns the input.
        return;
    }





    // =================================================
    // ACTIVE INSET
    // =================================================

    if (m_isInsetting)
    {
        const ImVec2 currentMouse =
            ImGui::GetMousePos();

        const float deltaX =
            currentMouse.x -
            m_insetStartMouse.x;

        m_insetAmount =
            deltaX *
            m_moveSensitivity;

        // Keep the inset in a sensible range.
        m_insetAmount =
            glm::clamp(
                m_insetAmount,
                0.0f,
                0.95f
            );

        UpdateInsetMesh(
            *entity
        );


        // Confirm
        if (ImGui::IsMouseClicked(
            ImGuiMouseButton_Left))
        {
            ConfirmInset(
                *entity
            );

            return;
        }


        // Cancel
        if (ImGui::IsKeyPressed(
            ImGuiKey_Escape,
            false))
        {
            CancelInset(
                *entity
            );

            return;
        }

        return;
    }

    // =================================================
    // ACTIVE ROUND INSET
    // =================================================

    if (m_isRoundingInset)
    {
        const ImVec2 currentMouse =
            ImGui::GetMousePos();

        const float deltaX =
            currentMouse.x -
            m_roundInsetStartMouse.x;

        m_roundInsetAmount =
            deltaX *
            m_moveSensitivity;

        m_roundInsetAmount =
            glm::clamp(
                m_roundInsetAmount,
                0.0f,
                0.95f
            );

        UpdateRoundInsetMesh(
            *entity
        );


        // Confirm
        if (ImGui::IsMouseClicked(
            ImGuiMouseButton_Left))
        {
            ConfirmRoundInset(
                *entity
            );

            return;
        }


        // Cancel
        if (ImGui::IsKeyPressed(
            ImGuiKey_Escape,
            false))
        {
            CancelRoundInset(
                *entity
            );

            return;
        }

        return;
    }

    // =================================================
    // DELETE SELECTED FACE
    // =================================================

    if (!m_isMoving &&
        !m_isExtruding &&
        !m_isInsetting &&
        !m_isRoundingInset &&
        m_selectedFace != InvalidFace)
    {
        if (ImGui::IsKeyPressed(
            ImGuiKey_Delete,
            false))
        {
            MeshEditing& editableMesh =
                entity->GetEditableMesh();

            FaceRemove removeFace;

            if (removeFace.Use(
                editableMesh,
                m_selectedFace))
            {
                MeshData renderMesh;

                if (editableMesh.BuildRenderMesh(
                    renderMesh))
                {
                    entity->CreateFromMeshData(
                        renderMesh
                    );
                }

                entity->ClearSelectedFace();

                m_selectedFace =
                    InvalidFace;

                BOX_LOG_INFO(
                    "Deleted selected face"
                );
            }

            return;
        }
    }



    // =================================================
    // NORMAL FACE MOVE
    // =================================================

    if (!m_isMoving &&
        m_selectedFace != InvalidFace)
    {
        if (ImGui::IsKeyPressed(
            ImGuiKey_X,
            false))
        {
            EditBeginMove(
                *entity,
                FaceMoveAxis::X
            );
        }
        else if (ImGui::IsKeyPressed(
            ImGuiKey_Y,
            false))
        {
            EditBeginMove(
                *entity,
                FaceMoveAxis::Y
            );
        }
        else if (ImGui::IsKeyPressed(
            ImGuiKey_Z,
            false))
        {
            EditBeginMove(
                *entity,
                FaceMoveAxis::Z
            );
        }
        else if (ImGui::IsKeyPressed(
            ImGuiKey_N,
            false))
        {
            EditBeginMove(
                *entity,
                FaceMoveAxis::Normal
            );
        }

    }


    // =================================================
    // UPDATE ACTIVE NORMAL FACE MOVE
    // =================================================

    if (m_isMoving)
    {
        EditUpdateMove(
            *entity
        );

        if (ImGui::IsMouseClicked(
            ImGuiMouseButton_Left))
        {
            EditConfirmMove();
        }
        else if (
            ImGui::IsKeyPressed(
                ImGuiKey_Escape,
                false))
        {
            EditCancelMove(
                *entity
            );
        }

        return;
    }


    // =================================================
    // NORMAL FACE PICKING
    // =================================================

    if (ImGui::IsMouseClicked(
        ImGuiMouseButton_Left))
    {
        PickFace(
            engine,
            viewportPosition,
            viewportSize
        );
    }
}


void FaceEditController::DrawFace(BoxEngine& engine, const ImVec2& viewportPosition,
    const ImVec2& viewportSize, bool faceModeActive)
{
    if (!faceModeActive ||
        viewportSize.x <= 0.0f ||
        viewportSize.y <= 0.0f)
    {
        return;
    }

    Entity* entity =
        engine.GetSelectedEntity();

    if (!entity)
    {
        return;
    }

    const std::vector<LogicalFace> faces =
        BuildLogicalFace(*entity);

    if (faces.empty())
    {
        return;
    }

    Camera& camera =
        engine.GetCamera();

    const float aspectRatio =
        viewportSize.x /
        viewportSize.y;

    const glm::mat4 modelViewProjection =
        camera.GetProjectionMatrix(
            aspectRatio
        ) *
        camera.GetViewMatrix() *
        entity->GetModelMatrix();

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    for (std::size_t index = 0;
        index < faces.size();
        ++index)
    {
        ImVec2 screenA;
        ImVec2 screenB;
        ImVec2 screenC;
        ImVec2 screenD;
        ImVec2 screenCentre;

        // ################## New
        std::vector<ImVec2>
            screenPoints;

        screenPoints.reserve(
            faces[index].positions.size()
        );


        bool allVisible =
            true;


        for (const glm::vec3& position :
            faces[index].positions)
        {
            ImVec2 screenPosition;

            if (!ProjectToScreen(
                position,
                modelViewProjection,
                viewportPosition,
                viewportSize,
                screenPosition))
            {
                allVisible =
                    false;

                break;
            }

            screenPoints.push_back(
                screenPosition
            );
        }


        if (!allVisible ||
            screenPoints.size() < 3)
        {
            continue;
        }

        const bool selected = entity->IsFaceSelected(faces[index].editableFaceIndex);
        // ############### New

        if (selected)
        {
            const ImU32 fillColor =
                IM_COL32(
                    30,
                    255,
                    70,
                    70
                );


            for (std::size_t i = 1;
                i + 1 < screenPoints.size();
                ++i)
            {
                drawList->AddTriangleFilled(
                    screenPoints[0],
                    screenPoints[i],
                    screenPoints[i + 1],
                    fillColor
                );
            }


            // Draw polygon outline.
            const ImU32 outline =
                IM_COL32(
                    30,
                    255,
                    70,
                    255
                );

            constexpr float thickness =
                3.0f;


            for (std::size_t i = 0;
                i < screenPoints.size();
                ++i)
            {
                const std::size_t next =
                    (i + 1) %
                    screenPoints.size();

                drawList->AddLine(
                    screenPoints[i],
                    screenPoints[next],
                    outline,
                    thickness
                );
            }
        }

    }
  
}

void FaceEditController::BeginExtrude(Entity& entity)
{
    if (m_selectedFace == InvalidFace)
    {
        return;
    }

    MeshEditing& mesh = entity.GetEditableMesh();

    if (m_selectedFace >= mesh.GetFaceCount())
    {
        return;
    }

    // Save the mesh BEFORE this extrusion.
    m_meshBeforeExtrude = mesh;

    m_extrudeFace = m_selectedFace;


    // ============================================
    // CAPTURE ALL SELECTED FACES
    // ============================================

    m_extrudeFaces.clear();

    for (std::size_t faceIndex = 0;
        faceIndex < mesh.GetFaceCount();
        ++faceIndex)
    {
        if (entity.IsFaceSelected(faceIndex))
        {
            m_extrudeFaces.push_back(faceIndex);
        }
    }

    // The active face might not be selected
    // after Shift-clicking.
    // In that case, use the first selected face.
    if (!m_extrudeFaces.empty() &&
        !entity.IsFaceSelected(m_extrudeFace))
    {
        m_extrudeFace = m_extrudeFaces.front();
    }

    // Keep existing single-face behaviour
    // if no selection is registered.
    if (m_extrudeFaces.empty())
    {
        m_extrudeFaces.push_back(m_extrudeFace);
    }

    BOX_LOG_INFO(
        "Extrusion captured "
        << m_extrudeFaces.size()
        << " selected faces"
    );
    // ######

    m_extrudeAxis = ExtrudeAxis::None;

    m_extrudeAmount = 0.0f;

    m_extrudeStartMouse = ImGui::GetMousePos();

    m_isExtruding = true;

    BOX_LOG_INFO("Started extrusion on face " << m_extrudeFace);
}


void FaceEditController::SetRegionExtrude(
    Entity& entity,
    bool enabled)
{
    m_regionExtrude = enabled;

    // Rebuild the current extrusion preview.
    if (m_isExtruding)
    {
        UpdateExtrudeMesh(entity);
    }
}


// ##########################################################################################################
// ############################################ Object Explorer #############################################
// ##########################################################################################################

void FaceEditController::SetExtrudeAmount(Entity& entity, float amount)
{
    if (!m_isExtruding)
    {
        return;
    }

    m_extrudeAmount =
        amount;

    UpdateExtrudeMesh(
        entity
    );
}

void FaceEditController::SetExtrudeAxis(Entity& entity, ExtrudeAxis axis)
{
    if (!m_isExtruding)
    {
        return;
    }

    m_extrudeAxis =
        axis;

    UpdateExtrudeMesh(
        entity
    );
}

void FaceEditController::ConfirmExtrude(Entity& entity)
{
    if (!m_isExtruding)
    {
        return;
    }

    ModifierAxis storedAxis =
        ModifierAxis::Y;

    switch (m_extrudeAxis)
    {
    case ExtrudeAxis::X:
        storedAxis =
            ModifierAxis::X;
        break;

    case ExtrudeAxis::Y:
        storedAxis =
            ModifierAxis::Y;
        break;

    case ExtrudeAxis::Z:
        storedAxis =
            ModifierAxis::Z;
        break;

    case ExtrudeAxis::None:
    default:
        break;
    }

    // The last-operation editor currently supports
// only one face and X/Y/Z extrusion.

    if (m_extrudeFaces.size() == 1 &&
        m_extrudeAxis != ExtrudeAxis::Normal &&
        m_extrudeAxis != ExtrudeAxis::None)
    {
        entity.SetLastExtrude(
            m_extrudeFace,
            storedAxis,
            m_extrudeAmount,
            m_meshBeforeExtrude
        );
    }


    m_isExtruding =
        false;

    m_extrudeAxis =
        ExtrudeAxis::None;

    BOX_LOG_INFO(
        "Extrude confirmed. Amount="
        << m_extrudeAmount
    );

    m_extrudeFaces.clear();

}

void FaceEditController::CancelExtrude(Entity& entity)
{
    if (!m_isExtruding)
    {
        return;
    }

    entity.GetEditableMesh() =
        m_meshBeforeExtrude;

    MeshData renderMesh;

    if (entity
        .GetEditableMesh()
        .BuildRenderMesh(renderMesh))
    {
        entity.CreateFromMeshData(
            renderMesh
        );
    }

    m_isExtruding = false;

    m_extrudeAxis =
        ExtrudeAxis::None;

    m_extrudeAmount =
        0.0f;

    BOX_LOG_INFO("Extrude cancelled");

    m_extrudeFaces.clear();
}


void FaceEditController::UpdateExtrudeMesh(
    Entity& entity)
{
    if (!m_isExtruding ||
        m_extrudeAxis == ExtrudeAxis::None)
    {
        return;
    }

    MeshEditing& mesh = entity.GetEditableMesh();

    // Always begin from the original mesh.
    mesh = m_meshBeforeExtrude;

    // ============================================
    // GET MIRROR SETTINGS
    // ============================================

    bool mirrorEnabled = false;
    int mirrorAxis = 0;
    float tolerance = 0.001f;

    for (const ModifierData& modifier :
        entity.GetModifiers())
    {
        if (modifier.type != ModifierType::Mirror)
            continue;

        mirrorEnabled = modifier.modEnabled;
        tolerance = modifier.mirror.tolerance;

        switch (modifier.mirror.axis)
        {
        case ModifierAxis::Y:
            mirrorAxis = 1;
            break;

        case ModifierAxis::Z:
            mirrorAxis = 2;
            break;

        default:
            mirrorAxis = 0;
            break;
        }

        break;
    }

    // ============================================
    // CALCULATE FACE CENTRE
    // ============================================

    auto getFaceCentre =
        [&](std::size_t faceIndex) -> glm::vec3
    {
        const EditFace& face =
            mesh.GetFace(faceIndex);

        glm::vec3 centre(0.0f);

        for (std::size_t index : face.vertices)
        {
            centre += mesh.GetVertex(index).position;
        }

        return centre /
            static_cast<float>(face.vertices.size());
    };

    const glm::vec3 activeCentre =
        getFaceCentre(m_extrudeFace);

    const float activeSide =
        activeCentre[mirrorAxis];

    // ============================================
    // COLLECT EXTRUSION DIRECTIONS
    // ============================================

    std::vector<std::size_t> faces;
    std::vector<glm::vec3> directions;

    for (std::size_t faceIndex : m_extrudeFaces)
    {
        if (faceIndex >= mesh.GetFaceCount())
            continue;

        const EditFace& face =
            mesh.GetFace(faceIndex);

        if (face.vertices.size() < 3)
            continue;

        glm::vec3 direction(0.0f);

        // ========================================
        // FACE NORMAL
        // ========================================

        if (m_extrudeAxis == ExtrudeAxis::Normal)
        {
            const glm::vec3 a =
                mesh.GetVertex(face.vertices[0]).position;

            const glm::vec3 b =
                mesh.GetVertex(face.vertices[1]).position;

            const glm::vec3 c =
                mesh.GetVertex(face.vertices[2]).position;

            direction = glm::cross(
                b - a,
                c - a
            );

            if (glm::length(direction) < 0.000001f)
                continue;

            direction = glm::normalize(direction);
        }
        else
        {
            // ====================================
            // WORLD-ALIGNED LOCAL AXES
            // ====================================

            switch (m_extrudeAxis)
            {
            case ExtrudeAxis::X:
                direction.x = 1.0f;
                break;

            case ExtrudeAxis::Y:
                direction.y = 1.0f;
                break;

            case ExtrudeAxis::Z:
                direction.z = 1.0f;
                break;

            default:
                continue;
            }

            // ====================================
            // MIRROR THE AXIS DISPLACEMENT
            // ====================================

            if (mirrorEnabled &&
                direction[mirrorAxis] != 0.0f)
            {
                const float faceSide =
                    getFaceCentre(faceIndex)[mirrorAxis];

                if (std::abs(faceSide) <= tolerance)
                {
                    // Don't push a centre-plane face
                    // off the mirror plane.
                    continue;
                }

                if (std::abs(activeSide) > tolerance &&
                    faceSide * activeSide < 0.0f)
                {
                    direction = -direction;
                }
            }
        }

        faces.push_back(faceIndex);
        directions.push_back(direction);
    }

    if (faces.empty())
    {
        BOX_LOG_WARNING(
            "Extrusion: no valid faces or directions"
        );
        return;
    }

    // ============================================
    // EXTRUDE ALL SELECTED FACES
    // ============================================


    FaceExtrude extrude;

    bool success = false;

    // ============================================
    // CHOOSE EXTRUSION METHOD
    // ============================================

    if (m_regionExtrude)
    {
        success = extrude.UseRegion(
            mesh,
            faces,
            directions,
            m_extrudeAmount
        );
    }
    else
    {
        success = extrude.UseMultiple(
            mesh,
            faces,
            directions,
            m_extrudeAmount
        );
    }

    if (!success)
    {
        mesh = m_meshBeforeExtrude;
        return;
    }
    // ########

    

    // ============================================
    // REBUILD DISPLAY MESH
    // ============================================

    MeshData renderMesh;

    if (mesh.BuildRenderMesh(renderMesh))
    {
        entity.CreateFromMeshData(renderMesh);
    }
}

// ------------------------------------------------------
//Angle Extrude
// ------------------------------------------------------
void FaceEditController::BeginAngleExtrude(
    Entity& entity)
{
    if (m_selectedFace ==
        InvalidFace)
    {
        return;
    }

	m_angleExtrudeSegmentInput = 0; // Reset the segment input when starting a new angle extrude operation

    MeshEditing& mesh =
        entity.GetEditableMesh();

    if (m_selectedFace >=
        mesh.GetFaceCount())
    {
        return;
    }

    // Save mesh BEFORE Angle Extrude.
    m_meshBeforeAngleExtrude =
        mesh;

    m_angleExtrudeFace =
        m_selectedFace;

    m_angleExtrudeDistance =
        0.5f;

    m_angleExtrudeAngle =
        90.0f;

    m_angleExtrudeSegments =
        4;

    m_angleExtrudeAxis =
        AngleExtrudeRotationAxis::Bitangent;

    m_angleExtrudeDirection =
        AngleExtrudeDirection::Negative;

    m_angleExtrudeStartMouse =
        ImGui::GetMousePos();

    m_isAngleExtruding =
        true;

    // Build initial preview.
    UpdateAngleExtrudeMesh(
        entity
    );

    BOX_LOG_INFO(
        "Started Angle Extrude on face "
        << m_angleExtrudeFace
    );
}

void FaceEditController::UpdateAngleExtrudeMesh(
    Entity& entity)
{
    if (!m_isAngleExtruding)
    {
        return;
    }

    MeshEditing& editableMesh =
        entity.GetEditableMesh();

    // Always return to the original mesh.
    //
    // Otherwise every mouse movement would
    // Angle Extrude the previous preview again.
    editableMesh =
        m_meshBeforeAngleExtrude;


    AngleExtrudeSettings settings;

    settings.distance =
        m_angleExtrudeDistance;

    settings.angleDegrees =
        m_angleExtrudeAngle;

    settings.segments =
        m_angleExtrudeSegments;

    settings.rotationAxis =
        m_angleExtrudeAxis;

    settings.direction =
        m_angleExtrudeDirection;


    AngleExtrude angleExtrude;

    if (!angleExtrude.Use(
        editableMesh,
        m_angleExtrudeFace,
        settings))
    {
        return;
    }


    MeshData renderMesh;

    if (!editableMesh.BuildRenderMesh(
        renderMesh))
    {
        return;
    }


    entity.CreateFromMeshData(
        renderMesh
    );
}
void FaceEditController::ConfirmAngleExtrude(
    Entity& entity)
{
    if (!m_isAngleExtruding)
    {
        return;
    }

    m_isAngleExtruding =
        false;

    BOX_LOG_INFO(
        "Angle Extrude confirmed. "
        << "Distance="
        << m_angleExtrudeDistance
        << " Angle="
        << m_angleExtrudeAngle
        << " Segments="
        << m_angleExtrudeSegments
    );
}

void FaceEditController::CancelAngleExtrude(
    Entity& entity)
{
    if (!m_isAngleExtruding)
    {
        return;
    }

    entity.GetEditableMesh() =
        m_meshBeforeAngleExtrude;


    MeshData renderMesh;

    if (entity
        .GetEditableMesh()
        .BuildRenderMesh(renderMesh))
    {
        entity.CreateFromMeshData(
            renderMesh
        );
    }


    m_isAngleExtruding =
        false;

    BOX_LOG_INFO(
        "Angle Extrude cancelled"
    );
}



// #################################### End Object Explorer ###########################################

// ####################################################################################################
// ######################################## Inset Face ################################################
// ####################################################################################################

void FaceEditController::BeginInset(
    Entity& entity)
{
    if (m_selectedFace ==
        InvalidFace)
    {
        return;
    }

    MeshEditing& mesh =
        entity.GetEditableMesh();

    if (m_selectedFace >=
        mesh.GetFaceCount())
    {
        return;
    }

    // Save mesh before normal inset.
    m_meshBeforeInset =
        mesh;

    m_insetFace =
        m_selectedFace;

    m_insetAmount =
        0.0f;

    m_insetStartMouse =
        ImGui::GetMousePos();

    m_isInsetting =
        true;

    BOX_LOG_INFO(
        "Started inset on face "
        << m_insetFace
    );
}


void FaceEditController::SetInsetAmount(Entity& entity, float amount)
{
    if (!m_isInsetting)
    {
        return;
    }

    m_insetAmount = amount;

    UpdateInsetMesh(entity);
}



void FaceEditController::ConfirmInset(Entity& entity)
{
    if (!m_isInsetting)
    {
        return;
    }

    entity.SetLastInset(m_insetFace, m_insetAmount, m_meshBeforeInset);

    m_isInsetting = false;

    BOX_LOG_INFO("Inset confirmed. Amount=" << m_insetAmount);

}

void FaceEditController::CancelInset(Entity& entity)
{
    if (!m_isInsetting)
    {
        return;
    }

    entity.GetEditableMesh() = m_meshBeforeInset;

    MeshData renderMesh;

    if (entity.GetEditableMesh().BuildRenderMesh(renderMesh))
    {
        entity.CreateFromMeshData(renderMesh);
    }

    m_isInsetting = false;

    m_insetAmount = 0.0f;

    BOX_LOG_INFO("Inset cancelled");
}

void FaceEditController::BeginRoundInset(Entity& entity)
{

    if (m_selectedFace == InvalidFace)
    {
        return;
    }

    MeshEditing& mesh =
        entity.GetEditableMesh();

    if (m_selectedFace >=
        mesh.GetFaceCount())
    {
        return;
    }

    // Stage 1 currently supports quads only.
    if (mesh.GetFace(
        m_selectedFace
    ).vertices.size() != 4)
    {
        BOX_LOG_WARNING(
            "Round Inset currently supports quad faces only"
        );

        return;
    }

    // Save mesh before this operation.
    m_meshBeforeRoundInset =
        mesh;

    m_roundInsetFace =
        m_selectedFace;

    m_roundInsetAmount =
        0.0f;

    m_roundSegs =
        1;

    m_roundRound =
        1.0f;

    m_roundInsetStartMouse =
        ImGui::GetMousePos();

    m_isRoundingInset =
        true;

    BOX_LOG_INFO(
        "Started round inset on face "
        << m_roundInsetFace
    );

}



void FaceEditController::UpdateInsetMesh(Entity& entity)
{
    if (!m_isInsetting)
    {
        return;
    }

    MeshEditing& editableMesh = entity.GetEditableMesh();

    // Always go back to the mesh before
    // THIS inset operation.
    editableMesh = m_meshBeforeInset;

    FaceInset inset;

    if (!inset.Use(editableMesh, m_insetFace, m_insetAmount))
    {
        BOX_LOG_ERROR("Inset: failed to apply inset");

        return;
    }

    MeshData renderMesh;

    if (!editableMesh.BuildRenderMesh(renderMesh))
    {
        BOX_LOG_ERROR("Inset: failed to build render mesh");

        return;
    }

    entity.CreateFromMeshData(renderMesh);

}
// ####################################################################################################
// ######################################## End Inset Face #############################################
// ####################################################################################################


void FaceEditController::SetRoundInsetAmount(Entity& entity, float amount)
{
    if (!m_isRoundingInset)
    {
        return;
    }

    m_roundInsetAmount =
        glm::clamp(
            amount,
            0.0f,
            0.95f
        );

    UpdateRoundInsetMesh(
        entity
    );
}


void FaceEditController::SetRoundSegs(
    Entity& entity,
    int segments)
{
    if (!m_isRoundingInset)
    {
        return;
    }

    m_roundSegs =
        std::max(
            segments,
            1
        );

    UpdateRoundInsetMesh(
        entity
    );
}

void FaceEditController::SetRoundRound(
    Entity& entity,
    float roundness)
{
    if (!m_isRoundingInset)
    {
        return;
    }

    m_roundRound =
        glm::clamp(
            roundness,
            0.0f,
            1.0f
        );

    UpdateRoundInsetMesh(
        entity
    );

}


void FaceEditController::ConfirmRoundInset(Entity& entity)
{
    if (!m_isRoundingInset)
    {
        return;
    }

    entity.SetLastRoundInset(
        m_roundInsetFace,
        m_roundInsetAmount,
        m_roundSegs,
        m_roundRound,
        m_meshBeforeRoundInset
    );

    m_isRoundingInset =
        false;

    BOX_LOG_INFO(
        "Round Inset confirmed. Amount="
        << m_roundInsetAmount
        << " Segments="
        << m_roundSegs
        << " Roundness="
        << m_roundRound
    );
}

void FaceEditController::CancelRoundInset(Entity& entity)
{
    if (!m_isRoundingInset)
    {
        return;
    }

    entity.GetEditableMesh() =
        m_meshBeforeRoundInset;

    MeshData renderMesh;

    if (entity
        .GetEditableMesh()
        .BuildRenderMesh(renderMesh))
    {
        entity.CreateFromMeshData(
            renderMesh
        );
    }

    m_isRoundingInset =
        false;

    m_roundInsetAmount =
        0.0f;

    m_roundSegs =
        1;

    m_roundRound =
        1.0f;

    BOX_LOG_INFO(
        "Round Inset cancelled"
    );
}

void FaceEditController::UpdateRoundInsetMesh(Entity& entity)
{
    if (!m_isRoundingInset)
    {
        return;
    }

    MeshEditing& editableMesh =
        entity.GetEditableMesh();

    // Always return to the mesh from before
    // this round inset operation.
    editableMesh =
        m_meshBeforeRoundInset;

    RoundInset roundInset;

    if (!roundInset.Use(
        editableMesh,
        m_roundInsetFace,
        m_roundInsetAmount,
        m_roundSegs,
        m_roundRound))
    {
        BOX_LOG_ERROR(
            "Round Inset: failed to apply"
        );

        return;
    }

    MeshData renderMesh;

    if (!editableMesh.BuildRenderMesh(
        renderMesh))
    {
        BOX_LOG_ERROR(
            "Round Inset: failed to build render mesh"
        );

        return;
    }

    if (!entity.CreateFromMeshData(
        renderMesh))
    {
        BOX_LOG_ERROR(
            "Round Inset: failed to update entity mesh"
        );
    }
}


// #####################################################################################################
// ######################################## End Inset Face #############################################
// #####################################################################################################

void FaceEditController::ClearSelection(BoxEngine& engine)
{
    m_selectedFace =
        InvalidFace;

    Entity* entity =
        engine.GetSelectedEntity();

    if (entity)
    {
        entity->ClearSelectedFace();
    }

}
struct TriangleData
{
    glm::vec3 a{ 0.0f };
    glm::vec3 b{ 0.0f };
    glm::vec3 c{ 0.0f };

    glm::vec3 normal{ 0.0f };
};

bool FaceEditController::PointInTriangle(const ImVec2& point, const ImVec2& a, const ImVec2& b, const ImVec2& c)
{
    const auto Sign =
        [](const ImVec2& p1,
            const ImVec2& p2,
            const ImVec2& p3)
    {
        return
            (p1.x - p3.x) *
            (p2.y - p3.y) -
            (p2.x - p3.x) *
            (p1.y - p3.y);
    };

    const float d1 =
        Sign(point, a, b);

    const float d2 =
        Sign(point, b, c);

    const float d3 =
        Sign(point, c, a);

    const bool hasNegative =
        d1 < 0.0f ||
        d2 < 0.0f ||
        d3 < 0.0f;

    const bool hasPositive =
        d1 > 0.0f ||
        d2 > 0.0f ||
        d3 > 0.0f;

    return !(hasNegative &&
        hasPositive);
}
std::vector<FaceEditController::LogicalFace>FaceEditController::BuildLogicalFace(
    const Entity& entity) const
{
    std::vector<LogicalFace> faces;

    const MeshEditing& editableMesh =
        entity.GetEditableMesh();

    const auto& editFaces =
        editableMesh.GetFaces();

    if (editFaces.empty())
    {
        return faces;
    }


    for (std::size_t faceIndex = 0;
        faceIndex < editFaces.size();
        ++faceIndex)
    {
        const EditFace& editFace =
            editFaces[faceIndex];


        // A valid polygon needs at least
        // three vertices.
        if (editFace.vertices.size() < 3)
        {
            continue;
        }


        LogicalFace face;

        face.editableFaceIndex =
            faceIndex;

        face.vertexIndices =
            editFace.vertices;

        face.positions.reserve(
            editFace.vertices.size()
        );


        bool valid =
            true;


        // -----------------------------------------
        // Copy all logical face positions.
        // -----------------------------------------

        for (const std::size_t vertexIndex :
        editFace.vertices)
        {
            if (vertexIndex >=
                editableMesh.GetVertexCount())
            {
                valid =
                    false;

                break;
            }

            face.positions.push_back(
                editableMesh
                .GetVertex(vertexIndex)
                .position
            );
        }


        if (!valid ||
            face.positions.size() < 3)
        {
            continue;
        }


        // -----------------------------------------
        // Calculate centre.
        // -----------------------------------------

        glm::vec3 centre(0.0f);

        for (const glm::vec3& position :
            face.positions)
        {
            centre +=
                position;
        }

        centre /=
            static_cast<float>(
                face.positions.size()
                );

        face.centre =
            centre;


        // -----------------------------------------
        // Calculate normal from first
        // non-degenerate triangle.
        // -----------------------------------------

        const glm::vec3& positionA =
            face.positions[0];


        bool normalFound =
            false;


        for (std::size_t i = 1;
            i + 1 < face.positions.size();
            ++i)
        {
            const glm::vec3 edgeAB =
                face.positions[i] -
                positionA;

            const glm::vec3 edgeAC =
                face.positions[i + 1] -
                positionA;

            const glm::vec3 crossProduct =
                glm::cross(
                    edgeAB,
                    edgeAC
                );

            const float length =
                glm::length(
                    crossProduct
                );


            if (length <= 0.000001f)
            {
                continue;
            }


            face.normal =
                crossProduct /
                length;

            normalFound =
                true;

            break;
        }


        if (!normalFound)
        {
            continue;
        }


        faces.push_back(
            std::move(face)
        );
    }


    return faces;
}

bool FaceEditController::ProjectToScreen(const glm::vec3& localPosition, const glm::mat4& modelViewProjection,
    const ImVec2& viewportPosition, const ImVec2& viewportSize, ImVec2& outScreenPosition) const
{
    const glm::vec4 clipPosition =
        modelViewProjection *
        glm::vec4(
            localPosition,
            1.0f
        );

    if (clipPosition.w <= 0.0001f)
    {
        return false;
    }

    const glm::vec3 ndc =
        glm::vec3(clipPosition) /
        clipPosition.w;

    if (ndc.x < -1.0f ||
        ndc.x > 1.0f ||
        ndc.y < -1.0f ||
        ndc.y > 1.0f ||
        ndc.z < -1.0f ||
        ndc.z > 1.0f)
    {
        return false;
    }

    outScreenPosition.x =
        viewportPosition.x +
        ((ndc.x + 1.0f) * 0.5f) *
        viewportSize.x;

    outScreenPosition.y =
        viewportPosition.y +
        ((1.0f - ndc.y) * 0.5f) *
        viewportSize.y;

    return true;
}
// pick a face in the viewport based on the mouse position and return true if a face was selected, false otherwise
bool FaceEditController::PickFace(BoxEngine& engine, const ImVec2& viewportPosition, const ImVec2& viewportSize)
{
    Entity* entity = engine.GetSelectedEntity();

    if (!entity)
    {
        return false;
    }

    const bool shiftHeld = ImGui::GetIO().KeyShift; // Check if the Shift key is held down for multi-selection

    const std::vector<LogicalFace> faces =
        BuildLogicalFace(*entity);

    if (faces.empty())
    {
        return false;
    }

    Camera& camera =
        engine.GetCamera();

    const float aspectRatio =
        viewportSize.x /
        viewportSize.y;

    const glm::mat4 modelViewProjection =
        camera.GetProjectionMatrix(
            aspectRatio
        ) *
        camera.GetViewMatrix() *
        entity->GetModelMatrix();

    const ImVec2 mousePosition = ImGui::GetMousePos();

    std::size_t closestFace = InvalidFace;

    float closestDepth = std::numeric_limits<float>::max();
    // ################ New 


    for (std::size_t index = 0;
        index < faces.size();
        ++index)
    {
        ImVec2 screenA;
        ImVec2 screenB;
        ImVec2 screenC;
        ImVec2 screenD;
		// ################ New
        std::vector<ImVec2>
            screenPoints;

        screenPoints.reserve(
            faces[index].positions.size()
        );


        bool allVisible =
            true;


        for (const glm::vec3& position :
            faces[index].positions)
        {
            ImVec2 screenPosition;

            if (!ProjectToScreen(
                position,
                modelViewProjection,
                viewportPosition,
                viewportSize,
                screenPosition))
            {
                allVisible =
                    false;

                break;
            }

            screenPoints.push_back(
                screenPosition
            );
        }


        if (!allVisible ||
            screenPoints.size() < 3)
        {
            continue;
        }


		// ################ New

        bool inside = false;


        // Triangle fan:
        //
        // 0,1,2
        // 0,2,3
        // 0,3,4
        // ...

        for (std::size_t i = 1;
            i + 1 < screenPoints.size();
            ++i)
        {
            if (PointInTriangle(
                mousePosition,
                screenPoints[0],
                screenPoints[i],
                screenPoints[i + 1]))
            {
                inside =
                    true;

                break;
            }
        }


        if (!inside)
        {
            continue;
        }

        const glm::vec4 clipCentre =
            modelViewProjection *
            glm::vec4(
                faces[index].centre,
                1.0f
            );

        if (clipCentre.w <= 0.0001f)
        {
            continue;
        }

        const float depth =
            clipCentre.z /
            clipCentre.w;

        if (depth < closestDepth)
        {
            closestDepth = depth;
            closestFace = index;
        }
    }

    if (closestFace == InvalidFace)
    {
        // Only clear the current selection when
        // Shift is NOT being held.
        //
        // This prevents a missed Shift-click
        // destroying our multi-selection.
        if (!ImGui::GetIO().KeyShift)
        {
            ClearSelection(engine);
        }

        return false;
    }


    // -------------------------------------------------
    // Convert the picked logical face into the real
    // editable MeshEditing face index.
    // -------------------------------------------------

    const std::size_t pickedFace =
        faces[closestFace].editableFaceIndex;


    // -------------------------------------------------
    // SHIFT + LEFT CLICK
    //
    // Toggle this face in the multi-selection.
    // -------------------------------------------------

    if (ImGui::GetIO().KeyShift)
    {
        entity->ToggleSelectedFace(pickedFace);


        // m_selectedFace remains the "active" face.
        //
        // This is useful because your existing
        // extrude / inset / move tools still operate
        // on one active face.
        if (entity->IsFaceSelected(
            pickedFace))
        {
            m_selectedFace =
                pickedFace;
        }
        else
        {
            // We removed the active face.
            //
            // For now simply mark the active face
            // as invalid.
            if (m_selectedFace ==
                pickedFace)
            {
                m_selectedFace =
                    InvalidFace;
            }
        }
    }
    else
    {
        // -------------------------------------------------
        // NORMAL CLICK
        //
        // Clear previous selection and select one face.
        // -------------------------------------------------

        entity->SelectFace(pickedFace);

        m_selectedFace = pickedFace;
    }



    // ============================================
    // MIRROR FACE SELECTION - FIRST TEST
    // ============================================


// ============================================
// READ MIRROR MODIFIER SETTINGS
// ============================================

    bool mirrorEnabled = false;
    int mirrorAxis = 0;
    float mirrorTolerance = 0.001f;

    // Check this object's modifiers.
    for (const ModifierData& modifier :
        entity->GetModifiers())
    {
        if (modifier.type != ModifierType::Mirror)
            continue;

        mirrorEnabled = modifier.modEnabled;

        mirrorTolerance =
            modifier.mirror.tolerance;

        switch (modifier.mirror.axis)
        {
        case ModifierAxis::Y:
            mirrorAxis = 1;
            break;

        case ModifierAxis::Z:
            mirrorAxis = 2;
            break;

        case ModifierAxis::X:
        default:
            mirrorAxis = 0;
            break;
        }

        break;
    }


    
    if (mirrorEnabled)
    {
        const MeshEditing& editableMesh =
            entity->GetEditableMesh();

        const std::size_t oppositeFace =
            FindMirroredFace(
                editableMesh,
                pickedFace,
                mirrorAxis,
                mirrorTolerance
            );

        constexpr std::size_t InvalidIndex =
            static_cast<std::size_t>(-1);

        if (oppositeFace != InvalidIndex)
        {
            // Make the mirrored face's selection
            // match the face the user clicked.
            const bool shouldBeSelected =
                entity->IsFaceSelected(pickedFace);

            const bool oppositeIsSelected =
                entity->IsFaceSelected(oppositeFace);

            if (shouldBeSelected != oppositeIsSelected)
            {
                entity->ToggleSelectedFace(
                    oppositeFace
                );
            }

            BOX_LOG_INFO(
                "Mirror selected face "
                << pickedFace
                << " -> "
                << oppositeFace
            );
        }
    }



    BOX_LOG_INFO(
        "Selected face index: "
        << pickedFace
    );


    return true;

}


void FaceEditController::EditBeginMove(Entity& entity, FaceMoveAxis axis)
{
    MeshEditing& mesh = entity.GetEditableMesh();

    if (mesh.GetFaceCount() == 0)
        return;

    // ============================================
    // GET MIRROR SETTINGS
    // ============================================

    bool mirrorEnabled = false;
    int mirrorAxis = 0;
    float tolerance = 0.001f;

    for (const ModifierData& modifier :
        entity.GetModifiers())
    {
        if (modifier.type != ModifierType::Mirror)
            continue;

        mirrorEnabled = modifier.modEnabled;

        tolerance = modifier.mirror.tolerance;

        switch (modifier.mirror.axis)
        {
        case ModifierAxis::Y:
            mirrorAxis = 1;
            break;

        case ModifierAxis::Z:
            mirrorAxis = 2;
            break;

        default:
            mirrorAxis = 0;
            break;
        }

        break;
    }

    // ============================================
    // COLLECT SELECTED FACES
    // ============================================

    std::vector<std::size_t> selectedFaces;

    for (std::size_t i = 0;
        i < mesh.GetFaceCount();
        ++i)
    {
        if (entity.IsFaceSelected(i))
        {
            selectedFaces.push_back(i);
        }
    }

    if (selectedFaces.empty())
        return;

    // The active face controls the direction
    // of mirrored X/Y/Z movement.
    std::size_t activeFace = m_selectedFace;

    if (activeFace >= mesh.GetFaceCount() ||
        !entity.IsFaceSelected(activeFace))
    {
        activeFace = selectedFaces.front();
    }

    // ============================================
    // CALCULATE FACE CENTRE
    // ============================================

    auto getFaceCentre =
        [&](std::size_t faceIndex) -> glm::vec3
    {
        const EditFace& face =
            mesh.GetFace(faceIndex);

        glm::vec3 centre(0.0f);
        std::size_t count = 0;

        for (std::size_t index : face.vertices)
        {
            if (index >= mesh.GetVertexCount())
                continue;

            centre += mesh.GetVertex(index).position;
            ++count;
        }

        return count > 0
            ? centre / static_cast<float>(count)
            : glm::vec3(0.0f);
    };

    const glm::vec3 activeCentre =
        getFaceCentre(activeFace);

    const float activeSide =
        activeCentre[mirrorAxis];

    // ============================================
    // GET AXIS MOVEMENT DIRECTION
    // ============================================

    glm::vec3 axisDirection(0.0f);

    switch (axis)
    {
    case FaceMoveAxis::X:
        axisDirection.x = 1.0f;
        break;

    case FaceMoveAxis::Y:
        axisDirection.y = 1.0f;
        break;

    case FaceMoveAxis::Z:
        axisDirection.z = 1.0f;
        break;

    case FaceMoveAxis::Normal:
        break;

    default:
        return;
    }

    // ============================================
    // COLLECT VERTICES AND DIRECTIONS
    // ============================================

    m_moveVertices.clear();

    // Used to average movement directions
    // where faces share the same vertex.
    std::vector<int> directionCount;

    for (std::size_t faceIndex : selectedFaces)
    {
        const EditFace& face =
            mesh.GetFace(faceIndex);

        if (face.vertices.size() < 3)
            continue;

        const glm::vec3 faceCentre =
            getFaceCentre(faceIndex);

        glm::vec3 faceDirection = axisDirection;

        // ========================================
        // NORMAL DIRECTION
        // ========================================

        if (axis == FaceMoveAxis::Normal)
        {
            const std::size_t ia = face.vertices[0];
            const std::size_t ib = face.vertices[1];
            const std::size_t ic = face.vertices[2];

            if (ia >= mesh.GetVertexCount() ||
                ib >= mesh.GetVertexCount() ||
                ic >= mesh.GetVertexCount())
            {
                continue;
            }

            const glm::vec3 a =
                mesh.GetVertex(ia).position;

            const glm::vec3 b =
                mesh.GetVertex(ib).position;

            const glm::vec3 c =
                mesh.GetVertex(ic).position;

            const glm::vec3 normal =
                glm::cross(b - a, c - a);

            const float length = glm::length(normal);

            if (length < 0.000001f)
                continue;

            faceDirection = normal / length;
        }

        // ========================================
        // SYMMETRICAL AXIS MOVEMENT
        // ========================================

        else if (mirrorEnabled &&
            axisDirection[mirrorAxis] != 0.0f)
        {
            const float faceSide =
                faceCentre[mirrorAxis];

            // A face centred on the mirror plane
            // must not move away from that plane
            // along the mirrored axis.
            if (std::abs(faceSide) <= tolerance)
            {
                faceDirection = glm::vec3(0.0f);
            }
            else if (std::abs(activeSide) > tolerance &&
                faceSide * activeSide < 0.0f)
            {
                // Opposite side of the mirror plane.
                faceDirection = -axisDirection;
            }
        }

        // ========================================
        // STORE EACH VERTEX ONLY ONCE
        // ========================================

        for (std::size_t vertexIndex : face.vertices)
        {
            if (vertexIndex >= mesh.GetVertexCount())
                continue;

            auto found = std::find_if(
                m_moveVertices.begin(),
                m_moveVertices.end(),
                [vertexIndex](const MoveVertexData& item)
            {
                return item.index == vertexIndex;
            }
            );

            if (found == m_moveVertices.end())
            {
                MoveVertexData data;

                data.index = vertexIndex;

                data.startPosition =
                    mesh.GetVertex(vertexIndex).position;

                data.direction = faceDirection;

                m_moveVertices.push_back(data);

                directionCount.push_back(1);
            }
            else
            {
                const std::size_t position =
                    static_cast<std::size_t>(
                        std::distance(
                            m_moveVertices.begin(),
                            found
                        )
                        );

                found->direction += faceDirection;

                ++directionCount[position];
            }
        }
    }

    // ============================================
    // AVERAGE SHARED VERTEX DIRECTIONS
    // ============================================

    for (std::size_t i = 0;
        i < m_moveVertices.size();
        ++i)
    {
        m_moveVertices[i].direction /=
            static_cast<float>(directionCount[i]);

        // Keep centreline vertices on the
        // symmetry plane for Normal movement.
        if (mirrorEnabled &&
            axis == FaceMoveAxis::Normal &&
            std::abs(
                m_moveVertices[i]
                .startPosition[mirrorAxis]
            ) <= tolerance)
        {
            m_moveVertices[i].direction[mirrorAxis] =
                0.0f;
        }
    }

    if (m_moveVertices.empty())
        return;

    // ============================================
    // BEGIN MOVE
    // ============================================

    m_moveAxis = axis;
    m_startMouse = ImGui::GetMousePos();
    m_isMoving = true;

    BOX_LOG_INFO(
        "Multi-face move started: "
        << selectedFaces.size() << " faces, "
        << m_moveVertices.size() << " vertices"
    );
}

void FaceEditController::EditUpdateMove(Entity& entity)
{
    if (!m_isMoving)
        return;

    const ImVec2 mouse = ImGui::GetMousePos();

    const float deltaX =
        mouse.x - m_startMouse.x;

    const float deltaY =
        mouse.y - m_startMouse.y;

    // ============================================
    // CALCULATE MOVE DISTANCE
    // ============================================

    float amount = 0.0f;

    switch (m_moveAxis)
    {
    case FaceMoveAxis::Y:
        amount = -deltaY * m_moveSensitivity;
        break;

    case FaceMoveAxis::X:
    case FaceMoveAxis::Z:
    case FaceMoveAxis::Normal:
        amount = deltaX * m_moveSensitivity;
        break;

    default:
        return;
    }

    // ============================================
    // MOVE ALL SELECTED VERTICES
    // ============================================

    MeshEditing& mesh =
        entity.GetEditableMesh();

    for (const MoveVertexData& vertex :
        m_moveVertices)
    {
        if (vertex.index >= mesh.GetVertexCount())
            continue;

        mesh.GetVertex(vertex.index).position =
            vertex.startPosition +
            vertex.direction * amount;
    }

    // ============================================
    // REBUILD RENDER MESH
    // ============================================

    MeshData renderMesh;

    if (!mesh.BuildRenderMesh(renderMesh))
    {
        BOX_LOG_ERROR(
            "Multi-face move: failed to build mesh"
        );
        return;
    }

    if (!entity.CreateFromMeshData(renderMesh))
    {
        BOX_LOG_ERROR(
            "Multi-face move: failed to update GPU mesh"
        );
    }
}


void FaceEditController::EditConfirmMove()
{
    m_isMoving = false;
    m_moveAxis = FaceMoveAxis::None;

    m_moveVertices.clear();

    // Clear the older move data as well.
    m_startVertexPositions.clear();
    m_editableFaceVertices.clear();
    m_editableStartPositions.clear();

    BOX_LOG_INFO("Multi-face move confirmed");
}


void FaceEditController::EditCancelMove(
    Entity& entity)
{
    MeshEditing& mesh =
        entity.GetEditableMesh();

    // Restore every original vertex position.
    for (const MoveVertexData& vertex :
        m_moveVertices)
    {
        if (vertex.index >= mesh.GetVertexCount())
            continue;

        mesh.GetVertex(vertex.index).position =
            vertex.startPosition;
    }

    MeshData renderMesh;

    if (mesh.BuildRenderMesh(renderMesh))
    {
        entity.CreateFromMeshData(renderMesh);
    }

    m_isMoving = false;
    m_moveAxis = FaceMoveAxis::None;

    m_moveVertices.clear();

    m_startVertexPositions.clear();
    m_editableFaceVertices.clear();
    m_editableStartPositions.clear();

    BOX_LOG_INFO("Multi-face move cancelled");
}
