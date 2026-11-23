#pragma once
#include <glm/glm.hpp>
#include <imgui\imgui.h>
#include <vector>
#include <cstddef>

class BoxEngine;
class Entity;

enum class VertexMoveAxis
{
    None,
    X,
    Y,
    Z,
    Normal
};


class VertexEditController
{
public:
    void HandleInput(
        BoxEngine& engine,
        bool viewportHovered,
        bool vertexModeActive,
        const ImVec2& viewportPosition,
        const ImVec2& viewportSize
    );

    void DrawVertices(
        BoxEngine& engine,
        const ImVec2& viewportPosition,
        const ImVec2& viewportSize,
        bool vertexModeActive
    );

    // ============================================
    // PRECISE VERTEX MOVEMENT
    // ============================================

    bool IsMoving() const
    {
        return m_isMoving;
    }

    VertexMoveAxis GetMoveAxis() const
    {
        return m_moveAxis;
    }

    float GetMoveDistance() const
    {
        return m_moveDistance;
    }

    void SetMoveDistance(
        Entity& entity,
        float distance
    );


    void ClearSelection(
        BoxEngine& engine
    );

    void StartMove(Entity& entity, VertexMoveAxis axis)
    {
        if (!m_isMoving)
        {
            EditBeginMove(entity, axis);
        }
    }

    // ============================================
    // GUI CONFIRM / CANCEL
    // ============================================

    void ConfirmMove()
    {
        EditConfirmMove();
    }

    void CancelMove(Entity& entity)
    {
        EditCancelMove(entity);
    }

private:
    bool PickVertex(
        BoxEngine& engine,
        const ImVec2& viewportPosition,
        const ImVec2& viewportSize
    );

	// new functions for editing and moving vertices
    void EditBeginMove(
        Entity& entity,
        VertexMoveAxis axis
    );

    void EditUpdateMove(
        Entity& entity
    );

    void EditConfirmMove();

    void EditCancelMove(
        Entity& entity
    );

    

private:
    
    static constexpr std::size_t InvalidVertex = static_cast<std::size_t>(-1);

    std::size_t m_selectedVertex = InvalidVertex;

    bool m_isMoving = false;

    VertexMoveAxis m_moveAxis = VertexMoveAxis::None;

    // ============================================
    // MULTI-VERTEX MOVEMENT
    // ============================================

    struct MoveVertex
    {
        std::size_t index = 0;

        glm::vec3 startPosition{ 0.0f };
        glm::vec3 direction{ 0.0f };
    };

    std::vector<MoveVertex> m_moveVertices;

    ImVec2 m_startMouse = ImVec2(0.0f, 0.0f);

    float m_moveSensitivity = 0.01f;

    // Precise movement settings
    float m_moveDistance = 0.0f;

    bool m_usePreciseDistance = false;


};

