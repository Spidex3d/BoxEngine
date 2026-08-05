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
    if (!m_isMoving && !m_selectedVertices.empty())
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


    /*if (!m_isMoving &&
        ImGui::IsMouseClicked(
            ImGuiMouseButton_Left))
    {
        PickVertex(
            engine,
            viewportPosition,
            viewportSize
        );
    }*/




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

    const MeshData& mesh = entity->GetMeshData();

    if (!mesh.IsValid())
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
        index < mesh.vertices.size();
        ++index)
    {
        const MeshVertex& vertex = mesh.vertices[index];

        const glm::vec4 clipPosition =
            modelViewProjection *
            glm::vec4(
                vertex.position,
                1.0f
            );

        /*
         * Behind the camera or too close to
         * the projection plane.
         */
        if (clipPosition.w <= 0.0001f)
        {
            continue;
        }

        const glm::vec3 ndc =
            glm::vec3(clipPosition) /
            clipPosition.w;

        /*
         * Skip points outside the visible
         * clip-space range.
         */
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

        /*
         * ImGui screen Y increases downward,
         * so the projected Y must be inverted.
         */
        const float screenY =
            viewportPosition.y +
            ((1.0f - ndc.y) * 0.5f) *
            viewportSize.y;
        // ############################# new
        bool alreadyDrawn = false;

        constexpr float epsilon =
            0.0001f;

        for (const glm::vec3& position : drawnPositions)
        {
            if (glm::length(
                position -
                vertex.position) <=
                epsilon)
            {
                alreadyDrawn = true;
                break;
            }
        }

        if (alreadyDrawn)
        {
            continue;
        }

        drawnPositions.push_back(vertex.position);

        bool selected = false;

        for (const std::size_t selectedIndex : m_selectedVertices)
        {
            if (selectedIndex <
                mesh.vertices.size())
            {
                const glm::vec3& selectedPosition =
                    mesh.vertices[
                        selectedIndex
                    ].position;

                if (glm::length(
                    selectedPosition -
                    vertex.position) <=
                    epsilon)
                {
                    selected = true;
                    break;
                }
            }
        }


        // ############################# new end
        const ImVec2 screenPosition(screenX, screenY);

		// draw a blue circle for unselected vertices and an orange circle for the selected vertex
        const ImU32 fillColor = selected
            ? IM_COL32(0, 255, 35, 255)
            : IM_COL32(50, 170, 255, 255); //IM_COL32(255, 145, 35, 255) orange
		
        const float radius = selected ? 6.0f : 4.0f;

        drawList->AddCircleFilled(
            screenPosition,
            radius,
            fillColor
        );
    }

}



void VertexEditController::ClearSelection(BoxEngine& engine)
{
    m_selectedVertices.clear();
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

    if (viewportSize.x <= 0.0f ||
        viewportSize.y <= 0.0f)
    {
        return false;
    }

    const MeshData& mesh =
        entity->GetMeshData();

    if (!mesh.IsValid())
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
        pickRadius *
        pickRadius;

    float closestDistanceSquared =
        std::numeric_limits<float>::max();

    std::size_t closestVertex = InvalidVertex;

    for (std::size_t index = 0;
        index < mesh.vertices.size();
        ++index)
    {
        const glm::vec4 clipPosition =
            modelViewProjection *
            glm::vec4(
                mesh.vertices[index].position,
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

        const float differenceX =
            mousePosition.x -
            screenX;

        const float differenceY =
            mousePosition.y -
            screenY;

        const float distanceSquared =
            differenceX * differenceX +
            differenceY * differenceY;

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
       
        m_selectedVertices.clear();

        entity->ClearSelectedVertices();

        return false;
    }

    const glm::vec3 selectedPosition =
        mesh.vertices[
            closestVertex
        ].position;

    m_selectedVertices =
        FindVerticesAtPosition(
            *entity,
            selectedPosition
        );

    entity->ClearSelectedVertices();

    for (const std::size_t index :
    m_selectedVertices)
    {
        entity->AddSelectedVertex(
            index
        );
    }

    BOX_LOG_INFO(
        "Selected vertex index: "
        << closestVertex
    );

    return true;
}

std::vector<std::size_t>
VertexEditController::FindVerticesAtPosition(
    const Entity& entity,
    const glm::vec3& position) const
{
    std::vector<std::size_t>
        matchingVertices;

    const MeshData& mesh =
        entity.GetMeshData();

    constexpr float epsilon =
        0.0001f;

    for (std::size_t index = 0;
        index < mesh.vertices.size();
        ++index)
    {
        const glm::vec3& candidate =
            mesh.vertices[index].position;

        const bool samePosition =
            glm::length(
                candidate -
                position
            ) <= epsilon;

        if (samePosition)
        {
            matchingVertices.push_back(
                index
            );
        }
    }

    return matchingVertices;
}
// ##########################################################################################
// ############################# new functions for moving vertices ##########################
// ##########################################################################################
void VertexEditController::EditBeginMove(Entity& entity, VertexMoveAxis axis)
{
    if (m_selectedVertices.empty())
    {
        return;
    }

    const MeshData& mesh = entity.GetMeshData();

    m_startVertexPositions.clear();

    m_startVertexPositions.reserve(
        m_selectedVertices.size()
    );

    for (const std::size_t index : m_selectedVertices)
    {
        if (index >=
            mesh.vertices.size())
        {
            continue;
        }

        VertexStartPosition start;

        start.index = index;

        start.position = mesh.vertices[index].position;

        m_startVertexPositions.push_back(start);
    }

    if (m_startVertexPositions.empty())
    {
        return;
    }

    m_moveAxis = axis;

    m_startMouse = ImGui::GetMousePos();

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

    for (const VertexStartPosition& start :
        m_startVertexPositions)
    {
        entity.SetVertexPosition(
            start.index,
            start.position + movement
        );
    }

    /*
     * Send the changed CPU mesh data back
     * to the existing OpenGL VBO.
     */
    entity.UploadMeshData();
}

void VertexEditController::EditConfirmMove()
{
    m_isMoving = false;

    m_moveAxis =
        VertexMoveAxis::None;

    m_startVertexPositions.clear();
}

void VertexEditController::EditCancelMove(Entity& entity)
{
    for (const VertexStartPosition& start :
        m_startVertexPositions)
    {
        entity.SetVertexPosition(
            start.index,
            start.position
        );
    }

    entity.UploadMeshData();

    m_isMoving = false;

    m_moveAxis =
        VertexMoveAxis::None;

    m_startVertexPositions.clear();
}
