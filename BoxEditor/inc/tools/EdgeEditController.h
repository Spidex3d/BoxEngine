#pragma once
#include <glm/glm.hpp>
#include <imgui\imgui.h>
#include <vector>
#include <cstddef>

class BoxEngine;
class Entity;

enum class EdgeMoveAxis
{
    None,
    X,
    Y,
    Z
};

class EdgeEditController
{
public:

    void HandleInput(
        BoxEngine& engine,
        bool viewportHovered,
        bool edgeModeActive,
        const ImVec2& viewportPosition,
        const ImVec2& viewportSize
    );

    void DrawEdge(
        BoxEngine& engine,
        const ImVec2& viewportPosition,
        const ImVec2& viewportSize,
        bool edgeModeActive 
    );

    void ClearSelection(
        BoxEngine& engine
    );

private:
    struct LogicalEdge
    {
        // Actual edge index inside MeshEditing.
        std::size_t editableEdgeIndex = static_cast<std::size_t>(-1);

        // Logical endpoint vertex indices.
        std::size_t vertexA = 0;
        std::size_t vertexB = 0;

        // Current logical positions.
        glm::vec3 positionA{ 0.0f };
        glm::vec3 positionB{ 0.0f };

    };
   
    std::vector<LogicalEdge>BuildLogicalEdges(const Entity& entity) const;


    bool ProjectToScreen(
        const glm::vec3& localPosition,
        const glm::mat4& modelViewProjection,
        const ImVec2& viewportPosition,
        const ImVec2& viewportSize,
        ImVec2& outScreenPosition
    ) const;

    static float DistanceToLineSegment(
        const ImVec2& point,
        const ImVec2& lineStart,
        const ImVec2& lineEnd
    );

    bool PickEdge(BoxEngine& engine, const ImVec2& viewportPosition,
        const ImVec2& viewportSize);

    // new functions for editing and moving vertices
    void EditBeginMove(
        Entity& entity,
        EdgeMoveAxis axis
    );

    void EditUpdateMove(
        Entity& entity
    );

    void EditConfirmMove();

    void EditCancelMove(
        Entity& entity
    );

private:
    // Edge Start Position
    std::size_t m_moveVertexA = 0;
    std::size_t m_moveVertexB = 0;

    glm::vec3 m_startPositionA{ 0.0f };
    glm::vec3 m_startPositionB{ 0.0f };

    static constexpr std::size_t InvalidEdge = static_cast<std::size_t>(-1);
	// ##################################################################################################

    std::size_t m_selectedEdge = InvalidEdge;

    bool m_isMoving = false;

    EdgeMoveAxis m_moveAxis = EdgeMoveAxis  ::None;

    ImVec2 m_startMouse = ImVec2(0.0f, 0.0f);

	float m_moveSensitivity = 0.01f; // Sensitivity for moving, adjust as needed

};
