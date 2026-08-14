#include <tools\EdgeEditController.h>
#include <BoxEngine.h> // Add this include to resolve incomplete type error
#include <entity/Entity.h>
#include <imgui/imgui.h>
#include <miniBoxLog.h>
#include <camera/Camera.h>
#include <limits>
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>


void EdgeEditController::HandleInput(BoxEngine& engine, bool viewportHovered, bool edgeModeActive, const ImVec2& viewportPosition,
	const ImVec2& viewportSize)
{
    if (!edgeModeActive)
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

    Entity* entity =
        engine.GetSelectedEntity();

    if (!entity)
    {
        return;
    }

    // Start moving the selected edge.
    if (!m_isMoving &&
        m_selectedEdge != InvalidEdge)
    {
        if (ImGui::IsKeyPressed(
            ImGuiKey_X,
            false))
        {
            EditBeginMove(
                *entity,
                EdgeMoveAxis::X
            );
        }
        else if (ImGui::IsKeyPressed(
            ImGuiKey_Y,
            false))
        {
            EditBeginMove(
                *entity,
                EdgeMoveAxis::Y
            );
        }
        else if (ImGui::IsKeyPressed(
            ImGuiKey_Z,
            false))
        {
            EditBeginMove(
                *entity,
                EdgeMoveAxis::Z
            );
        }
    }

    // Update active edge movement.
    if (m_isMoving)
    {
        EditUpdateMove(*entity);

        if (ImGui::IsMouseClicked(
            ImGuiMouseButton_Left))
        {
            EditConfirmMove();
        }
        else if (
            ImGui::IsKeyPressed(
                ImGuiKey_Escape,
                false) ||
            ImGui::IsMouseClicked(
                ImGuiMouseButton_Right))
        {
            EditCancelMove(*entity);
        }

        /*
         * Do not let the confirmation click
         * select another edge.
         */
        return;
    }

    // Normal edge selection.
    if (ImGui::IsMouseClicked(
        ImGuiMouseButton_Left))
    {
        PickEdge(
            engine,
            viewportPosition,
            viewportSize
        );
    }

}

