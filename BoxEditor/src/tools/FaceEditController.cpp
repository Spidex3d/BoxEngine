#include <tools\FaceEditController.h>
#include <BoxEngine.h> // Add this include to resolve incomplete type error
#include <entity/Entity.h>
#include <imgui/imgui.h>
#include <miniBoxLog.h>
#include <camera/Camera.h>
#include <limits>
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <mesh/modifiers/FaceExtrude.h>


void FaceEditController::HandleInput(
    BoxEngine& engine,
    bool viewportHovered,
    bool faceModeActive,
    const ImVec2& viewportPosition,
    const ImVec2& viewportSize)
{
    Entity* entity = engine.GetSelectedEntity();

    // -------------------------------------------------
    // Face mode is not active
    // -------------------------------------------------
    if (!faceModeActive)
    {
        if (m_isMoving && entity)
        {
            EditCancelMove(*entity);
        }

        return;
    }

    if (!viewportHovered)
    {
        return;
    }

    if (!entity)
    {
        return;
    }


    // =================================================
    // ACTIVE EXTRUSION
    // =================================================
    if (m_isExtruding)
    {
        // Choose extrusion axis.
        if (ImGui::IsKeyPressed(
            ImGuiKey_X,
            false))
        {
            m_extrudeAxis =
                ExtrudeAxis::X;

            m_extrudeStartMouse =
                ImGui::GetMousePos();

            BOX_LOG_INFO(
                "Extrude axis X"
            );
        }
        else if (ImGui::IsKeyPressed(
            ImGuiKey_Y,
            false))
        {
            m_extrudeAxis =
                ExtrudeAxis::Y;

            m_extrudeStartMouse =
                ImGui::GetMousePos();

            BOX_LOG_INFO(
                "Extrude axis Y"
            );
        }
        else if (ImGui::IsKeyPressed(
            ImGuiKey_Z,
            false))
        {
            m_extrudeAxis =
                ExtrudeAxis::Z;

            m_extrudeStartMouse =
                ImGui::GetMousePos();

            BOX_LOG_INFO(
                "Extrude axis Z"
            );
        }


        // ---------------------------------------------
        // Update extrusion after an axis is selected.
        // ---------------------------------------------
        if (m_extrudeAxis !=
            ExtrudeAxis::None)
        {
            const ImVec2 currentMouse =
                ImGui::GetMousePos();

            const float deltaX =
                currentMouse.x -
                m_extrudeStartMouse.x;

            const float deltaY =
                currentMouse.y -
                m_extrudeStartMouse.y;

            /*
             * X and Z use horizontal mouse movement.
             * Y uses vertical mouse movement.
             */
            switch (m_extrudeAxis)
            {
            case ExtrudeAxis::X:
                m_extrudeAmount =
                    deltaX *
                    m_moveSensitivity;
                break;

            case ExtrudeAxis::Y:
                m_extrudeAmount =
                    -deltaY *
                    m_moveSensitivity;
                break;

            case ExtrudeAxis::Z:
                m_extrudeAmount =
                    deltaX *
                    m_moveSensitivity;
                break;

            case ExtrudeAxis::None:
            default:
                break;
            }


            // -----------------------------------------
            // Restore the mesh to the exact state it
            // had before THIS extrusion.
            // -----------------------------------------
            MeshEditing& editableMesh =
                entity->GetEditableMesh();

            editableMesh =
                m_meshBeforeExtrude;


            // -----------------------------------------
            // Work out extrusion direction.
            // -----------------------------------------
            glm::vec3 direction(0.0f);

            switch (m_extrudeAxis)
            {
            case ExtrudeAxis::X:
                direction =
                    glm::vec3(
                        1.0f,
                        0.0f,
                        0.0f
                    );
                break;

            case ExtrudeAxis::Y:
                direction =
                    glm::vec3(
                        0.0f,
                        1.0f,
                        0.0f
                    );
                break;

            case ExtrudeAxis::Z:
                direction =
                    glm::vec3(
                        0.0f,
                        0.0f,
                        1.0f
                    );
                break;

            case ExtrudeAxis::None:
            default:
                break;
            }


            // -----------------------------------------
            // Apply THIS extrusion to the restored
            // pre-extrusion mesh.
            // -----------------------------------------
            FaceExtrude extrude;

            if (extrude.Use(
                editableMesh,
                m_extrudeFace,
                direction,
                m_extrudeAmount))
            {
                MeshData renderMesh;

                if (editableMesh.BuildRenderMesh(
                    renderMesh))
                {
                    entity->CreateFromMeshData(
                        renderMesh
                    );
                }
            }
        }


        // ---------------------------------------------
        // Confirm extrusion
        // ---------------------------------------------

        if (ImGui::IsMouseClicked(
            ImGuiMouseButton_Left))
        {
            ConfirmExtrude(*entity);

            return;
        }

        // ---------------------------------------------
        // Cancel extrusion
        // ---------------------------------------------
        
        if (ImGui::IsKeyPressed(
            ImGuiKey_Escape,
            false) ||
            ImGui::IsMouseClicked(
                ImGuiMouseButton_Right))
        {
            CancelExtrude(
                *entity
            );

            return;
        }

        return;
    }


    // =================================================
    // NORMAL FACE MOVE
    // =================================================

    if (!m_isMoving &&
        m_selectedFace != InvalidFace)
    {
        if (ImGui::IsKeyPressed(
            ImGuiKey_X,
            false))
        {
            EditBeginMove(
                *entity,
                FaceMoveAxis::X
            );
        }
        else if (ImGui::IsKeyPressed(
            ImGuiKey_Y,
            false))
        {
            EditBeginMove(
                *entity,
                FaceMoveAxis::Y
            );
        }
        else if (ImGui::IsKeyPressed(
            ImGuiKey_Z,
            false))
        {
            EditBeginMove(
                *entity,
                FaceMoveAxis::Z
            );
        }
    }


    // =================================================
    // UPDATE ACTIVE NORMAL FACE MOVE
    // =================================================

    if (m_isMoving)
    {
        EditUpdateMove(
            *entity
        );

        if (ImGui::IsMouseClicked(
            ImGuiMouseButton_Left))
        {
            EditConfirmMove();
        }
        else if (
            ImGui::IsKeyPressed(
                ImGuiKey_Escape,
                false) ||
            ImGui::IsMouseClicked(
                ImGuiMouseButton_Right))
        {
            EditCancelMove(
                *entity
            );
        }

        return;
    }


    // =================================================
    // NORMAL FACE PICKING
    // =================================================

    if (ImGui::IsMouseClicked(
        ImGuiMouseButton_Left))
    {
        PickFace(
            engine,
            viewportPosition,
            viewportSize
        );
    }
}


