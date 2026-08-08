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


void FaceEditController::HandleInput(BoxEngine& engine, bool viewportHovered, bool faceModeActive,
    const ImVec2& viewportPosition, const ImVec2& viewportSize)
{

    Entity* entity = engine.GetSelectedEntity();


    // ####################### Temporary Face Extrude test
    /*if (!m_isMoving &&
        m_selectedFace != InvalidFace &&
        ImGui::IsKeyPressed(ImGuiKey_E, false))
    {
        BOX_LOG_INFO(
            "E pressed for Face Extrude. Face="
            << m_selectedFace
        );

        FaceExtrude extrude;

        if (extrude.Use(
            *entity,
            m_selectedFace,
            0.5f))
        {
            BOX_LOG_INFO("Face extrusion successful");
        }
        else
        {
            BOX_LOG_ERROR(
                "Face extrusion failed"
            );
        }

        return;
    }*/

    // ##################################################



    if (!faceModeActive)
    {
        if (m_isMoving)
        {
            Entity* entity =
                engine.GetSelectedEntity();

            if (entity)
            {
                EditCancelMove(*entity);
            }
        }

        return;
    }

    if (!viewportHovered)
    {
        return;
    }

  
    
    // unresolved external symbol "public: struct EditFace & __cdecl MeshEditing::GetFace(unsigned __int64)" (? GetFace@MeshEditing@@QEAAAEAUEditFace@@_K@Z) referenced in function "public: bool __cdecl FaceExtrude::Use(class Entity &,unsigned __int64,float)" (? Use@FaceExtrude@@QEAA_NAEAVEntity@@_KM@Z)


    if (!entity)
    {
        return;
    }

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

    if (m_isMoving)
    {
        EditUpdateMove(*entity);

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
            EditCancelMove(*entity);
        }

        return;
    }

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

        const bool selected =
            index == m_selectedFace;

        if (selected)
        {
            drawList->AddQuadFilled(
                screenA,
                screenB,
                screenC,
                screenD,
                IM_COL32(
                    30,
                    255,
                    70,
                    70
                )
            );

            drawList->AddQuad(
                screenA,
                screenB,
                screenC,
                screenD,
                IM_COL32(
                    30,
                    255,
                    70,
                    255
                ),
                3.0f
            );
        }

        /*const ImU32 color =
            selected
            ? IM_COL32(30, 255, 70, 255)
            : IM_COL32(255, 165, 40, 255);*/

        /*const float radius =
            selected ? 8.0f : 6.0f;

        drawList->AddCircleFilled(
            screenCentre,
            radius,
            color
        );

        drawList->AddCircle(
            screenCentre,
            radius,
            IM_COL32(20, 20, 20, 255),
            0,
            1.0f
        );*/
    }
  
}

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
    std::vector<LogicalFace> logicalFaces;

    const MeshData& mesh = entity.GetMeshData();

    if (!mesh.IsValid())
    {
        return logicalFaces;
    }

    constexpr float epsilon = 0.0001f;

    constexpr float coplanarThreshold = 0.999f;

    const auto SamePosition =
        [](const glm::vec3& a,
            const glm::vec3& b)
    {
        constexpr float positionEpsilon = 0.0001f;

        return glm::length(a - b) <= positionEpsilon;
    };

    std::vector<TriangleData> triangles;

    const auto AddTriangle =
        [&](const glm::vec3& a,
            const glm::vec3& b,
            const glm::vec3& c)
    {
        const glm::vec3 crossProduct =
            glm::cross(
                b - a,
                c - a
            );

        const float crossLength = glm::length(crossProduct);

        if (crossLength <= epsilon)
        {
            return;
        }

        TriangleData triangle;

        triangle.a = a;
        triangle.b = b;
        triangle.c = c;

        triangle.normal =
            crossProduct /
            crossLength;

        triangles.push_back(triangle);
    };

    if (mesh.HasIndices())
    {
        for (std::size_t index = 0;
            index + 2 < mesh.indices.size();
            index += 3)
        {
            const std::uint32_t ia =
                mesh.indices[index];

            const std::uint32_t ib =
                mesh.indices[index + 1];

            const std::uint32_t ic =
                mesh.indices[index + 2];

            if (ia >= mesh.vertices.size() ||
                ib >= mesh.vertices.size() ||
                ic >= mesh.vertices.size())
            {
                continue;
            }

            AddTriangle(
                mesh.vertices[ia].position,
                mesh.vertices[ib].position,
                mesh.vertices[ic].position
            );
        }
    }
    else
    {
        for (std::size_t index = 0;
            index + 2 < mesh.vertices.size();
            index += 3)
        {
            AddTriangle(
                mesh.vertices[index].position,
                mesh.vertices[index + 1].position,
                mesh.vertices[index + 2].position
            );
        }
    }

    std::vector<bool> triangleUsed(triangles.size(), false);

    for (std::size_t first = 0;
        first < triangles.size();
        ++first)
    {
        if (triangleUsed[first])
        {
            continue;
        }

        const TriangleData& triangleA = triangles[first];

        for (std::size_t second = first + 1;
            second < triangles.size();
            ++second)
        {
            if (triangleUsed[second])
            {
                continue;
            }

            const TriangleData& triangleB = triangles[second];

            const float normalAgreement =
                glm::dot(
                    triangleA.normal,
                    triangleB.normal
                );

            if (normalAgreement <
                coplanarThreshold)
            {
                continue;
            }

            std::vector<glm::vec3> uniquePositions;

            const auto AddUniquePosition =
                [&](const glm::vec3& position)
            {
                for (const glm::vec3& existing :
                    uniquePositions)
                {
                    if (SamePosition(
                        existing,
                        position))
                    {
                        return;
                    }
                }

                uniquePositions.push_back(position);
            };

            AddUniquePosition(triangleA.a);
            AddUniquePosition(triangleA.b);
            AddUniquePosition(triangleA.c);

            AddUniquePosition(triangleB.a);
            AddUniquePosition(triangleB.b);
            AddUniquePosition(triangleB.c);

            /*
             * Two triangles forming a quad
             * have exactly four unique corners.
             */
            if (uniquePositions.size() != 4)
            {
                continue;
            }

            /*
             * Ensure both triangles are on the
             * same plane, not merely parallel.
             */
            const float planeDistance =
                glm::dot(
                    triangleA.normal,
                    triangleB.a -
                    triangleA.a
                );

            if (std::abs(planeDistance) >
                epsilon)
            {
                continue;
            }

            LogicalFace face;

            face.positionA =
                uniquePositions[0];

            face.positionB =
                uniquePositions[1];

            face.positionC =
                uniquePositions[2];

            face.positionD =
                uniquePositions[3];

            face.normal =
                triangleA.normal;

            face.centre =
                (
                    face.positionA +
                    face.positionB +
                    face.positionC +
                    face.positionD
                    ) * 0.25f;

            /*
             * Store all duplicated rendering
             * vertices for each logical corner.
             */
            for (std::size_t vertexIndex = 0;
                vertexIndex < mesh.vertices.size();
                ++vertexIndex)
            {
                const glm::vec3& position =
                    mesh.vertices[
                        vertexIndex
                    ].position;

                if (SamePosition(
                    position,
                    face.positionA))
                {
                    face.verticesAtA.push_back(
                        vertexIndex
                    );
                }

                if (SamePosition(
                    position,
                    face.positionB))
                {
                    face.verticesAtB.push_back(
                        vertexIndex
                    );
                }

                if (SamePosition(
                    position,
                    face.positionC))
                {
                    face.verticesAtC.push_back(
                        vertexIndex
                    );
                }

                if (SamePosition(
                    position,
                    face.positionD))
                {
                    face.verticesAtD.push_back(
                        vertexIndex
                    );
                }
            }

            logicalFaces.push_back(
                std::move(face)
            );

            triangleUsed[first] = true;
            triangleUsed[second] = true;

            break;
        }
    }

    return logicalFaces;
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

    m_selectedFace =
        closestFace;

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

    if (m_selectedFace >= faces.size())
    {
        m_selectedFace = InvalidFace;
        return;
    }

    const LogicalFace& selectedFace =
        faces[m_selectedFace];

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
    selectedFace.verticesAtA)
    {
        AddStartVertex(index);
    }

    for (const std::size_t index :
    selectedFace.verticesAtB)
    {
        AddStartVertex(index);
    }

    for (const std::size_t index :
    selectedFace.verticesAtC)
    {
        AddStartVertex(index);
    }

    for (const std::size_t index :
    selectedFace.verticesAtD)
    {
        AddStartVertex(index);
    }

    if (m_startVertexPositions.empty())
    {
        return;
    }

    m_moveAxis = axis;

    m_startMouse =
        ImGui::GetMousePos();

    m_isMoving = true;
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

    for (const FaceStartPosition& start :
        m_startVertexPositions)
    {
        entity.SetVertexPosition(
            start.index,
            start.position + movement
        );
    }

    entity.RecalculateNormals();

    entity.UploadMeshData();

}
// confirm the move operation and finalize the face's new position
void FaceEditController::EditConfirmMove()
{
    m_isMoving = false;

    m_moveAxis =
        FaceMoveAxis::None;

    m_startVertexPositions.clear();

}
// cancel the move operation and revert the face to its original position
void FaceEditController::EditCancelMove(Entity& entity)
{
    for (const FaceStartPosition& start :
        m_startVertexPositions)
    {
        entity.SetVertexPosition(
            start.index,
            start.position
        );
    }

    entity.RecalculateNormals();

    entity.UploadMeshData();

    m_isMoving = false;

    m_moveAxis =
        FaceMoveAxis::None;

    m_startVertexPositions.clear();

}
