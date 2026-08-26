#pragma once
#include <glm/glm.hpp>
#include <imgui\imgui.h>
#include <vector>
#include <cstddef>

#include <mesh/MeshEditing.h>

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
    // ###################################################################################################
    // ########################################### FaceCut ###############################################
    // ###################################################################################################
    void BeginFaceCut(Entity& entity);

    MeshEditing m_meshBeforeFaceCut; // Store the mesh state before the face cut operation

	// ###################################################################################################
	// ########################################### LoopCut ###############################################
	// ###################################################################################################
    void EdgeLoopCutMove(Entity& entity);   

    void CancelLoopCut(Entity& entity);

    void BeginLoopCut(Entity& entity);

	MeshEditing m_meshBeforeLoopCut; // Store the mesh state before the loop cut operation

	// this function is a shared function for both loop cut and face cut, it will be called when the user confirms the cut operation.
    std::size_t GetSelectedEdge() const
    {
        return m_selectedEdge;
    }

	// this function is a shared function for both loop cut and face cut, it will be called when the user confirms the cut operation.
    bool HasSelectedEdge() const
    {
        return m_selectedEdge != InvalidEdge;
    }
	// ###################################### Getters for loop cut and face cut variables ######################################
    bool IsLoopCutting() const
    {
        return m_isLoopCutting;
    }

    float GetLoopCutAmount() const
    {
        return m_loopCutAmount;
    }

    void SetLoopCutAmount(Entity& entity, float amount);

    
    void ConfirmLoopCut(Entity& entity);

    // ###################################################################################################
    // ########################################## Bevel ##################################################
    // ###################################################################################################
    void EdgeBevelMove(Entity& entity);

    void CancelBevel(Entity& entity);

    void BeginBevel(Entity& entity);

    void SetBevelValues(Entity& entity, float width, int segments, float profile);

    MeshEditing m_meshBeforeBevel;

    bool IsBeveling() const
    {
        return m_isBeveling;
    }

    int GetBevelSegments() const
    {
        return m_bevelSegments;
    }
    float GetBevelProfile() const
    {
        return m_bevelProfile;
    }
    float GetBevelWidth() const
    {
        return m_bevelWidth;
	}

    void ConfirmBevel(Entity& entity);

	
private: // loopcuts - facecut
    // face cut variables
    std::size_t m_faceCutEdge = InvalidEdge;
    float m_faceCutAmount = 0.5f;
    ImVec2 m_faceCutStartMouse = ImVec2(0.0f, 0.0f);
    bool m_isFaceCutting = false;
	// loop cut variables
    std::size_t m_loopCutEdge = InvalidEdge;
    float m_loopCutAmount = 0.5f;
    ImVec2 m_loopCutStartMouse = ImVec2(0.0f, 0.0f);
    bool m_isLoopCutting = false;
	// bevel variables
    std::size_t m_bevelEdge = InvalidEdge;
    int m_bevelSegments = 1;
    float m_bevelWidth = 0.1f;
	float m_bevelProfile = 0.5f; // default profile value, can be adjusted as needed
    ImVec2 m_bevelStartMouse = ImVec2(0.0f, 0.0f);
	bool m_isBeveling = false;
	

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

    void EditCancelMove(Entity& entity);

    // ###################################################################################################
    // ####################################### Multi Select Edges #########################################
    // ###################################################################################################

    // Multi-edge scale "S"
    void EditBeginScale(Entity& entity);
    void EditUpdateScale(Entity& entity);
    void EditConfirmScale();
    void EditCancelScale(Entity& entity);

    // Multi-edge Move "G"
	void EditBeginMultiMove(Entity& entity, EdgeMoveAxis axis);
    void EditUpdateMultiMove(Entity& entity);
    void EditConfirmMultiMove();
	void EditCancelMultiMove(Entity& entity);
    void SetMultiMoveAxis(Entity& entity, EdgeMoveAxis axis);

	// Multi-edge Rotate "R"
    void EditBeginMultiRotate(Entity& entity, EdgeMoveAxis axis);
    void EditUpdateMultiRotate(Entity& entity);
    void EditConfirmMultiRotate();
    void EditCancelMultiRotate(Entity& entity);
    void SetMultiRotateAxis(Entity& entity, EdgeMoveAxis axis);

private:
    // -------------------------------------------------
    // Edge move
    // -------------------------------------------------
    std::size_t m_moveVertexA = 0;
    std::size_t m_moveVertexB = 0;

    glm::vec3 m_startPositionA{ 0.0f };
    glm::vec3 m_startPositionB{ 0.0f };

    bool m_isMoving = false;

    EdgeMoveAxis m_moveAxis = EdgeMoveAxis  ::None;

    ImVec2 m_startMouse = ImVec2(0.0f, 0.0f);

	float m_moveSensitivity = 0.01f; // Sensitivity for moving, adjust as needed

    // -------------------------------------------------
    // Multi-edge scale
    // -------------------------------------------------

    bool m_isScaling = false;
    glm::vec3 m_scaleCenter{ 0.0f };
    std::vector<std::size_t>m_scaleVertices;
    std::vector<glm::vec3>m_scaleStartPositions;
    ImVec2 m_scaleStartMouse = ImVec2(0.0f, 0.0f);
	float m_scaleSensitivity = 0.01f; // Sensitivity for scaling, adjust as needed

    // -------------------------------------------------
    // Multi-edge move
    // -------------------------------------------------
	bool m_isMultiMoving = false;
    EdgeMoveAxis m_multiMoveAxis = EdgeMoveAxis::None;
	std::vector<std::size_t> m_multiMoveVertices;
	std::vector<glm::vec3> m_multiMoveStartPositions;
	ImVec2 m_multiMoveStartMouse = ImVec2(0.0f, 0.0f);
	float m_multiMoveSensitivity = 0.01f; // Sensitivity for multi-edge moving, adjust as needed
    // -------------------------------------------------
    // Multi-edge rotate
    // -------------------------------------------------
    bool m_isMultiRotating = false;
    EdgeMoveAxis m_multiRotateAxis = EdgeMoveAxis::None;
    glm::vec3 m_multiRotateCenter{ 0.0f };
    std::vector<std::size_t>m_multiRotateVertices;
    std::vector<glm::vec3>m_multiRotateStartPositions;
    ImVec2 m_multiRotateStartMouse = ImVec2(0.0f, 0.0f);
    float m_multiRotateSensitivity = 0.2f;


    // -------------------------------------------------
    // Selection
    // -------------------------------------------------

    static constexpr std::size_t
        InvalidEdge =
        static_cast<std::size_t>(-1);

    std::size_t m_selectedEdge =
        InvalidEdge;

};