void FaceEditController::DrawFace(BoxEngine& engine, const ImVec2& viewportPosition,
    const ImVec2& viewportSize, bool faceModeActive)
{
    if (!faceModeActive ||
        viewportSize.x <= 0.0f ||
        viewportSize.y <= 0.0f)
    {
        return;
    }

    Entity* entity =
        engine.GetSelectedEntity();

    if (!entity)
    {
        return;
    }

    const std::vector<LogicalFace> faces =
        BuildLogicalFace(*entity);

    if (faces.empty())
    {
        return;
    }

    Camera& camera =
        engine.GetCamera();

    const float aspectRatio =
        viewportSize.x /
        viewportSize.y;

    const glm::mat4 modelViewProjection =
        camera.GetProjectionMatrix(
            aspectRatio
        ) *
        camera.GetViewMatrix() *
        entity->GetModelMatrix();

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    for (std::size_t index = 0;
        index < faces.size();
        ++index)
    {
        ImVec2 screenA;
        ImVec2 screenB;
        ImVec2 screenC;
        ImVec2 screenD;
        ImVec2 screenCentre;

        const bool cornersVisible =
            ProjectToScreen(
                faces[index].positionA,
                modelViewProjection,
                viewportPosition,
                viewportSize,
                screenA
            ) &&
            ProjectToScreen(
                faces[index].positionB,
                modelViewProjection,
                viewportPosition,
                viewportSize,
                screenB
            ) &&
            ProjectToScreen(
                faces[index].positionC,
                modelViewProjection,
                viewportPosition,
                viewportSize,
                screenC
            ) &&
            ProjectToScreen(
                faces[index].positionD,
                modelViewProjection,
                viewportPosition,
                viewportSize,
                screenD
            );

        const bool centreVisible =
            ProjectToScreen(
                faces[index].centre,
                modelViewProjection,
                viewportPosition,
                viewportSize,
                screenCentre
            );

        if (!cornersVisible ||
            !centreVisible)
        {
            continue;
        }

        //const bool selected = index == m_selectedFace;

        const bool selected = faces[index].editableFaceIndex == m_selectedFace;

        if (selected)
        {
            // Same triangulation used by BuildRenderMesh:
            // A-B-C
            // A-C-D

            drawList->AddTriangleFilled(
                screenA,
                screenB,
                screenC,
                IM_COL32(
                    30,
                    255,
                    70,
                    70
                )
            );

            drawList->AddTriangleFilled(
                screenA,
                screenC,
                screenD,
                IM_COL32(
                    30,
                    255,
                    70,
                    70
                )
            );

            // Draw the real outside boundary.
            const ImU32 outline =
                IM_COL32(
                    30,
                    255,
                    70,
                    255
                );

            constexpr float thickness =
                3.0f;

            drawList->AddLine(
                screenA,
                screenB,
                outline,
                thickness
            );

            drawList->AddLine(
                screenB,
                screenC,
                outline,
                thickness
            );

            drawList->AddLine(
                screenC,
                screenD,
                outline,
                thickness
            );

            drawList->AddLine(
                screenD,
                screenA,
                outline,
                thickness
            );
        }

       
    }
  
}

