#include <tools/VertexDrawController.h>

#include <BoxEngine.h>
#include <entity/Entity.h>
#include <mesh/MeshEditing.h>
#include <camera/Camera.h>
#include <miniBoxLog.h>

#include <glm/gtc/matrix_inverse.hpp>

#include <cmath>


// =============================================================
// HANDLE INPUT
// =============================================================

void VertexDrawController::HandleInput(
    BoxEngine& engine,
    bool viewportHovered,
    bool vertexModeActive,
    const ImVec2& viewportPosition,
    const ImVec2& viewportSize)
{
    if (!vertexModeActive)
    {
        return;
    }

    if (!viewportHovered)
    {
        return;
    }

    Entity* entity = engine.GetSelectedEntity();

    if (!entity)
    {
        return;
    }


    // =========================================================
    // START DRAW MODE
    //
    // Select one vertex first, then press D.
    // =========================================================

    if (!m_isDrawing)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_D, false))
        {
            BeginDraw(
                *entity
            );
        }

        return;
    }


    // =========================================================
    // CHOOSE CONSTRUCTION PLANE
    // =========================================================

    if (m_plane ==
        VertexDrawPlane::None)
    {
        if (ImGui::IsKeyPressed(
            ImGuiKey_X,
            false))
        {
            m_plane =
                VertexDrawPlane::X;

            BOX_LOG_INFO(
                "Vertex Draw: X plane selected"
            );
        }
        else if (ImGui::IsKeyPressed(
            ImGuiKey_Y,
            false))
        {
            m_plane =
                VertexDrawPlane::Y;

            BOX_LOG_INFO(
                "Vertex Draw: Y plane selected"
            );
        }
        else if (ImGui::IsKeyPressed(
            ImGuiKey_Z,
            false))
        {
            m_plane =
                VertexDrawPlane::Z;

            BOX_LOG_INFO(
                "Vertex Draw: Z plane selected"
            );
        }

        return;
    }


    // =========================================================
    // FINISH
    // =========================================================

    if (ImGui::IsKeyPressed(
        ImGuiKey_Enter,
        false))
    {
        FinishDraw();

        return;
    }


    // =========================================================
    // PLACE VERTEX
    // =========================================================

    if (ImGui::IsMouseClicked(
        ImGuiMouseButton_Left))
    {
        CreateVertexFromMouse(
            engine,
            *entity,
            viewportPosition,
            viewportSize
        );
    }
}


// =============================================================
// BEGIN DRAW
// =============================================================

bool VertexDrawController::BeginDraw(
    Entity& entity)
{
    const std::vector<std::size_t>&
        selectedVertices =
        entity.GetSelectedVertices();


    if (selectedVertices.size() != 1)
    {
        BOX_LOG_WARNING(
            "Vertex Draw requires exactly one selected vertex"
        );

        return false;
    }


    MeshEditing& mesh =
        entity.GetEditableMesh();


    const std::size_t vertexIndex =
        selectedVertices[0];


    if (vertexIndex >=
        mesh.GetVertexCount())
    {
        BOX_LOG_ERROR(
            "Vertex Draw: Invalid starting vertex"
        );

        return false;
    }


    m_startVertex =
        vertexIndex;

    m_lastVertex =
        vertexIndex;


    m_planePoint =
        mesh.GetVertex(
            vertexIndex
        ).position;


    m_plane =
        VertexDrawPlane::None;

    m_isDrawing =
        true;


    BOX_LOG_INFO(
        "Vertex Draw started at vertex "
        << vertexIndex
        << ". Press X, Y or Z to choose plane."
    );


    return true;
}


// =============================================================
// CREATE VERTEX
// =============================================================

bool VertexDrawController::CreateVertexFromMouse(
    BoxEngine& engine,
    Entity& entity,
    const ImVec2& viewportPosition,
    const ImVec2& viewportSize)
{
    glm::vec3 newPosition(0.0f);


    if (!MouseToPlane(
        engine,
        entity,
        viewportPosition,
        viewportSize,
        newPosition))
    {
        return false;
    }


    MeshEditing& mesh =
        entity.GetEditableMesh();


    const std::size_t newVertex =
        mesh.AddVertex(
            newPosition
        );


    if (newVertex ==
        InvalidVertex)
    {
        BOX_LOG_ERROR(
            "Vertex Draw: Failed to create vertex"
        );

        return false;
    }


    if (m_lastVertex !=
        InvalidVertex)
    {
        const std::size_t edgeIndex =
            mesh.AddEdge(
                m_lastVertex,
                newVertex,
                true
            );


        if (edgeIndex ==
            InvalidVertex)
        {
            BOX_LOG_ERROR(
                "Vertex Draw: Failed to create edge"
            );

            return false;
        }
    }


    m_lastVertex =
        newVertex;


    // Keep only the newest vertex selected.
    entity.ClearSelectedVertices();

    entity.AddSelectedVertex(
        newVertex
    );


    BOX_LOG_INFO(
        "Vertex Draw: Added vertex "
        << newVertex
        << " at "
        << newPosition.x
        << ", "
        << newPosition.y
        << ", "
        << newPosition.z
    );


    return true;
}


