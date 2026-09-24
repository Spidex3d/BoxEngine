#include "panels/UVPanel.h"
#include <imgui/imgui.h>
#include <BoxEngine.h>
#include <entity/Entity.h>
#include <mesh/UV/UVMapping.h>
#include <mesh/MeshEditing.h>
#include <mesh/MeshData.h>
#include <miniBoxLog.h>
//#include <cstdint>

UVPanel::~UVPanel() = default;

void UVPanel::Open()
{
    m_isOpen = true;
}

void UVPanel::Close()
{
    m_isOpen = false;
}

bool UVPanel::IsOpen() const
{
    return m_isOpen;
}

void UVPanel::Draw(BoxEngine& engine)
{
    if (!m_isOpen)
    {
        return;
    }

    // Draw the UV editor UI here
    ImGui::SetNextWindowSize(
        ImVec2(650.0f, 500.0f),
        ImGuiCond_FirstUseEver
    );

    if (!ImGui::Begin(
        "BoxEditor UV Panel",
        &m_isOpen))
    {
        ImGui::End();
        return;
    }

    Entity* selectedEntity =
        engine.GetSelectedEntity();

    if (!selectedEntity)
    {
        ImGui::TextDisabled(
            "No object selected."
        );

        ImGui::End();
        return;
    }

    ImGui::Text(
        "Object: %s",
        selectedEntity->GetName().c_str()
    );

    ImGui::Separator();

    const char* projectionTypes[] =
    {
        "Box",
        "Spherical"
    };

    ImGui::SetNextItemWidth(180.0f);

    ImGui::Combo(
        "Projection",
        &m_projectionType,
        projectionTypes,
        IM_ARRAYSIZE(projectionTypes)
    );

	ImGui::SeparatorText("UV Mapping");

    if (ImGui::Button(
        "Generate UV",
        ImVec2(140.0f, 32.0f)))
    {
        MeshEditing& editableMesh =
            selectedEntity->GetEditableMesh();

        bool generated = false;

        switch (m_projectionType)
        {
        case 0:
            generated =
                UVMapping::Box(editableMesh);
            break;

        case 1:
            generated =
                UVMapping::Spherical(editableMesh);
            break;
        }

        if (generated)
        {
            MeshData renderMesh;

            if (editableMesh.BuildRenderMesh(renderMesh))
            {
                selectedEntity->CreateFromMeshData(
                    renderMesh
                );

                BOX_LOG_INFO("UV coordinates generated for object: " + selectedEntity->GetName());
            }
        }
    }

    ImGui::SeparatorText("UV Layout");

    // -------------------------------------------------
    // UV canvas.
    //
    // Keep the canvas square and fit it into the
    // available panel space.
    // -------------------------------------------------

    const ImVec2 available =
        ImGui::GetContentRegionAvail();

    float canvasSize =
        available.x;

    if (available.y > 0.0f)
    {
        canvasSize =
            std::min(
                available.x,
                available.y
            );
    }

    // Don't allow the canvas to become too small.
    canvasSize =
        std::max(
            canvasSize,
            100.0f
        );

    const ImVec2 canvasPosition =
        ImGui::GetCursorScreenPos();

    const ImVec2 canvasDimensions(
        canvasSize,
        canvasSize
    );

    DrawUVLayout(
        *selectedEntity,
        canvasPosition,
        canvasDimensions
    );

    // Reserve the space in the ImGui window.
    ImGui::Dummy(
        canvasDimensions
    );
		
   


    ImGui::End();
}

