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
    Rotate,
    Scale,
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

private:
    void BeginMoveTransform(BoxEngine& engine, TransformAxis axis);

    void UpdateMoveTransform(BoxEngine& engine);

    void ConfirmTransform();

private:
    TransformToolType m_activeTool = TransformToolType::None;

    TransformAxis m_axis = TransformAxis::None;

    bool m_isTransforming = false;

    glm::vec3 m_startPosition = glm::vec3(0.0f);

    ImVec2 m_startMouse = ImVec2(0.0f, 0.0f);

    float m_moveSensitivity = 0.01f;
};
