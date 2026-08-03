#include "tools/TransformToolController.h"

#include <BoxEngine.h>
#include <entity/Entity.h>
#include <imgui/imgui.h>
#include <miniBoxLog.h>

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
void TransformTools::HandleInput(
    BoxEngine& engine,
    bool viewportHovered,
    bool objectModeActive)
{
    if (!objectModeActive)
    {
        if (m_isTransforming)
        {
            CancelTransform(engine);
        }

        return;
    }

    if (!viewportHovered)
    {
        return;
    }

    Entity* entity =
        engine.GetSelectedEntity();

    if (!entity)
    {
        return;
    }

    // Start a transform.
    if (!m_isTransforming)
    {
        TransformAxis selectedAxis = TransformAxis::None;

        if (ImGui::IsKeyPressed(
            ImGuiKey_X,
            false))
        {
            selectedAxis = TransformAxis::X;
        }
        else if (ImGui::IsKeyPressed(
            ImGuiKey_Y,
            false))
        {
            selectedAxis = TransformAxis::Y;
        }
        else if (ImGui::IsKeyPressed(
            ImGuiKey_Z,
            false))
        {
            selectedAxis = TransformAxis::Z;
        }

        if (selectedAxis != TransformAxis::None)
        {
            switch (m_activeTool)
            {
            case TransformToolType::Move:
                BeginMoveTransform(
                    engine,
                    selectedAxis
                );
                break;

            case TransformToolType::Scale:
                BeginScaleTransform(
                    engine,
                    selectedAxis
                );
                break;
            case TransformToolType::Rotate:
                BeginRotateTransform(
                    engine,
                    selectedAxis
                );
				break;
            default:
                break;
            }
        }
    }

    // Update an active transform.
    if (m_isTransforming)
    {
        switch (m_activeTool)
        {
        case TransformToolType::Move:
            UpdateMoveTransform(engine);
            break;

        case TransformToolType::Scale:
            UpdateScaleTransform(engine);
            break;
        case TransformToolType::Rotate:
            UpdateRotateTransform(engine);
			break;
        default:
            break;
        }

        if (ImGui::IsMouseClicked(
            ImGuiMouseButton_Left))
        {
            ConfirmTransform(engine);
        }
        else if (ImGui::IsMouseClicked(
                ImGuiMouseButton_Right))
        {
            CancelTransform(engine);
        }
    }
}
// #############################################################################################################################
// ###################################################### Move Transform ######################################################
// #############################################################################################################################

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

    const ImVec2 currentMouse = ImGui::GetMousePos();

    const float deltaX = currentMouse.x - m_startMouse.x;

    const float deltaY = currentMouse.y - m_startMouse.y;

    glm::vec3 position = m_startPosition;

    switch (m_axis)
    {
    case TransformAxis::X:
        position.x += deltaX *
            m_moveSensitivity;
        break;

    case TransformAxis::Y:
        position.y -= deltaY *
            m_moveSensitivity;
        break;

    case TransformAxis::Z:
        position.z += deltaX *
            m_moveSensitivity;
        break;

    case TransformAxis::None:
    default:
        return;
    }

    entity->SetPosition(position);
}
// #############################################################################################################################
// ###################################################### Scale Transform ######################################################
// #############################################################################################################################
void TransformTools::BeginScaleTransform(BoxEngine& engine, TransformAxis axis)
{
    Entity* entity = engine.GetSelectedEntity();

    if (!entity)
    {
        return;
    }

    m_axis = axis;
	m_startScale = entity->GetScale();
    m_startMouse = ImGui::GetMousePos();
    m_isTransforming = true;
}

void TransformTools::UpdateScaleTransform(BoxEngine& engine)
{
    Entity* entity =
        engine.GetSelectedEntity();

    if (!entity)
    {
        m_isTransforming = false;
        m_axis = TransformAxis::None;

        return;
    }

    const ImVec2 currentMouse =
        ImGui::GetMousePos();

    const float deltaX = currentMouse.x - m_startMouse.x;

    const float deltaY = currentMouse.y - m_startMouse.y;

    glm::vec3 scale = m_startScale;

    switch (m_axis)
    {
    case TransformAxis::X:
        scale.x += deltaX *
            m_scaleSensitivity;
        break;

    case TransformAxis::Y:
        scale.y -= deltaY *
            m_scaleSensitivity;
        break;

    case TransformAxis::Z:
        scale.z += deltaX *
            m_scaleSensitivity;
        break;

    case TransformAxis::None:
    default:
        return;
    }

    constexpr float minimumScale = 0.01f;

    scale.x = glm::max(scale.x, minimumScale);

    scale.y = glm::max(scale.y, minimumScale);

    scale.z = glm::max(scale.z, minimumScale);

    entity->SetScale(scale);
   
}
// #############################################################################################################################
// ###################################################### Rotation Transform ###################################################
// #############################################################################################################################

void TransformTools::BeginRotateTransform(BoxEngine& engine, TransformAxis axis)
{
    Entity* entity = engine.GetSelectedEntity();

    if (!entity)
    {
        return;
    }

    m_axis = axis;
    m_startRotation = entity->GetRotation();
    m_startMouse = ImGui::GetMousePos();
    m_isTransforming = true;
}

void TransformTools::UpdateRotateTransform(BoxEngine& engine)
{
    Entity* entity = engine.GetSelectedEntity();

    if (!entity)
    {
        m_isTransforming = false;
        m_axis = TransformAxis::None;

        return;
    }

    const ImVec2 currentMouse = ImGui::GetMousePos();

    const float deltaX = currentMouse.x - m_startMouse.x;

    const float deltaY = currentMouse.y - m_startMouse.y;

    glm::vec3 rotation = m_startRotation;

    switch (m_axis)
    {
    case TransformAxis::X:
        rotation.x += deltaX *
            m_rotateSensitivity;
        break;

    case TransformAxis::Y:
        rotation.y -= deltaY *
            m_rotateSensitivity;
        break;

    case TransformAxis::Z:
        rotation.z += deltaX *
            m_rotateSensitivity;
        break;

    case TransformAxis::None:
    default:
        return;
    }
        
    entity->SetRotation(rotation);
    
}

// #############################################################################################################################
// ###################################################### Drop Transform #######################################################
// #############################################################################################################################




void TransformTools::ConfirmTransform(BoxEngine& engine)
{
    Entity* entity = engine.GetSelectedEntity();

    if (entity)
    {
        switch (m_activeTool)
        {
        case TransformToolType::Move:
            m_startPosition = entity->GetPosition();
            break;

        case TransformToolType::Scale:
            m_startScale = entity->GetScale();
            break;
        case TransformToolType::Rotate:
            m_startRotation = entity->GetRotation();
			break;
        default:
            break;
        }
    }

    m_isTransforming = false;
    m_axis = TransformAxis::None;
}

void TransformTools::CancelTransform(BoxEngine& engine)
{
    Entity* entity = engine.GetSelectedEntity();

    if (entity)
    {
        switch (m_activeTool)
        {
        case TransformToolType::Move:
            entity->SetPosition(m_startPosition);
            break;

        case TransformToolType::Scale:
            entity->SetScale(m_startScale);
            break;
		case TransformToolType::Rotate:
            entity->SetRotation(m_startRotation);
			break;  

        default:
            break;
        }
    }

    m_isTransforming = false;
    m_axis = TransformAxis::None;
}