void EdgeEditController::DrawEdge(BoxEngine& engine, const ImVec2& viewportPosition, const ImVec2& viewportSize, bool edgeModeActive)
{
    if (!edgeModeActive ||
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

    const std::vector<LogicalEdge> edges =
        BuildLogicalEdges(*entity);

    if (edges.empty())
    {
        return;
    }

    Camera& camera = engine.GetCamera();

    const float aspectRatio =
        viewportSize.x /
        viewportSize.y;

    const glm::mat4 modelViewProjection =
        camera.GetProjectionMatrix(
            aspectRatio
        ) *
        camera.GetViewMatrix() *
        entity->GetModelMatrix();

    ImDrawList* drawList =
        ImGui::GetWindowDrawList();

    for (std::size_t index = 0;
        index < edges.size();
        ++index)
    {
        ImVec2 screenA;
        ImVec2 screenB;

        if (!ProjectToScreen(
            edges[index].positionA,
            modelViewProjection,
            viewportPosition,
            viewportSize,
            screenA) ||
            !ProjectToScreen(
                edges[index].positionB,
                modelViewProjection,
                viewportPosition,
                viewportSize,
                screenB))
        {
            continue;
        }

       // const bool selected = index == m_selectedEdge;
        const bool selected = edges[index].editableEdgeIndex == m_selectedEdge;

        const ImU32 color =
            selected
            ? IM_COL32(30, 255, 70, 255)
            : IM_COL32(50, 170, 255, 255);

        const float thickness =
            selected ? 4.0f : 2.0f;

        drawList->AddLine(
            screenA,
            screenB,
            color,
            thickness
        );
    }
}

void EdgeEditController::ClearSelection(BoxEngine& engine)
{
	// Clear selected edges
	m_selectedEdge = InvalidEdge;
	Entity* entity = engine.GetSelectedEntity();
	if (entity)
	{
		entity->ClearSelectedEdges(); // this is in entity
	}
}

   // ###################################################################################################
   // ########################################### LoopCut ###############################################
   // ###################################################################################################

//void EdgeEditController::BeginLoopCut(Entity& entity, std::size_t edgeIndex, const ImVec2& viewportPosition, const ImVec2& viewportSize)
//{
//    if (edgeIndex == InvalidEdge)
//    {
//        return;
//    }
//    MeshEditing& mesh =
//        entity.GetEditableMesh();
//    if (edgeIndex >=
//        mesh.GetEdgeCount())
//    {
//        return;
//    }
//    m_meshBeforeLoopCut = mesh;
//    m_loopCutEdge = edgeIndex;
//    m_loopCutAmount = 0.5f;
//    m_loopCutStartMouse = ImGui::GetMousePos();
//	m_isLoopCutting = true;
//}

void EdgeEditController::BeginLoopCut(Entity& entity)
{
    if (m_selectedEdge == InvalidEdge)
    {
        return;
    }

    MeshEditing& mesh =
        entity.GetEditableMesh();

    if (m_selectedEdge >=
        mesh.GetEdgeCount())
    {
        return;
    }

    m_meshBeforeLoopCut = mesh;

    m_loopCutEdge = m_selectedEdge;

    m_loopCutAmount = 0.5f;

    m_loopCutStartMouse = ImGui::GetMousePos();

    m_isLoopCutting = true;
}


std::vector<EdgeEditController::LogicalEdge> EdgeEditController::BuildLogicalEdges(const Entity& entity) const
{

    std::vector<LogicalEdge> edges;

    const MeshEditing& editableMesh =
        entity.GetEditableMesh();

    const auto& editableEdges =
        editableMesh.GetEdges();

    if (editableEdges.empty())
    {
        return edges;
    }

    edges.reserve(
        editableEdges.size()
    );

    for (std::size_t edgeIndex = 0;
        edgeIndex < editableEdges.size();
        ++edgeIndex)
    {
        const EditEdge& editEdge =
            editableEdges[edgeIndex];

        if (editEdge.vertexA >=
            editableMesh.GetVertexCount() ||
            editEdge.vertexB >=
            editableMesh.GetVertexCount())
        {
            continue;
        }

        LogicalEdge edge;

        edge.editableEdgeIndex =
            edgeIndex;

        edge.vertexA =
            editEdge.vertexA;

        edge.vertexB =
            editEdge.vertexB;

        edge.positionA =
            editableMesh
            .GetVertex(editEdge.vertexA)
            .position;

        edge.positionB =
            editableMesh
            .GetVertex(editEdge.vertexB)
            .position;

        edges.push_back(
            edge
        );
    }

    return edges;

}

bool EdgeEditController::ProjectToScreen(const glm::vec3& localPosition, const glm::mat4& modelViewProjection,
    const ImVec2& viewportPosition, const ImVec2& viewportSize, ImVec2& outScreenPosition) const
{
    const glm::vec4 clipPosition =
        modelViewProjection *
        glm::vec4(localPosition, 1.0f);

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

float EdgeEditController::DistanceToLineSegment(const ImVec2& point, const ImVec2& lineStart, const ImVec2& lineEnd)
{
    const float lineX = lineEnd.x - lineStart.x;

    const float lineY = lineEnd.y - lineStart.y;

    const float lineLengthSquared =
        lineX * lineX +
        lineY * lineY;

    /*
     * Handle a projected edge whose two
     * endpoints occupy the same screen point.
     */
    if (lineLengthSquared <= 0.000001f)
    {
        const float deltaX = point.x - lineStart.x;

        const float deltaY = point.y - lineStart.y;

        return std::sqrt(
            deltaX * deltaX +
            deltaY * deltaY
        );
    }

    const float pointX = point.x - lineStart.x;

    const float pointY = point.y - lineStart.y;

    float t =
        (pointX * lineX +
            pointY * lineY) /
        lineLengthSquared;

    t = std::clamp(
        t,
        0.0f,
        1.0f
    );

    const float closestX =
        lineStart.x +
        t * lineX;

    const float closestY =
        lineStart.y +
        t * lineY;

    const float deltaX =
        point.x - closestX;

    const float deltaY =
        point.y - closestY;

    return std::sqrt(
        deltaX * deltaX +
        deltaY * deltaY
    );
}

bool EdgeEditController::PickEdge(BoxEngine& engine, const ImVec2& viewportPosition, const ImVec2& viewportSize)
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

    const std::vector<LogicalEdge> edges =
        BuildLogicalEdges(*entity);

    if (edges.empty())
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

    constexpr float pickDistance =
        8.0f;

    float closestDistance =
        std::numeric_limits<float>::max();

    std::size_t closestEdge =
        InvalidEdge;

    for (std::size_t index = 0;
        index < edges.size();
        ++index)
    {
        ImVec2 screenA;
        ImVec2 screenB;

        if (!ProjectToScreen(
            edges[index].positionA,
            modelViewProjection,
            viewportPosition,
            viewportSize,
            screenA) ||
            !ProjectToScreen(
                edges[index].positionB,
                modelViewProjection,
                viewportPosition,
                viewportSize,
                screenB))
        {
            continue;
        }

        const float distance =
            DistanceToLineSegment(
                mousePosition,
                screenA,
                screenB
            );

        if (distance <= pickDistance &&
            distance < closestDistance)
        {
            closestDistance =
                distance;

            closestEdge =
                index;
        }
    }

    if (closestEdge == InvalidEdge)
    {
        ClearSelection(engine);
        return false;
    }

    // m_selectedEdge = closestEdge;

    m_selectedEdge = edges[closestEdge].editableEdgeIndex;

    BOX_LOG_INFO(
        "Selected edge index: "
        << m_selectedEdge
    );

    return true;
}

void EdgeEditController::EditBeginMove(Entity& entity, EdgeMoveAxis axis)
{

    if (m_selectedEdge == InvalidEdge)
    {
        return;
    }

    MeshEditing& editableMesh =
        entity.GetEditableMesh();

    if (m_selectedEdge >=
        editableMesh.GetEdgeCount())
    {
        m_selectedEdge =
            InvalidEdge;

        return;
    }

    const EditEdge& edge =
        editableMesh.GetEdge(
            m_selectedEdge
        );

    if (edge.vertexA >=
        editableMesh.GetVertexCount() ||
        edge.vertexB >=
        editableMesh.GetVertexCount())
    {
        return;
    }

    // Remember which logical vertices
    // belong to the selected edge.
    m_moveVertexA =
        edge.vertexA;

    m_moveVertexB =
        edge.vertexB;

    // Store their original positions.
    m_startPositionA =
        editableMesh
        .GetVertex(m_moveVertexA)
        .position;

    m_startPositionB =
        editableMesh
        .GetVertex(m_moveVertexB)
        .position;

    m_moveAxis =
        axis;

    m_startMouse =
        ImGui::GetMousePos();

    m_isMoving = true;

}

void EdgeEditController::EditUpdateMove(Entity& entity)
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
    case EdgeMoveAxis::X:
        movement.x =
            deltaX *
            m_moveSensitivity;
        break;

    case EdgeMoveAxis::Y:
        movement.y =
            -deltaY *
            m_moveSensitivity;
        break;

    case EdgeMoveAxis::Z:
        movement.z =
            deltaX *
            m_moveSensitivity;
        break;

    case EdgeMoveAxis::None:
    default:
        return;
    }

    MeshEditing& editableMesh =
        entity.GetEditableMesh();

    // Move both logical endpoints.
    editableMesh
        .GetVertex(m_moveVertexA)
        .position =
        m_startPositionA +
        movement;

    editableMesh
        .GetVertex(m_moveVertexB)
        .position =
        m_startPositionB +
        movement;


    // Rebuild the render mesh.
    MeshData renderMesh;

    if (!editableMesh.BuildRenderMesh(
        renderMesh))
    {
        BOX_LOG_ERROR(
            "Edge move: failed to rebuild render mesh"
        );

        return;
    }

    if (!entity.CreateFromMeshData(
        renderMesh))
    {
        BOX_LOG_ERROR(
            "Edge move: failed to update GPU mesh"
        );

        return;

    }
}

void EdgeEditController::EditConfirmMove()
{

    if (!m_isMoving)
    {
        return;
    }

    m_isMoving = false;

    m_moveAxis = EdgeMoveAxis::None;
}

void EdgeEditController::EditCancelMove(Entity& entity)
{

    if (!m_isMoving)
    {
        return;
    }

    MeshEditing& editableMesh =
        entity.GetEditableMesh();

    editableMesh
        .GetVertex(m_moveVertexA)
        .position =
        m_startPositionA;

    editableMesh
        .GetVertex(m_moveVertexB)
        .position =
        m_startPositionB;

    MeshData renderMesh;

    if (editableMesh.BuildRenderMesh(renderMesh))
    {
        entity.CreateFromMeshData(renderMesh);
    }

    m_isMoving = false;

    m_moveAxis = EdgeMoveAxis::None;

 }