void FaceEditController::BeginExtrude(Entity& entity)
{
    if (m_selectedFace == InvalidFace)
    {
        return;
    }

    MeshEditing& mesh = entity.GetEditableMesh();

    if (m_selectedFace >= mesh.GetFaceCount())
    {
        return;
    }

    // Save the mesh BEFORE this extrusion.
    m_meshBeforeExtrude = mesh;

    m_extrudeFace = m_selectedFace;

    m_extrudeAxis = ExtrudeAxis::None;

    m_extrudeAmount = 0.0f;

    m_extrudeStartMouse = ImGui::GetMousePos();

    m_isExtruding = true;

    BOX_LOG_INFO("Started extrusion on face " << m_extrudeFace);
}

// ##########################################################################################################
// ############################################ Object Explorer #############################################
// ##########################################################################################################

void FaceEditController::SetExtrudeAmount(Entity& entity, float amount)
{
    if (!m_isExtruding)
    {
        return;
    }

    m_extrudeAmount =
        amount;

    UpdateExtrudeMesh(
        entity
    );
}

void FaceEditController::SetExtrudeAxis(Entity& entity, ExtrudeAxis axis)
{
    if (!m_isExtruding)
    {
        return;
    }

    m_extrudeAxis =
        axis;

    UpdateExtrudeMesh(
        entity
    );
}

void FaceEditController::ConfirmExtrude(Entity& entity)
{
    if (!m_isExtruding)
    {
        return;
    }

    ModifierAxis storedAxis =
        ModifierAxis::Y;

    switch (m_extrudeAxis)
    {
    case ExtrudeAxis::X:
        storedAxis =
            ModifierAxis::X;
        break;

    case ExtrudeAxis::Y:
        storedAxis =
            ModifierAxis::Y;
        break;

    case ExtrudeAxis::Z:
        storedAxis =
            ModifierAxis::Z;
        break;

    case ExtrudeAxis::None:
    default:
        break;
    }

    entity.SetLastExtrude(
        m_extrudeFace,
        storedAxis,
        m_extrudeAmount,
        m_meshBeforeExtrude
    );

    m_isExtruding =
        false;

    m_extrudeAxis =
        ExtrudeAxis::None;

    BOX_LOG_INFO(
        "Extrude confirmed. Amount="
        << m_extrudeAmount
    );

}

