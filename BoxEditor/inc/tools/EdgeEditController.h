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
        glm::vec3 positionA{ 0.0f };
        glm::vec3 positionB{ 0.0f };

        std::vector<std::size_t> verticesAtA;
        std::vector<std::size_t> verticesAtB;

        // The triangles that share this edge.
        std::vector<glm::vec3> adjacentFaceNormals;
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
    struct EdgeStartPosition
    {
        std::size_t index = 0;
        glm::vec3 position{ 0.0f };
    };

    static constexpr std::size_t InvalidEdge =
        static_cast<std::size_t>(-1);

    std::size_t m_selectedEdge = InvalidEdge;

    bool m_isMoving = false;


    EdgeMoveAxis m_moveAxis = EdgeMoveAxis  ::None;
   // glm::vec3 m_startEdgePosition = glm::vec3(0.0f);
    ImVec2 m_startMouse = ImVec2(0.0f, 0.0f);

   // std::vector<EdgeStartPosition>m_startEdgePositions;
    std::vector<EdgeStartPosition>m_startVertexPositions;

    //std::vector<std::size_t>m_selectedEdge;

	float m_moveSensitivity = 0.01f; // Sensitivity for moving, adjust as needed

   




};
