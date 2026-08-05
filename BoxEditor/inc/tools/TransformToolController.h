#pragma once
#include <glm/glm.hpp>
#include <imgui/imgui.h>

class BoxEngine;


enum class TransformAxis
{
    None,
    X,
    Y,
    Z
};

enum class TransformToolType
{
    None,
    Move,
    Scale,
    Rotate,
	Local,
	Snap,
    Drop
};

class TransformTools
{
public:
    TransformTools() = default;

    void SetActiveTool(TransformToolType tool);

    TransformToolType GetActiveTool() const;

    bool IsTransforming() const;

    void HandleInput(BoxEngine& engine, bool viewportHovered, bool objectModeActive);

    void CancelTransform(BoxEngine& engine);

    void DropToGrid(BoxEngine& engine);
private:
	// Move Transform
    void BeginMoveTransform(BoxEngine& engine, TransformAxis axis);
    void UpdateMoveTransform(BoxEngine& engine);
	// Scale Transform
	void BeginScaleTransform(BoxEngine& engine, TransformAxis axis);
	void UpdateScaleTransform(BoxEngine& engine);
	// Rotate Transform
	void BeginRotateTransform(BoxEngine& engine, TransformAxis axis);
	void UpdateRotateTransform(BoxEngine& engine);

	
	// Confirm the transform and update the starting values for the next transform
    void ConfirmTransform(BoxEngine& engine);

private:
    TransformToolType m_activeTool = TransformToolType::None;

    TransformAxis m_axis = TransformAxis::None;

    bool m_isTransforming = false;
    // move
	glm::vec3 m_startPosition = glm::vec3(0.0f); // Store the initial position of the entity when starting a transform
    //scale
	glm::vec3 m_startScale = glm::vec3(1.0f); // Store the initial scale of the entity when starting a transform
	// rotate
	glm::vec3 m_startRotation = glm::vec3(0.0f); // Store the initial rotation of the entity when starting a transform

    ImVec2 m_startMouse = ImVec2(0.0f, 0.0f);

	float m_moveSensitivity = 0.01f;    // Sensitivity for moving, adjust as needed
	float m_scaleSensitivity = 0.01f;   // Sensitivity for scaling, adjust as needed
	float m_rotateSensitivity = 0.2f;  // Sensitivity for rotation, adjust as needed

};