void FaceEditController::CancelExtrude(Entity& entity)
{
    if (!m_isExtruding)
    {
        return;
    }

    entity.GetEditableMesh() =
        m_meshBeforeExtrude;

    MeshData renderMesh;

    if (entity
        .GetEditableMesh()
        .BuildRenderMesh(renderMesh))
    {
        entity.CreateFromMeshData(
            renderMesh
        );
    }

    m_isExtruding = false;

    m_extrudeAxis =
        ExtrudeAxis::None;

    m_extrudeAmount =
        0.0f;

    BOX_LOG_INFO(
        "Extrude cancelled"
    );
}


void FaceEditController::UpdateExtrudeMesh(Entity& entity)
{
    if (!m_isExtruding ||
        m_extrudeAxis == ExtrudeAxis::None)
    {
        return;
    }

    // Always restore the mesh to the state
    // before this extrusion.
    MeshEditing& editableMesh =
        entity.GetEditableMesh();

    editableMesh =
        m_meshBeforeExtrude;


    glm::vec3 direction(0.0f);

    switch (m_extrudeAxis)
    {
    case ExtrudeAxis::X:
        direction =
            glm::vec3(
                1.0f,
                0.0f,
                0.0f
            );
        break;

    case ExtrudeAxis::Y:
        direction =
            glm::vec3(
                0.0f,
                1.0f,
                0.0f
            );
        break;

    case ExtrudeAxis::Z:
        direction =
            glm::vec3(
                0.0f,
                0.0f,
                1.0f
            );
        break;

    case ExtrudeAxis::None:
    default:
        return;
    }

    FaceExtrude extrude;

    if (!extrude.Use(
        editableMesh,
        m_extrudeFace,
        direction,
        m_extrudeAmount))
    {
        return;
    }

    MeshData renderMesh;

    if (!editableMesh.BuildRenderMesh(
        renderMesh))
    {
        return;
    }

    entity.CreateFromMeshData(
        renderMesh
    );
}
// #################################### End Object Explorer ###########################################


void FaceEditController::ClearSelection(BoxEngine& engine)
{
    m_selectedFace =
        InvalidFace;

    Entity* entity =
        engine.GetSelectedEntity();

    if (entity)
    {
        entity->ClearSelectedFace();
    }

}
struct TriangleData
{
    glm::vec3 a{ 0.0f };
    glm::vec3 b{ 0.0f };
    glm::vec3 c{ 0.0f };

    glm::vec3 normal{ 0.0f };
};

bool FaceEditController::PointInTriangle(const ImVec2& point, const ImVec2& a, const ImVec2& b, const ImVec2& c)
{
    const auto Sign =
        [](const ImVec2& p1,
            const ImVec2& p2,
            const ImVec2& p3)
    {
        return
            (p1.x - p3.x) *
            (p2.y - p3.y) -
            (p2.x - p3.x) *
            (p1.y - p3.y);
    };

    const float d1 =
        Sign(point, a, b);

    const float d2 =
        Sign(point, b, c);

    const float d3 =
        Sign(point, c, a);

    const bool hasNegative =
        d1 < 0.0f ||
        d2 < 0.0f ||
        d3 < 0.0f;

    const bool hasPositive =
        d1 > 0.0f ||
        d2 > 0.0f ||
        d3 > 0.0f;

    return !(hasNegative &&
        hasPositive);
}

