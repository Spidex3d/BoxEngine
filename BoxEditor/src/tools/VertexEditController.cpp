#include "tools/VertexEditController.h"
#include <BoxEngine.h> // Add this include to resolve incomplete type error
#include <entity/Entity.h>
#include <imgui/imgui.h>
#include <miniBoxLog.h>
#include <camera/Camera.h>
#include <limits>
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <mesh/modifiers/FaceFill.h>

void VertexEditController::HandleInput(BoxEngine& engine, bool viewportHovered, bool vertexModeActive, const ImVec2& viewportPosition,
    const ImVec2& viewportSize)
{
   
    if (!vertexModeActive)
    {
        if (m_isMoving)
        {
            Entity* entity =
                engine.GetSelectedEntity();

            if (entity)
            {
                EditCancelMove(*entity);
            }
        }

        return;
    }

    if (!viewportHovered)
    {
        return;
    }
    // #############################################################################################################
	// ############################################### Handle editing vertex movement ##############################
    // #############################################################################################################
    Entity* entity = engine.GetSelectedEntity();

    if (!entity)
    {
        return;
    }
    // =================================================
    // FILL FACE FROM SELECTED VERTICES
    // =================================================

    if (!m_isMoving)
    {
        if (ImGui::IsKeyPressed(
            ImGuiKey_F,
            false))
        {
            const std::vector<std::size_t>&
                selectedVertices =
                entity->GetSelectedVertices();

            if (selectedVertices.size() < 3)
            {
                BOX_LOG_WARNING("Face Fill requires at least 3 selected vertices");

                return;
            }

            MeshEditing& editableMesh =
                entity->GetEditableMesh();

            FaceFill faceFill;

            if (!faceFill.Use(
                editableMesh,
                selectedVertices))
            {
                BOX_LOG_ERROR(
                    "Face Fill failed"
                );

                return;
            }

            MeshData renderMesh;

            if (!editableMesh.BuildRenderMesh(renderMesh))
            {
                BOX_LOG_ERROR("Face Fill: failed to rebuild render mesh");

                return;
            }

            if (!entity->CreateFromMeshData(
                renderMesh))
            {
                BOX_LOG_ERROR("Face Fill: failed to update entity mesh");

                return;
            }

            BOX_LOG_INFO(
                "Face Fill completed with "
                << selectedVertices.size()
                << " vertices"
            );

            return;
        }
    }
	// ----------------------------------------- End of Face Fill ----------------------------------------- 




    /*
     * Start moving the selected logical vertex
     * when X, Y or Z is pressed.
     */
    if (!m_isMoving && m_selectedVertex != InvalidVertex)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_X, false))
        {
            EditBeginMove(*entity, VertexMoveAxis::X);
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_Y, false))
        {
            EditBeginMove(*entity, VertexMoveAxis::Y);  
                
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_Z, false))
            
        {
            EditBeginMove(*entity, VertexMoveAxis::Z);
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_N, false))
        {
            EditBeginMove(*entity, VertexMoveAxis::Normal);
        }
    }

    /*
     * Update, confirm or cancel an active move.
     */
    if (m_isMoving)
    {
        EditUpdateMove(*entity);

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            EditConfirmMove();
        }
        else if (
            ImGui::IsMouseClicked(ImGuiMouseButton_Right) ||
            ImGui::IsKeyPressed(ImGuiKey_Escape, false))
        {
            EditCancelMove(*entity);
        }

        return;
    }

    /*
     * Pick another vertex only when a move is
     * not currently active.
     */
    if (ImGui::IsMouseClicked(
        ImGuiMouseButton_Left))
    {
        PickVertex(
            engine,
            viewportPosition,
            viewportSize
        );
    }

}
void VertexEditController::DrawVertices(BoxEngine& engine, const ImVec2& viewportPosition, const ImVec2& viewportSize, bool vertexModeActive)
{
    if (!vertexModeActive)
    {
        return;
    }

    if (viewportSize.x <= 0.0f || viewportSize.y <= 0.0f)
    {
        return;
    }

    Entity* entity = engine.GetSelectedEntity();

    if (!entity)
    {
        return;
    }

    const MeshEditing& editableMesh =
        entity->GetEditableMesh();

    if (editableMesh.GetVertexCount() == 0)
    {
        return;
    }

    Camera& camera = engine.GetCamera();

    const float aspectRatio =
        viewportSize.x /
        viewportSize.y;

    const glm::mat4 model = entity->GetModelMatrix();

    const glm::mat4 view = camera.GetViewMatrix();

    const glm::mat4 projection = camera.GetProjectionMatrix(aspectRatio);

    const glm::mat4 modelViewProjection = projection * view * model;

    ImDrawList* drawList = ImGui::GetWindowDrawList();


    std::vector<glm::vec3>drawnPositions;

    for (std::size_t index = 0;
        index < editableMesh.GetVertexCount();
        ++index)
    {
        const glm::vec3& position =
            editableMesh
            .GetVertex(index)
            .position;

        const glm::vec4 clipPosition =
            modelViewProjection *
            glm::vec4(
                position,
                1.0f
            );

        if (clipPosition.w <= 0.0001f)
        {
            continue;
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
            continue;
        }

        const float screenX =
            viewportPosition.x +
            ((ndc.x + 1.0f) * 0.5f) *
            viewportSize.x;

        const float screenY =
            viewportPosition.y +
            ((1.0f - ndc.y) * 0.5f) *
            viewportSize.y;

        const ImVec2 screenPosition(
            screenX,
            screenY
        );

        const bool selected = entity->IsVertexSelected(index);
       // const bool selected = index == m_selectedVertex;

        const ImU32 fillColor =
            selected
            ? IM_COL32(
                0,
                255,
                35,
                255
            )
            : IM_COL32(
                50,
                170,
                255,
                255
            );

        const float radius =
            selected ? 6.0f : 4.0f;

        drawList->AddCircleFilled(
            screenPosition,
            radius,
            fillColor
        );

    }
}