void UVPanel::DrawUVLayout(Entity& entity, const ImVec2& canvasPosition, const ImVec2& canvasSize)
{
    ImDrawList* drawList =
        ImGui::GetWindowDrawList();

    if (!drawList)
    {
        return;
    }

    const ImVec2 canvasEnd(
        canvasPosition.x + canvasSize.x,
        canvasPosition.y + canvasSize.y
    );

    // -------------------------------------------------
    // Canvas background.
    // -------------------------------------------------

    drawList->AddRectFilled(
        canvasPosition,
        canvasEnd,
        IM_COL32(
            35,
            35,
            35,
            255
        )
    );

    // -------------------------------------------------
// Find the first material slot containing
// a base colour texture.
// -------------------------------------------------

    GLuint textureID = 0;

    for (std::size_t slot = 0;
        slot < entity.GetMaterialSlotCount();
        ++slot)
    {
        const Material& material =
            entity.GetMaterialSlot(slot);

        if (material.UsesBaseColorTexture() &&
            material.GetBaseColorTexture() != 0)
        {
            textureID =
                material.GetBaseColorTexture();

            break;
        }
    }

    if (textureID != 0)
    {
        drawList->AddImage(
            (ImTextureID)(
                static_cast<intptr_t>(
                    textureID
                    )
                ),
            canvasPosition,
            canvasEnd,
            ImVec2(0.0f, 0.0f),
            ImVec2(1.0f, 1.0f)
        );
    }


    // -------------------------------------------------
    // Simple UV grid.
    // 0.0 -> 1.0 divided into ten sections.
    // -------------------------------------------------

    constexpr int gridDivisions = 10;

    const ImU32 gridColor =
        IM_COL32(
            65,
            65,
            65,
            255
        );

    for (int i = 1;
        i < gridDivisions;
        ++i)
    {
        const float t =
            static_cast<float>(i) /
            static_cast<float>(gridDivisions);

        const float x =
            canvasPosition.x +
            canvasSize.x * t;

        const float y =
            canvasPosition.y +
            canvasSize.y * t;

        // Vertical.
        drawList->AddLine(
            ImVec2(
                x,
                canvasPosition.y
            ),
            ImVec2(
                x,
                canvasEnd.y
            ),
            gridColor
        );

        // Horizontal.
        drawList->AddLine(
            ImVec2(
                canvasPosition.x,
                y
            ),
            ImVec2(
                canvasEnd.x,
                y
            ),
            gridColor
        );
    }

    // -------------------------------------------------
    // 0-1 UV boundary.
    // -------------------------------------------------

    drawList->AddRect(
        canvasPosition,
        canvasEnd,
        IM_COL32(
            180,
            180,
            180,
            255
        ),
        0.0f,
        0,
        2.0f
    );

    // -------------------------------------------------
// Draw UV face layout.
// -------------------------------------------------

     MeshEditing& mesh =
        entity.GetEditableMesh();

     auto& faces = mesh.GetFaces();

    const ImU32 uvColor =
        IM_COL32(
            80,
            220,
            120,
            255
        );

    drawList->PushClipRect(canvasPosition, canvasEnd, true);

    for (const EditFace& face :
        faces)
    {
        // We need one UV for every face corner.
        if (face.uvs.size() !=
            face.vertices.size())
        {
            continue;
        }

        if (face.uvs.size() < 2)
        {
            continue;
        }

        for (std::size_t corner = 0;
            corner < face.uvs.size();
            ++corner)
        {
            const std::size_t next =
                (corner + 1) %
                face.uvs.size();

            const glm::vec2& uvA =
                face.uvs[corner];

            const glm::vec2& uvB =
                face.uvs[next];

            // -----------------------------------------
            // Convert UV coordinates into ImGui
            // screen coordinates.
            //
            // UV Y goes upwards.
            // ImGui Y goes downwards.
            // -----------------------------------------

            const ImVec2 pointA(
                canvasPosition.x +
                uvA.x * canvasSize.x,

                canvasPosition.y +
                (1.0f - uvA.y) *
                canvasSize.y
            );

            const ImVec2 pointB(
                canvasPosition.x +
                uvB.x * canvasSize.x,

                canvasPosition.y +
                (1.0f - uvB.y) *
                canvasSize.y
            );

            drawList->AddLine(
                pointA,
                pointB,
                uvColor,
                1.0f
            );
        }
    }

    // -------------------------------------------------
    // Draw and select UV vertices.
    // -------------------------------------------------

    const ImU32 uvVertexColor =
        IM_COL32(
            230,
            230,
            230,
            255
        );

    const ImU32 selectedUVColor =
        IM_COL32(
            255,
            150,
            40,
            255
        );

    constexpr float uvVertexRadius = 3.5f;
    constexpr float uvSelectRadius = 7.0f;

    const ImVec2 mousePosition =
        ImGui::GetIO().MousePos;

    for (std::size_t faceIndex = 0;
        faceIndex < faces.size();
        ++faceIndex)
    {
        const EditFace& face =
            faces[faceIndex];

        if (face.uvs.size() !=
            face.vertices.size())
        {
            continue;
        }

        for (std::size_t corner = 0;
            corner < face.uvs.size();
            ++corner)
        {
            const glm::vec2& uv =
                face.uvs[corner];

            const ImVec2 point(
                canvasPosition.x +
                uv.x * canvasSize.x,

                canvasPosition.y +
                (1.0f - uv.y) *
                canvasSize.y
            );

            // -----------------------------------------
            // Distance from mouse to this UV point.
            // -----------------------------------------

            const float dx =
                mousePosition.x - point.x;

            const float dy =
                mousePosition.y - point.y;

            const float distanceSquared =
                dx * dx + dy * dy;

            const bool hovered =
                distanceSquared <=
                uvSelectRadius * uvSelectRadius;

            // -----------------------------------------
            // Select this UV point.
            // -----------------------------------------
            if (hovered &&
                ImGui::IsMouseClicked(
                    ImGuiMouseButton_Left))
            {
                m_selectedFace =
                    static_cast<int>(faceIndex);

                m_selectedCorner =
                    static_cast<int>(corner);

                m_draggingUV = true;
            }
            
            // -----------------------------------------
            // Is this the selected UV?
            // -----------------------------------------

            const bool selected =
                m_selectedFace ==
                static_cast<int>(faceIndex) &&
                m_selectedCorner ==
                static_cast<int>(corner);

            drawList->AddCircleFilled(
                point,
                selected
                ? 5.0f
                : uvVertexRadius,
                selected
                ? selectedUVColor
                : uvVertexColor
            );
        }
        
    }
    if (ImGui::IsMouseReleased(
        ImGuiMouseButton_Left))
    {
        m_draggingUV = false;
    }

    // -------------------------------------------------
// Drag selected UV.
// -------------------------------------------------

    if (m_draggingUV &&
        m_selectedFace >= 0 &&
        m_selectedCorner >= 0)
    {
        const std::size_t faceIndex =
            static_cast<std::size_t>(
                m_selectedFace
                );

        const std::size_t cornerIndex =
            static_cast<std::size_t>(
                m_selectedCorner
                );

        if (faceIndex < faces.size())
        {
            //EditFace& face = faces[faceIndex];
            EditFace& face = mesh.GetFace(faceIndex);

            if (cornerIndex < face.uvs.size())
            {
                glm::vec2& uv =
                    face.uvs[cornerIndex];

                // Convert mouse screen position
                // back into UV coordinates.
                uv.x =
                    (mousePosition.x -
                        canvasPosition.x) /
                    canvasSize.x;

                uv.y =
                    1.0f -
                    ((mousePosition.y -
                        canvasPosition.y) /
                        canvasSize.y);
            }
        }
    }

    drawList->PopClipRect();

}
