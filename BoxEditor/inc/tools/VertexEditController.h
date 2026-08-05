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
    Z
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

    void ClearSelection(
        BoxEngine& engine
    );

private:
    bool PickVertex(
        BoxEngine& engine,
        const ImVec2& viewportPosition,
        const ImVec2& viewportSize
    );

    std::vector<std::size_t>
        FindVerticesAtPosition(
            const Entity& entity,
            const glm::vec3& position
        ) const;

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
    struct VertexStartPosition
    {
        std::size_t index = 0;
        glm::vec3 position{ 0.0f };
    };

    static constexpr std::size_t InvalidVertex =
        static_cast<std::size_t>(-1);

    bool m_isMoving = false;

   
    VertexMoveAxis m_moveAxis = VertexMoveAxis::None;
    glm::vec3 m_startVertexPosition = glm::vec3(0.0f);
    ImVec2 m_startMouse = ImVec2(0.0f, 0.0f);

    std::vector<VertexStartPosition>m_startVertexPositions;

    std::vector<std::size_t>m_selectedVertices;

    float m_moveSensitivity = 0.01f;
};

