#include "tools/VertexEditController.h"
#include <BoxEngine.h> // Add this include to resolve incomplete type error
#include <entity/Entity.h>
#include <imgui/imgui.h>
#include <miniBoxLog.h>
#include <camera/Camera.h>

#include <glm/gtc/matrix_transform.hpp>


void VertexEditController::HandleInput(BoxEngine& engine, bool viewportHovered, bool vertexModeActive)
{
    if (vertexModeActive)
    {
        static bool once = false;

        if (!once)
        {
            BOX_LOG_INFO("VertexEditController is alive!");

            once = true;
        }
    }
    else
    {
        static bool once = false;
        once = false;
    }

}
void VertexEditController::DrawVertices(BoxEngine& engine, const ImVec2& viewportPosition, const ImVec2& viewportSize, bool vertexModeActive)
{
    if (!vertexModeActive)
    {
        return;
    }

    if (viewportSize.x <= 0.0f || viewportSize.y <= 0.0f)
    {
        return;
    }

    Entity* entity = engine.GetSelectedEntity();

    if (!entity)
    {
        return;
    }

    const MeshData& mesh = entity->GetMeshData();

    if (!mesh.IsValid())
    {
        return;
    }

    Camera& camera = engine.GetCamera();

    const float aspectRatio =
        viewportSize.x /
        viewportSize.y;

    const glm::mat4 model = entity->GetModelMatrix();

    const glm::mat4 view = camera.GetViewMatrix();

    const glm::mat4 projection = camera.GetProjectionMatrix(aspectRatio);

    const glm::mat4 modelViewProjection = projection * view * model;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    constexpr float vertexRadius = 4.0f;

    for (std::size_t index = 0;
        index < mesh.vertices.size();
        ++index)
    {
        const MeshVertex& vertex = mesh.vertices[index];

        const glm::vec4 clipPosition =
            modelViewProjection *
            glm::vec4(
                vertex.position,
                1.0f
            );

        /*
         * Behind the camera or too close to
         * the projection plane.
         */
        if (clipPosition.w <= 0.0001f)
        {
            continue;
        }

        const glm::vec3 ndc =
            glm::vec3(clipPosition) /
            clipPosition.w;

        /*
         * Skip points outside the visible
         * clip-space range.
         */
        if (ndc.x < -1.0f ||
            ndc.x > 1.0f ||
            ndc.y < -1.0f ||
            ndc.y > 1.0f ||
            ndc.z < -1.0f ||
            ndc.z > 1.0f)
        {
            continue;
        }

        const float screenX =
            viewportPosition.x +
            ((ndc.x + 1.0f) * 0.5f) *
            viewportSize.x;

        /*
         * ImGui screen Y increases downward,
         * so the projected Y must be inverted.
         */
        const float screenY =
            viewportPosition.y +
            ((1.0f - ndc.y) * 0.5f) *
            viewportSize.y;

        const ImVec2 screenPosition(screenX, screenY);

        const bool selected = index == m_selectedVertex;

        const ImU32 fillColor = selected
            ? IM_COL32(255, 145, 35, 255)
            : IM_COL32(50, 170, 255, 255);

        drawList->AddCircleFilled(
            screenPosition,
            vertexRadius,
            fillColor
        );

        drawList->AddCircle(
            screenPosition,
            vertexRadius,
            IM_COL32(10, 10, 10, 255),
            0,
            1.0f
        );
    }

}



void VertexEditController::ClearSelection(BoxEngine& engine)
{
}

bool VertexEditController::EditPickVertex(BoxEngine& engine)
{
	return false;
}

void VertexEditController::EditCancelMove(Entity& entity)
{
}
