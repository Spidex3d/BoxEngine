#pragma once
#include <glm/glm.hpp>
#include <imgui\imgui.h>
#include <vector>
#include <cstddef>

class BoxEngine;
class Entity;

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

    void EditBeginMove(
        Entity& entity,
        std::size_t vertexIndex
    );

    void EditUpdateMove(
        Entity& entity
    );

    void EditConfirmMove();

    void EditCancelMove(
        Entity& entity
    );

private:
    static constexpr std::size_t InvalidVertex =
        static_cast<std::size_t>(-1);

    bool m_isMoving = false;

    glm::vec3 m_startVertexPosition =
        glm::vec3(0.0f);

    std::vector<std::size_t>
        m_selectedVertices;
};

