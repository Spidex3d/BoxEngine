#pragma once
#include <glm/glm.hpp>
#include <imgui/imgui.h>

#include <cstddef>

class BoxEngine;
class Entity;

enum class VertexDrawPlane
{
    None,
    X,
    Y,
    Z
};

class VertexDrawController
{
public:

    void HandleInput(
        BoxEngine& engine,
        bool viewportHovered,
        bool vertexModeActive,
        const ImVec2& viewportPosition,
        const ImVec2& viewportSize
    );

    bool IsDrawing() const
    {
        return m_isDrawing;
    }

private:

    bool BeginDraw(Entity& entity);

    bool CreateVertexFromMouse(
        BoxEngine& engine,
        Entity& entity,
        const ImVec2& viewportPosition,
        const ImVec2& viewportSize
    );

    bool MouseToPlane(
        BoxEngine& engine,
        Entity& entity,
        const ImVec2& viewportPosition,
        const ImVec2& viewportSize,
        glm::vec3& outPosition
    ) const;

    void FinishDraw();

private:

    static constexpr std::size_t InvalidVertex =
        static_cast<std::size_t>(-1);

    bool m_isDrawing = false;

    VertexDrawPlane m_plane =
        VertexDrawPlane::None;

    std::size_t m_startVertex =
        InvalidVertex;

    std::size_t m_lastVertex =
        InvalidVertex;

    glm::vec3 m_planePoint{ 0.0f };
};