// =============================================================
// MOUSE -> CONSTRUCTION PLANE
// =============================================================

bool VertexDrawController::MouseToPlane(
    BoxEngine& engine,
    Entity& entity,
    const ImVec2& viewportPosition,
    const ImVec2& viewportSize,
    glm::vec3& outPosition) const
{
    if (viewportSize.x <= 0.0f ||
        viewportSize.y <= 0.0f)
    {
        return false;
    }


    const ImVec2 mouse =
        ImGui::GetMousePos();


    // ---------------------------------------------------------
    // Mouse position inside viewport.
    // ---------------------------------------------------------

    const float mouseX =
        mouse.x -
        viewportPosition.x;

    const float mouseY =
        mouse.y -
        viewportPosition.y;


    // ---------------------------------------------------------
    // Screen -> NDC.
    //
    // ImGui Y goes downward, OpenGL Y goes upward.
    // ---------------------------------------------------------

    const float ndcX =
        (2.0f * mouseX) /
        viewportSize.x -
        1.0f;


    const float ndcY =
        1.0f -
        (2.0f * mouseY) /
        viewportSize.y;


    Camera& camera =
        engine.GetCamera();


    const float aspectRatio =
        viewportSize.x /
        viewportSize.y;


    const glm::mat4 view =
        camera.GetViewMatrix();


    const glm::mat4 projection =
        camera.GetProjectionMatrix(
            aspectRatio
        );


    // ---------------------------------------------------------
    // Unproject near and far points into world space.
    // ---------------------------------------------------------

    const glm::mat4 inverseViewProjection =
        glm::inverse(
            projection *
            view
        );


    glm::vec4 nearPoint =
        inverseViewProjection *
        glm::vec4(
            ndcX,
            ndcY,
            -1.0f,
            1.0f
        );


    glm::vec4 farPoint =
        inverseViewProjection *
        glm::vec4(
            ndcX,
            ndcY,
            1.0f,
            1.0f
        );


    if (std::abs(nearPoint.w) <=
        0.000001f ||
        std::abs(farPoint.w) <=
        0.000001f)
    {
        return false;
    }


    nearPoint /=
        nearPoint.w;

    farPoint /=
        farPoint.w;


    const glm::vec3 rayOriginWorld =
        glm::vec3(
            nearPoint
        );


    const glm::vec3 rayDirectionWorld =
        glm::normalize(
            glm::vec3(
                farPoint -
                nearPoint
            )
        );


    // ---------------------------------------------------------
    // Convert world ray into ENTITY LOCAL SPACE.
    //
    // MeshEditing stores local-space positions.
    // ---------------------------------------------------------

    const glm::mat4 inverseModel =
        glm::inverse(
            entity.GetModelMatrix()
        );


    const glm::vec3 rayOrigin =
        glm::vec3(
            inverseModel *
            glm::vec4(
                rayOriginWorld,
                1.0f
            )
        );


    const glm::vec3 rayDirection =
        glm::normalize(
            glm::mat3(
                inverseModel
            ) *
            rayDirectionWorld
        );


    // ---------------------------------------------------------
    // Build construction-plane normal.
    // ---------------------------------------------------------

    glm::vec3 planeNormal(0.0f);


    switch (m_plane)
    {
    case VertexDrawPlane::X:

        planeNormal =
            glm::vec3(
                1.0f,
                0.0f,
                0.0f
            );

        break;


    case VertexDrawPlane::Y:

        planeNormal =
            glm::vec3(
                0.0f,
                1.0f,
                0.0f
            );

        break;


    case VertexDrawPlane::Z:

        planeNormal =
            glm::vec3(
                0.0f,
                0.0f,
                1.0f
            );

        break;


    default:

        return false;
    }


    // ---------------------------------------------------------
    // Ray / plane intersection:
    //
    // t = dot(P - O, N) / dot(D, N)
    // ---------------------------------------------------------

    const float denominator =
        glm::dot(
            rayDirection,
            planeNormal
        );


    // Looking almost parallel to the plane.
    if (std::abs(denominator) <=
        0.000001f)
    {
        return false;
    }


    const float distance =
        glm::dot(
            m_planePoint -
            rayOrigin,
            planeNormal
        ) /
        denominator;


    outPosition =
        rayOrigin +
        rayDirection *
        distance;


    // ---------------------------------------------------------
    // Force the locked coordinate exactly.
    //
    // This prevents tiny floating-point drift.
    // ---------------------------------------------------------

    switch (m_plane)
    {
    case VertexDrawPlane::X:

        outPosition.x =
            m_planePoint.x;

        break;


    case VertexDrawPlane::Y:

        outPosition.y =
            m_planePoint.y;

        break;


    case VertexDrawPlane::Z:

        outPosition.z =
            m_planePoint.z;

        break;


    default:
        break;
    }


    return true;
}


// =============================================================
// FINISH
// =============================================================

void VertexDrawController::FinishDraw()
{
    BOX_LOG_INFO(
        "Vertex Draw finished"
    );


    m_isDrawing =
        false;

    m_plane =
        VertexDrawPlane::None;

    m_startVertex =
        InvalidVertex;

    m_lastVertex =
        InvalidVertex;

    m_planePoint =
        glm::vec3(0.0f);
}