std::vector<FaceEditController::LogicalFace>FaceEditController::BuildLogicalFace(const Entity& entity) const
{
    std::vector<LogicalFace> faces;

    const MeshEditing& editableMesh =
        entity.GetEditableMesh();

    const auto& editFaces =
        editableMesh.GetFaces();

    if (editFaces.empty())
    {
        return faces;
    }

    for (std::size_t faceIndex = 0;
        faceIndex < editFaces.size();
        ++faceIndex)
    {
        const EditFace& editFace =
            editFaces[faceIndex];

        // For now we are handling quad faces.
        if (editFace.vertices.size() != 4)
        {
            continue;
        }

        const std::size_t ia =
            editFace.vertices[0];

        const std::size_t ib =
            editFace.vertices[1];

        const std::size_t ic =
            editFace.vertices[2];

        const std::size_t id =
            editFace.vertices[3];

        if (ia >= editableMesh.GetVertexCount() ||
            ib >= editableMesh.GetVertexCount() ||
            ic >= editableMesh.GetVertexCount() ||
            id >= editableMesh.GetVertexCount())
        {
            continue;
        }

        LogicalFace face;

        // This is the REAL MeshEditing face index.
        face.editableFaceIndex =
            faceIndex;

        // Get the four logical corner positions.
        face.positionA =
            editableMesh.GetVertex(ia).position;

        face.positionB =
            editableMesh.GetVertex(ib).position;

        face.positionC =
            editableMesh.GetVertex(ic).position;

        face.positionD =
            editableMesh.GetVertex(id).position;


        // ==================================================
        // ADD THE CODE HERE
        // ==================================================

        const MeshData& renderMesh =
            entity.GetMeshData();

        constexpr float epsilon =
            0.0001f;

        for (std::size_t renderIndex = 0;
            renderIndex < renderMesh.vertices.size();
            ++renderIndex)
        {
            const glm::vec3& position =
                renderMesh.vertices[
                    renderIndex
                ].position;

            if (glm::length(
                position -
                face.positionA) <= epsilon)
            {
                face.verticesAtA.push_back(
                    renderIndex
                );
            }

            if (glm::length(
                position -
                face.positionB) <= epsilon)
            {
                face.verticesAtB.push_back(
                    renderIndex
                );
            }

            if (glm::length(
                position -
                face.positionC) <= epsilon)
            {
                face.verticesAtC.push_back(
                    renderIndex
                );
            }

            if (glm::length(
                position -
                face.positionD) <= epsilon)
            {
                face.verticesAtD.push_back(
                    renderIndex
                );
            }
        }

        // ==================================================
        // END OF NEW CODE
        // ==================================================


        // Calculate face centre.
        face.centre =
            (
                face.positionA +
                face.positionB +
                face.positionC +
                face.positionD
                ) * 0.25f;


        // Calculate face normal.
        const glm::vec3 edgeAB =
            face.positionB -
            face.positionA;

        const glm::vec3 edgeAC =
            face.positionC -
            face.positionA;

        const glm::vec3 crossProduct =
            glm::cross(
                edgeAB,
                edgeAC
            );

        const float length =
            glm::length(
                crossProduct
            );

        if (length <= 0.000001f)
        {
            continue;
        }

        face.normal =
            crossProduct / length;


        // Completed face.
        faces.push_back(
            std::move(face)
        );
    }

    return faces;
    

    
}

