#include "tools/TransformToolController.h"

#include <BoxEngine.h>
#include <entity/Entity.h>
#include <imgui/imgui.h>

void TransformTools::SetActiveTool(TransformToolType tool)
{
    m_activeTool = tool;
}

TransformToolType
TransformTools::GetActiveTool() const
{
    return m_activeTool;
}

bool TransformTools::IsTransforming() const
{
    return m_isTransforming;
}

void TransformTools::HandleInput(BoxEngine& engine, bool viewportHovered, bool objectModeActive)
{
    if (!objectModeActive)
    {
        if (m_isTransforming)
        {
            CancelTransform(engine);
        }

        return;
    }

    if (m_activeTool != TransformToolType::Move)
    {
        return;
    }

    if (!viewportHovered)
    {
        return;
    }

    if (!m_isTransforming)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_X, false))
        {
            BeginMoveTransform(engine, TransformAxis::X);
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_Y, false))
        {
            BeginMoveTransform(engine, TransformAxis::Y);
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_Z, false))
        {
            BeginMoveTransform(engine, TransformAxis::Z
            );
        }
    }

    if (m_isTransforming)
    {
        UpdateMoveTransform(engine);

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            ConfirmTransform();
        }
        else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
           // (ImGui::IsKeyPressed(ImGuiKey_Escape, false) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            CancelTransform(engine);
        }
    }
}

void TransformTools::BeginMoveTransform(BoxEngine& engine, TransformAxis axis)
{
    Entity* entity = engine.GetSelectedEntity();

    if (!entity)
    {
        return;
    }

    m_axis = axis;

    m_startPosition = entity->GetPosition();

    m_startMouse = ImGui::GetMousePos();

    m_isTransforming = true;
}

void TransformTools::UpdateMoveTransform(BoxEngine& engine)
{
    Entity* entity = engine.GetSelectedEntity();

    if (!entity)
    {
        m_isTransforming = false;
        m_axis = TransformAxis::None;
        return;
    }

    const ImVec2 currentMouse =
        ImGui::GetMousePos();

    const float deltaX =
        currentMouse.x -
        m_startMouse.x;

    const float deltaY =
        currentMouse.y -
        m_startMouse.y;

    glm::vec3 position =
        m_startPosition;

    switch (m_axis)
    {
    case TransformAxis::X:
        position.x +=
            deltaX *
            m_moveSensitivity;
        break;

    case TransformAxis::Y:
        position.y -=
            deltaY *
            m_moveSensitivity;
        break;

    case TransformAxis::Z:
        position.z +=
            deltaX *
            m_moveSensitivity;
        break;

    case TransformAxis::None:
    default:
        return;
    }

    entity->SetPosition(position);
}

void TransformTools::ConfirmTransform()
{
    m_isTransforming = false;
    m_axis = TransformAxis::None;
}

void TransformTools::CancelTransform(BoxEngine& engine)
{
    Entity* entity = engine.GetSelectedEntity();

    if (entity)
    {
        entity->SetPosition(
            m_startPosition
        );
    }

    m_isTransforming = false;
    m_axis = TransformAxis::None;
}