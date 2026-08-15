#pragma once
#include <glm/glm.hpp>
#include <imgui\imgui.h>
#include <vector>
#include <cstddef>

#include <mesh/MeshEditing.h>

class BoxEngine;
class Entity;

enum class ExtrudeAxis
{
    None,
    X,
    Y,
    Z
};

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

	// ############################## for extruding the selected face ##############################
    void BeginExtrude(Entity& entity);


    bool m_isExtruding = false;

    std::size_t m_extrudeFace =
        InvalidFace;

    ExtrudeAxis m_extrudeAxis = ExtrudeAxis::None;

    float m_extrudeAmount = 0.0f;

    ImVec2 m_extrudeStartMouse = ImVec2(0.0f, 0.0f);

    MeshEditing m_meshBeforeExtrude;

	// ######################################## useful for our Object Explorer ############################
    
    bool IsExtruding() const
    {
        return m_isExtruding;
    }

    float GetExtrudeAmount() const
    {
        return m_extrudeAmount;
    }

    ExtrudeAxis GetExtrudeAxis() const
    {
        return m_extrudeAxis;
    }
	// ######## Object Explorer ###########################
    void SetExtrudeAmount(Entity& entity, float amount);

    void SetExtrudeAxis(Entity& entity, ExtrudeAxis axis);

    void ConfirmExtrude(Entity& entity);

    void CancelExtrude(Entity& entity);

	// ##########################################################################################################
    // ################################################### Inset Face ###########################################
	// ##########################################################################################################
   
    void BeginInset(Entity& entity);

    bool m_isInsetting = false;

    std::size_t m_insetFace = InvalidFace;

    float m_insetAmount = 0.0f;

    ImVec2 m_insetStartMouse = ImVec2(0.0f, 0.0f);

    MeshEditing m_meshBeforeInset;

    // ######## Object Explorer ###########################
	void SetInsetAmount(Entity& entity, float amount); // Set the amount of inset for the selected face


    void ConfirmInset(Entity& entity);

    void CancelInset(Entity& entity);
   

    bool IsInsetting() const
    {
        return m_isInsetting;
    }
    float GetInsetAmount() const
    {
        return m_insetAmount;
    }

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

        std::size_t editableFaceIndex = static_cast<std::size_t>(-1);

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


    std::vector<std::size_t>m_editableFaceVertices;

    std::vector<glm::vec3>m_editableStartPositions;

    bool ProjectToScreen(const glm::vec3& localPosition, const glm::mat4& modelViewProjection,
        const ImVec2& viewportPosition, const ImVec2& viewportSize, ImVec2& outScreenPosition) const;

	

    bool PickFace(BoxEngine& engine, const ImVec2& viewportPosition,
        const ImVec2& viewportSize);

    // new functions for editing and moving vertices
    void EditBeginMove(Entity& entity, FaceMoveAxis axis);

    void EditUpdateMove(Entity& entity);

    void EditConfirmMove();

    void EditCancelMove(Entity& entity);

    // ######## Object Explorer Extrude ###########################
    void UpdateExtrudeMesh(Entity& entity);

    // ######### Object Explorer Inset ############################
	void UpdateInsetMesh(Entity& entity);

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