bool FaceEditController::ProjectToScreen(const glm::vec3& localPosition, const glm::mat4& modelViewProjection,
    const ImVec2& viewportPosition, const ImVec2& viewportSize, ImVec2& outScreenPosition) const
{
    const glm::vec4 clipPosition =
        modelViewProjection *
        glm::vec4(
            localPosition,
            1.0f
        );

    if (clipPosition.w <= 0.0001f)
    {
        return false;
    }

    const glm::vec3 ndc =
        glm::vec3(clipPosition) /
        clipPosition.w;

    if (ndc.x < -1.0f ||
        ndc.x > 1.0f ||
        ndc.y < -1.0f ||
        ndc.y > 1.0f ||
        ndc.z < -1.0f ||
        ndc.z > 1.0f)
    {
        return false;
    }

    outScreenPosition.x =
        viewportPosition.x +
        ((ndc.x + 1.0f) * 0.5f) *
        viewportSize.x;

    outScreenPosition.y =
        viewportPosition.y +
        ((1.0f - ndc.y) * 0.5f) *
        viewportSize.y;

    return true;
}
// pick a face in the viewport based on the mouse position and return true if a face was selected, false otherwise
bool FaceEditController::PickFace(BoxEngine& engine, const ImVec2& viewportPosition, const ImVec2& viewportSize)
{
    Entity* entity =
        engine.GetSelectedEntity();

    if (!entity)
    {
        return false;
    }

    const std::vector<LogicalFace> faces =
        BuildLogicalFace(*entity);

    if (faces.empty())
    {
        return false;
    }

    Camera& camera =
        engine.GetCamera();

    const float aspectRatio =
        viewportSize.x /
        viewportSize.y;

    const glm::mat4 modelViewProjection =
        camera.GetProjectionMatrix(
            aspectRatio
        ) *
        camera.GetViewMatrix() *
        entity->GetModelMatrix();

    const ImVec2 mousePosition =
        ImGui::GetMousePos();

    std::size_t closestFace =
        InvalidFace;

    float closestDepth =
        std::numeric_limits<float>::max();

    for (std::size_t index = 0;
        index < faces.size();
        ++index)
    {
        ImVec2 screenA;
        ImVec2 screenB;
        ImVec2 screenC;
        ImVec2 screenD;

        if (!ProjectToScreen(
            faces[index].positionA,
            modelViewProjection,
            viewportPosition,
            viewportSize,
            screenA) ||
            !ProjectToScreen(
                faces[index].positionB,
                modelViewProjection,
                viewportPosition,
                viewportSize,
                screenB) ||
            !ProjectToScreen(
                faces[index].positionC,
                modelViewProjection,
                viewportPosition,
                viewportSize,
                screenC) ||
            !ProjectToScreen(
                faces[index].positionD,
                modelViewProjection,
                viewportPosition,
                viewportSize,
                screenD))
        {
            continue;
        }

        const bool inside =
            PointInTriangle(
                mousePosition,
                screenA,
                screenB,
                screenC
            ) ||
            PointInTriangle(
                mousePosition,
                screenA,
                screenC,
                screenD
            );

        if (!inside)
        {
            continue;
        }

        const glm::vec4 clipCentre =
            modelViewProjection *
            glm::vec4(
                faces[index].centre,
                1.0f
            );

        if (clipCentre.w <= 0.0001f)
        {
            continue;
        }

        const float depth =
            clipCentre.z /
            clipCentre.w;

        if (depth < closestDepth)
        {
            closestDepth = depth;
            closestFace = index;
        }
    }

    if (closestFace == InvalidFace)
    {
        ClearSelection(engine);
        return false;
    }

    m_selectedFace = faces[closestFace].editableFaceIndex;

    /*const LogicalFace& selected = faces[closestFace];

    BOX_LOG_INFO(
        "Selected editable face: "
        << selected.editableFaceIndex
        << " A=("
        << selected.positionA.x << ", "
        << selected.positionA.y << ", "
        << selected.positionA.z << ")"
        << " B=("
        << selected.positionB.x << ", "
        << selected.positionB.y << ", "
        << selected.positionB.z << ")"
        << " C=("
        << selected.positionC.x << ", "
        << selected.positionC.y << ", "
        << selected.positionC.z << ")"
        << " D=("
        << selected.positionD.x << ", "
        << selected.positionD.y << ", "
        << selected.positionD.z << ")" );*/

    BOX_LOG_INFO(
        "Selected face index: "
        << m_selectedFace
    );

    return true;

}