void VertexEditController::ClearSelection(BoxEngine& engine)
{
    m_selectedVertex = InvalidVertex;

    Entity* entity = engine.GetSelectedEntity();
    if (entity)
    {
        entity->ClearSelectedVertices();
    }
}


bool VertexEditController::PickVertex(BoxEngine& engine, const ImVec2& viewportPosition, const ImVec2& viewportSize)
{

    Entity* entity =
        engine.GetSelectedEntity();

    if (!entity)
    {
        return false;
    }

    const MeshEditing& editableMesh =
        entity->GetEditableMesh();

    if (editableMesh.GetVertexCount() == 0)
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

    const ImVec2 mousePosition =
        ImGui::GetMousePos();

    constexpr float pickRadius =
        10.0f;

    const float pickRadiusSquared =
        pickRadius * pickRadius;

    float closestDistanceSquared =
        std::numeric_limits<float>::max();

    std::size_t closestVertex =
        InvalidVertex;

    for (std::size_t index = 0;
        index < editableMesh.GetVertexCount();
        ++index)
    {
        const glm::vec3& position =
            editableMesh
            .GetVertex(index)
            .position;

        const glm::vec4 clipPosition =
            modelViewProjection *
            glm::vec4(
                position,
                1.0f
            );

        if (clipPosition.w <= 0.0001f)
        {
            continue;
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
            continue;
        }

        const float screenX =
            viewportPosition.x +
            ((ndc.x + 1.0f) * 0.5f) *
            viewportSize.x;

        const float screenY =
            viewportPosition.y +
            ((1.0f - ndc.y) * 0.5f) *
            viewportSize.y;

        const float dx =
            mousePosition.x -
            screenX;

        const float dy =
            mousePosition.y -
            screenY;

        const float distanceSquared =
            dx * dx +
            dy * dy;

        if (distanceSquared <=
            pickRadiusSquared &&
            distanceSquared <
            closestDistanceSquared)
        {
            closestDistanceSquared =
                distanceSquared;

            closestVertex =
                index;
        }
    }

    if (closestVertex == InvalidVertex)
    {
        if (!ImGui::GetIO().KeyShift)
        {
            m_selectedVertex =
                InvalidVertex;

            entity->ClearSelectedVertices();
        }

        return false;
    }
    
    const bool shiftHeld =
        ImGui::GetIO().KeyShift;

    m_selectedVertex =
        closestVertex;

    if (shiftHeld)
    {
        // Add another vertex to the existing selection.
        if (!entity->IsVertexSelected(
            closestVertex))
        {
            entity->AddSelectedVertex(
                closestVertex
            );
        }
    }
    else
    {
        // Normal click selects only one vertex.
        entity->ClearSelectedVertices();

        entity->AddSelectedVertex(
            closestVertex
        );
    }


    // =================================================
    // MIRROR VERTEX SELECTION
    // =================================================

    bool mirrorEnabled = false;
    int mirrorAxis = 0;
    float mirrorTolerance = 0.001f;

    // -------------------------------------------------
    // Read the existing Mirror Modifier
    // -------------------------------------------------

    for (const ModifierData& modifier :
        entity->GetModifiers())
    {
        if (modifier.type != ModifierType::Mirror)
            continue;

        if (!modifier.modEnabled ||
            modifier.mirror.axis == ModifierAxis::None)
        {
            break;
        }

        mirrorEnabled = true;

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

    // -------------------------------------------------
    // Find and select matching opposite vertices
    // -------------------------------------------------

    if (mirrorEnabled)
    {
        // Copy before adding mirrored vertices.
        // This preserves Shift multi-selection.
        const std::vector<std::size_t> selectedVertices =
            entity->GetSelectedVertices();

        const float toleranceSquared =
            mirrorTolerance * mirrorTolerance;

        int mirroredCount = 0;

        for (std::size_t selectedIndex : selectedVertices)
        {
            if (selectedIndex >= editableMesh.GetVertexCount())
                continue;

            glm::vec3 mirroredPosition =
                editableMesh.GetVertex(selectedIndex).position;

            // Reflect the position across the chosen plane.
            mirroredPosition[mirrorAxis] =
                -mirroredPosition[mirrorAxis];

            std::size_t closestMatch =
                InvalidVertex;

            float bestDistanceSquared =
                toleranceSquared;

            // Search the editable mesh for a matching vertex.
            for (std::size_t candidateIndex = 0;
                candidateIndex < editableMesh.GetVertexCount();
                ++candidateIndex)
            {
                const glm::vec3 candidatePosition =
                    editableMesh.GetVertex(candidateIndex).position;

                const glm::vec3 difference =
                    candidatePosition - mirroredPosition;

                const float distanceSquared =
                    glm::dot(difference, difference);

                if (distanceSquared <= bestDistanceSquared)
                {
                    bestDistanceSquared = distanceSquared;
                    closestMatch = candidateIndex;
                }
            }

            // No matching vertex found.
            if (closestMatch == InvalidVertex)
                continue;

            // A centreline vertex can be its own mirror.
            // Don't add it again.
            if (!entity->IsVertexSelected(closestMatch))
            {
                entity->AddSelectedVertex(closestMatch);
                ++mirroredCount;
            }
        }

        BOX_LOG_INFO(
            "Mirror Vertex Selection: "
            << mirroredCount
            << " opposite vertices added"
        );
    }


    BOX_LOG_INFO(
        "Selected editable vertex index: "
        << m_selectedVertex
    );


    return true;
    
}


// ##########################################################################################
// ############################# new functions for moving vertices ##########################
// ##########################################################################################


void VertexEditController::EditBeginMove(
    Entity& entity,
    VertexMoveAxis axis)
{
    MeshEditing& mesh = entity.GetEditableMesh();

    const auto& selected =
        entity.GetSelectedVertices();

    if (selected.empty())
        return;

    m_moveVertices.clear();

    // ============================================
    // READ MIRROR SETTINGS
    // ============================================

    bool mirrorEnabled = false;
    int mirrorAxis = 0;
    float mirrorTolerance = 0.001f;

    for (const ModifierData& modifier :
        entity.GetModifiers())
    {
        if (modifier.type != ModifierType::Mirror)
            continue;

        if (!modifier.modEnabled ||
            modifier.mirror.axis == ModifierAxis::None)
        {
            break;
        }

        mirrorEnabled = true;
        mirrorTolerance = modifier.mirror.tolerance;

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
    // ACTIVE VERTEX DETERMINES MIRROR DIRECTION
    // ============================================

    float activeSide = 0.0f;

    if (m_selectedVertex < mesh.GetVertexCount())
    {
        activeSide =
            mesh.GetVertex(m_selectedVertex)
            .position[mirrorAxis];
    }

    // ============================================
    // COLLECT UNIQUE SELECTED VERTICES
    // ============================================

    for (std::size_t vertexIndex : selected)
    {
        if (vertexIndex >= mesh.GetVertexCount())
            continue;

        const bool alreadyAdded =
            std::find_if(
                m_moveVertices.begin(),
                m_moveVertices.end(),
                [vertexIndex](const MoveVertex& item)
        {
            return item.index == vertexIndex;
        }
            ) != m_moveVertices.end();

        if (alreadyAdded)
            continue;

        MoveVertex item;

        item.index = vertexIndex;

        item.startPosition =
            mesh.GetVertex(vertexIndex).position;

        glm::vec3 direction(0.0f);

        // ========================================
        // STANDARD AXES
        // ========================================

        switch (axis)
        {
        case VertexMoveAxis::X:
            direction.x = 1.0f;
            break;

        case VertexMoveAxis::Y:
            direction.y = 1.0f;
            break;

        case VertexMoveAxis::Z:
            direction.z = 1.0f;
            break;

        case VertexMoveAxis::Normal:
        {
            // ====================================
            // CALCULATE AVERAGED VERTEX NORMAL
            // ====================================

            glm::vec3 normalSum(0.0f);

            for (const EditFace& face : mesh.GetFaces())
            {
                if (face.vertices.size() < 3)
                    continue;

                const bool containsVertex =
                    std::find(
                        face.vertices.begin(),
                        face.vertices.end(),
                        vertexIndex
                    ) != face.vertices.end();

                if (!containsVertex)
                    continue;

                glm::vec3 faceNormal(0.0f);

                const std::size_t count =
                    face.vertices.size();

                // Newell polygon normal calculation.
                for (std::size_t j = 0; j < count; ++j)
                {
                    const std::size_t aIndex =
                        face.vertices[j];

                    const std::size_t bIndex =
                        face.vertices[(j + 1) % count];

                    if (aIndex >= mesh.GetVertexCount() ||
                        bIndex >= mesh.GetVertexCount())
                    {
                        continue;
                    }

                    const glm::vec3 a =
                        mesh.GetVertex(aIndex).position;

                    const glm::vec3 b =
                        mesh.GetVertex(bIndex).position;

                    faceNormal.x +=
                        (a.y - b.y) * (a.z + b.z);

                    faceNormal.y +=
                        (a.z - b.z) * (a.x + b.x);

                    faceNormal.z +=
                        (a.x - b.x) * (a.y + b.y);
                }

                const float length =
                    glm::length(faceNormal);

                if (length > 0.000001f)
                {
                    normalSum += faceNormal / length;
                }
            }

            if (glm::length(normalSum) > 0.000001f)
            {
                direction = glm::normalize(normalSum);
            }

            break;
        }

        default:
            break;
        }

        // ========================================
        // MIRROR MOVEMENT
        // ========================================

        if (mirrorEnabled)
        {
            const float vertexSide =
                item.startPosition[mirrorAxis];

            // A vertex on the mirror plane
            // must remain on that plane.
            if (std::abs(vertexSide) <= mirrorTolerance)
            {
                direction[mirrorAxis] = 0.0f;
            }
            else if (
                axis != VertexMoveAxis::Normal &&
                std::abs(activeSide) > mirrorTolerance &&
                vertexSide * activeSide < 0.0f)
            {
                // Mirror movement on the opposite side.
                direction[mirrorAxis] =
                    -direction[mirrorAxis];
            }
        }

        item.direction = direction;

        m_moveVertices.push_back(item);
    }

    if (m_moveVertices.empty())
        return;

    m_moveAxis = axis;
    m_startMouse = ImGui::GetMousePos();

    // Start a new operation at zero distance.
    m_moveDistance = 0.0f;
    m_usePreciseDistance = false;


    m_isMoving = true;

    BOX_LOG_INFO(
        "Vertex move started. Vertices="
        << m_moveVertices.size()
        << " Axis="
        << static_cast<int>(axis)
    );
}

void VertexEditController::EditUpdateMove(
    Entity& entity)
{
    if (!m_isMoving)
        return;

    const ImVec2 currentMouse =
        ImGui::GetMousePos();

    const float deltaX =
        currentMouse.x - m_startMouse.x;

    const float deltaY =
        currentMouse.y - m_startMouse.y;

    // ============================================
    // CALCULATE MOVEMENT DISTANCE
    // ============================================

    float amount = 0.0f;

    switch (m_moveAxis)
    {
    case VertexMoveAxis::X:
        amount = deltaX * m_moveSensitivity;
        break;

    case VertexMoveAxis::Y:
        amount = -deltaY * m_moveSensitivity;
        break;

    case VertexMoveAxis::Z:
        amount = deltaX * m_moveSensitivity;
        break;

    case VertexMoveAxis::Normal:
        amount = deltaX * m_moveSensitivity;
        break;

    default:
        return;
    }

    // ============================================
    // APPLY PRECISE DISTANCE IF ENTERED
    // ============================================

    if (m_usePreciseDistance)
    {
        amount = m_moveDistance;
    }
    else
    {
        // Keep the panel's numerical value
        // updated while moving the mouse.
        m_moveDistance = amount;
    }


    MeshEditing& mesh = entity.GetEditableMesh();

    // ============================================
    // MOVE ALL SELECTED VERTICES
    // ============================================

    for (const MoveVertex& item : m_moveVertices)
    {
        if (item.index >= mesh.GetVertexCount())
            continue;

        mesh.GetVertex(item.index).position =
            item.startPosition +
            item.direction * amount;
    }

    // ============================================
    // REBUILD THE RENDER MESH
    // ============================================

    MeshData renderMesh;

    if (!mesh.BuildRenderMesh(renderMesh))
    {
        BOX_LOG_ERROR(
            "Vertex move: failed to rebuild mesh"
        );
        return;
    }

    if (!entity.CreateFromMeshData(renderMesh))
    {
        BOX_LOG_ERROR(
            "Vertex move: failed to update GPU mesh"
        );
    }
}


void VertexEditController::EditConfirmMove()
{
    if (!m_isMoving)
        return;

    m_isMoving = false;
    m_moveAxis = VertexMoveAxis::None;

    m_moveDistance = 0.0f;
    m_usePreciseDistance = false;
    m_moveVertices.clear();

    BOX_LOG_INFO("Vertex move confirmed");
}


void VertexEditController::EditCancelMove(
    Entity& entity)
{
    if (!m_isMoving)
        return;

    MeshEditing& mesh =
        entity.GetEditableMesh();

    // Restore every moved vertex.
    for (const MoveVertex& item : m_moveVertices)
    {
        if (item.index >= mesh.GetVertexCount())
            continue;

        mesh.GetVertex(item.index).position =
            item.startPosition;
    }

    MeshData renderMesh;

    if (mesh.BuildRenderMesh(renderMesh))
    {
        entity.CreateFromMeshData(renderMesh);
    }

    m_isMoving = false;
    m_moveAxis = VertexMoveAxis::None;

    m_moveDistance = 0.0f;
    m_usePreciseDistance = false;
    m_moveVertices.clear();

    BOX_LOG_INFO("Vertex move cancelled");
}


void VertexEditController::SetMoveDistance(
    Entity& entity,
    float distance)
{
    if (!m_isMoving)
        return;

    m_moveDistance = distance;

    // The user has entered a numerical value.
    // Stop mouse movement overwriting it.
    m_usePreciseDistance = true;

    // Rebuild the vertex positions immediately.
    EditUpdateMove(entity);
}