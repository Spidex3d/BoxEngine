#include "tools/VertexEditController.h"
#include <BoxEngine.h> // Add this include to resolve incomplete type error
#include <entity/Entity.h>
#include <imgui/imgui.h>
#include <miniBoxLog.h>
#include <camera/Camera.h>
#include <limits>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

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
            ImGui::IsMouseClicked(ImGuiMouseButton_Right))
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

        const bool selected =
            index == m_selectedVertex;

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
        m_selectedVertex =
            InvalidVertex;

        entity->ClearSelectedVertices();

        return false;
    }

    m_selectedVertex =
        closestVertex;

    entity->ClearSelectedVertices();

    entity->AddSelectedVertex(
        closestVertex
    );

    BOX_LOG_INFO(
        "Selected editable vertex index: "
        << m_selectedVertex
    );

    return true;
    
}


// ##########################################################################################
// ############################# new functions for moving vertices ##########################
// ##########################################################################################
void VertexEditController::EditBeginMove(Entity& entity, VertexMoveAxis axis)
{

    if (m_selectedVertex ==
        InvalidVertex)
    {
        return;
    }

    MeshEditing& editableMesh =
        entity.GetEditableMesh();

    if (m_selectedVertex >=
        editableMesh.GetVertexCount())
    {
        m_selectedVertex =
            InvalidVertex;

        return;
    }

    m_startVertexPosition =
        editableMesh
        .GetVertex(m_selectedVertex)
        .position;

    m_moveAxis =
        axis;

    m_startMouse =
        ImGui::GetMousePos();

    m_isMoving = true;

}

void VertexEditController::EditUpdateMove(Entity& entity)
{
    

    if (!m_isMoving)
    {
        return;
    }

    const ImVec2 currentMouse =
        ImGui::GetMousePos();

    const float deltaX =
        currentMouse.x -
        m_startMouse.x;

    const float deltaY =
        currentMouse.y -
        m_startMouse.y;

    glm::vec3 movement(0.0f);

    switch (m_moveAxis)
    {
    case VertexMoveAxis::X:
        movement.x =
            deltaX *
            m_moveSensitivity;
        break;

    case VertexMoveAxis::Y:
        movement.y =
            -deltaY *
            m_moveSensitivity;
        break;

    case VertexMoveAxis::Z:
        movement.z =
            deltaX *
            m_moveSensitivity;
        break;

    case VertexMoveAxis::None:
    default:
        return;
    }

    MeshEditing& editableMesh =
        entity.GetEditableMesh();

    editableMesh
        .GetVertex(m_selectedVertex)
        .position =
        m_startVertexPosition +
        movement;


    // Rebuild render mesh from the
    // modelling topology.
    MeshData renderMesh;

    if (!editableMesh.BuildRenderMesh(
        renderMesh))
    {
        BOX_LOG_ERROR(
            "Vertex move: failed to rebuild render mesh"
        );

        return;
    }

    if (!entity.CreateFromMeshData(
        renderMesh))
    {
        BOX_LOG_ERROR(
            "Vertex move: failed to update GPU mesh"
        );

        return;
    }
    
}

void VertexEditController::EditConfirmMove()
{
    m_isMoving = false;

    m_moveAxis =
        VertexMoveAxis::None;

}

void VertexEditController::EditCancelMove(Entity& entity)
{

    if (!m_isMoving)
    {
        return;
    }

    MeshEditing& editableMesh =
        entity.GetEditableMesh();

    if (m_selectedVertex <
        editableMesh.GetVertexCount())
    {
        editableMesh
            .GetVertex(m_selectedVertex)
            .position =
            m_startVertexPosition;
    }

    MeshData renderMesh;

    if (editableMesh.BuildRenderMesh(
        renderMesh))
    {
        entity.CreateFromMeshData(
            renderMesh
        );
    }

    m_isMoving = false;

    m_moveAxis =
        VertexMoveAxis::None;

}