void FaceEditController::EditBeginMove(Entity& entity, FaceMoveAxis axis)
{

    if (m_selectedFace == InvalidFace)
    {
        return;
    }

    const std::vector<LogicalFace> faces =
        BuildLogicalFace(entity);

    /*
     * m_selectedFace is now the real
     * MeshEditing face index.
     *
     * Find the LogicalFace that represents
     * that editable face.
     */
    const LogicalFace* selectedFace =
        nullptr;

    for (const LogicalFace& face : faces)
    {
        if (face.editableFaceIndex ==
            m_selectedFace)
        {
            selectedFace =
                &face;

            break;
        }
    }

    if (!selectedFace)
    {
        BOX_LOG_WARNING(
            "EditBeginMove: selected editable face "
            << m_selectedFace
            << " was not found"
        );

        return;
    }

    MeshEditing& editableMesh =
        entity.GetEditableMesh();

    if (m_selectedFace >=
        editableMesh.GetFaceCount())
    {
        return;
    }

    const EditFace& editFace =
        editableMesh.GetFace(
            m_selectedFace
        );

    m_editableFaceVertices =
        editFace.vertices;

    m_editableStartPositions.clear();

    for (const std::size_t vertexIndex :
    m_editableFaceVertices)
    {
        m_editableStartPositions.push_back(
            editableMesh
            .GetVertex(vertexIndex)
            .position
        );
    }

    /*
     * We can still keep the render-index data
     * temporarily while converting the system.
     */
    const MeshData& mesh =
        entity.GetMeshData();

    m_startVertexPositions.clear();

    const auto AddStartVertex =
        [&](std::size_t index)
    {
        if (index >= mesh.vertices.size())
        {
            return;
        }

        for (const FaceStartPosition& existing :
            m_startVertexPositions)
        {
            if (existing.index == index)
            {
                return;
            }
        }

        FaceStartPosition start;

        start.index = index;
        start.position =
            mesh.vertices[index].position;

        m_startVertexPositions.push_back(
            start
        );
    };

    for (const std::size_t index :
    selectedFace->verticesAtA)
    {
        AddStartVertex(index);
    }

    for (const std::size_t index :
    selectedFace->verticesAtB)
    {
        AddStartVertex(index);
    }

    for (const std::size_t index :
    selectedFace->verticesAtC)
    {
        AddStartVertex(index);
    }

    for (const std::size_t index :
    selectedFace->verticesAtD)
    {
        AddStartVertex(index);
    }

    m_moveAxis = axis;

    m_startMouse = ImGui::GetMousePos();

    m_isMoving = !m_editableFaceVertices.empty();

}

void FaceEditController::EditUpdateMove(Entity& entity)
{
    if (!m_isMoving)
    {
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

    glm::vec3 movement(0.0f);

    switch (m_moveAxis)
    {
    case FaceMoveAxis::X:
        movement.x =
            deltaX *
            m_moveSensitivity;
        break;

    case FaceMoveAxis::Y:
        movement.y =
            -deltaY *
            m_moveSensitivity;
        break;

    case FaceMoveAxis::Z:
        movement.z =
            deltaX *
            m_moveSensitivity;
        break;

    case FaceMoveAxis::None:
    default:
        return;
    }


    MeshEditing& editableMesh = entity.GetEditableMesh();

    for (std::size_t i = 0;
        i < m_editableFaceVertices.size();
        ++i)
    {
        const std::size_t vertexIndex =
            m_editableFaceVertices[i];

        editableMesh
            .GetVertex(vertexIndex)
            .position =
            m_editableStartPositions[i] +
            movement;
    }

    /*
     * MeshEditing is now the source of truth.
     * Regenerate the render mesh from it.
     */
    MeshData renderMesh;

    if (!editableMesh.BuildRenderMesh(
        renderMesh))
    {
        BOX_LOG_ERROR("Face move: failed to rebuild render mesh");

        return;
    }

    if (!entity.CreateFromMeshData(
        renderMesh))
    {
        BOX_LOG_ERROR("Face move: failed to update GPU mesh");

        return;
    }

}
// confirm the move operation and finalize the face's new position
void FaceEditController::EditConfirmMove()
{

    m_isMoving = false;

    m_moveAxis =
        FaceMoveAxis::None;

    m_startVertexPositions.clear();

    m_editableFaceVertices.clear();
    m_editableStartPositions.clear();

}
// cancel the move operation and revert the face to its original position
void FaceEditController::EditCancelMove(Entity& entity)
{

    MeshEditing& editableMesh =
        entity.GetEditableMesh();

    for (std::size_t i = 0;
        i < m_editableFaceVertices.size();
        ++i)
    {
        editableMesh
            .GetVertex(
                m_editableFaceVertices[i]
            )
            .position =
            m_editableStartPositions[i];
    }

    MeshData renderMesh;

    if (editableMesh.BuildRenderMesh(
        renderMesh))
    {
        entity.CreateFromMeshData(
            renderMesh
        );
    }

    m_isMoving = false;

    m_moveAxis =
        FaceMoveAxis::None;

    m_startVertexPositions.clear();
    m_editableFaceVertices.clear();
    m_editableStartPositions.clear();

}
