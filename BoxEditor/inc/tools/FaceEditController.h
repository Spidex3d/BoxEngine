#pragma once
#include <glm/glm.hpp>
#include <imgui\imgui.h>
#include <vector>
#include <cstddef>

class BoxEngine;
class Entity;

enum class FaceMoveAxis
{
    None,
    X,
    Y,
    Z
};

class FaceEditController
{
public:

    void HandleInput(BoxEngine& engine, bool viewportHovered, bool faceModeActive,
        const ImVec2& viewportPosition, const ImVec2& viewportSize);

    void DrawFace(BoxEngine& engine, const ImVec2& viewportPosition,
        const ImVec2& viewportSize, bool faceModeActive);

    void ClearSelection(BoxEngine& engine);


    // ############################## for modifying the selected face ##############################
        std::size_t GetSelectedFace() const
    {
        return m_selectedFace;
    }

    bool HasSelectedFace() const
    {
        return m_selectedFace != InvalidFace;
    }

    // #################################################################################################

private:
    struct LogicalFace
    {
		// The positions of the four vertices of the face.
        glm::vec3 positionA{ 0.0f };
        glm::vec3 positionB{ 0.0f };
        glm::vec3 positionC{ 0.0f };
        glm::vec3 positionD{ 0.0f };
		// The indices of the vertices in the mesh that correspond to the face's vertices.
        std::vector<std::size_t> verticesAtA;
        std::vector<std::size_t> verticesAtB;
		std::vector<std::size_t> verticesAtC;
		std::vector<std::size_t> verticesAtD;

        glm::vec3 normal{ 0.0f };
        glm::vec3 centre{ 0.0f };
    };

    static bool PointInTriangle(
        const ImVec2& point,
        const ImVec2& a,
        const ImVec2& b,
        const ImVec2& c
    );
  

    std::vector<LogicalFace>BuildLogicalFace(const Entity& entity) const;


    bool ProjectToScreen(const glm::vec3& localPosition, const glm::mat4& modelViewProjection,
        const ImVec2& viewportPosition, const ImVec2& viewportSize, ImVec2& outScreenPosition) const;

	

    bool PickFace(BoxEngine& engine, const ImVec2& viewportPosition,
        const ImVec2& viewportSize);

    // new functions for editing and moving vertices
    void EditBeginMove(Entity& entity, FaceMoveAxis axis);

    void EditUpdateMove(Entity& entity);

    void EditConfirmMove();

    void EditCancelMove(Entity& entity);

private:
    struct FaceStartPosition
    {
        std::size_t index = 0;
        glm::vec3 position{ 0.0f };
    };

    static constexpr std::size_t InvalidFace =
        static_cast<std::size_t>(-1);

    std::size_t m_selectedFace = InvalidFace;

    bool m_isMoving = false;


    FaceMoveAxis m_moveAxis = FaceMoveAxis::None;
    ImVec2 m_startMouse = ImVec2(0.0f, 0.0f);

    std::vector<FaceStartPosition>m_startVertexPositions;

    float m_moveSensitivity = 0.01f; // Sensitivity for moving, adjust as needed
};
