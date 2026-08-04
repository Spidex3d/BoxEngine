#pragma once
#include <glm/glm.hpp>
#include <imgui\imgui.h>
#include <cstddef>


class BoxEngine;
class Entity;

class VertexEditController
{
public:
    void HandleInput(
        BoxEngine& engine,
        bool viewportHovered,
        bool editModeActive
    );

    void DrawVertices(
        BoxEngine& engine,
        const ImVec2& viewportPosition,
        const ImVec2& viewportSize,
        bool editModeActive
    );

    void ClearSelection(
        BoxEngine& engine
    );

private:
    bool EditPickVertex(
        BoxEngine& engine
    );

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
    bool m_isMoving = false;

    std::size_t m_selectedVertex =
        static_cast<std::size_t>(-1);

    glm::vec3 m_startVertexPosition =
        glm::vec3(0.0f);
};

//class BoxEngine;
//class Entity;
//
//class VertexEditController
//{
//public:
//	// Handle input for vertex editing, including picking and moving vertices.
//    void HandleInput(BoxEngine& engine, bool viewportHovered, bool editModeActive);
//
//    void Draw();
//
//    void ClearSelection(BoxEngine& engine);
//
//private:
//    bool EditPickVertex(BoxEngine& engine);
//
//    void EditBeginMove(Entity& entity, std::size_t vertexIndex);
//
//    void EditUpdateMove(Entity& entity);
//
//    void EditConfirmMove();
//
//    void EditCancelMove(Entity& entity);
//private:
//    bool m_isMoving = false;
//
//    std::size_t m_selectedVertex = static_cast<std::size_t>(-1);
//
//    glm::vec3 m_startVertexPosition = glm::vec3(0.0f);
//};
