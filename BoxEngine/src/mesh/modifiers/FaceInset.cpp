#include "mesh/modifiers/FaceInset.h"
#include <entity/Entity.h>

#include <mesh/MeshEditing.h>
#include <miniBoxLog.h>

#include <glm/glm.hpp>

bool FaceInset::Use(MeshEditing& mesh, std::size_t faceIndex, float amount)
{
    if (faceIndex >= mesh.GetFaceCount())
    {
        BOX_LOG_ERROR(
            "FaceInset: Invalid face index"
        );

        return false;
    }

    amount = glm::clamp(amount, 0.0f, 0.95f);

    const EditFace originalFace =
        mesh.GetFace(faceIndex);

    if (originalFace.vertices.size() < 3)
    {
        BOX_LOG_ERROR(
            "FaceInset: Face has fewer than 3 vertices"
        );

        return false;
    }

    // ------------------------------------------
    // Calculate face centre
    // ------------------------------------------

    glm::vec3 centre(0.0f);

    for (const std::size_t vertexIndex :
    originalFace.vertices)
    {
        centre +=
            mesh.GetVertex(
                vertexIndex
            ).position;
    }

    centre /=
        static_cast<float>(
            originalFace.vertices.size()
            );

    // ------------------------------------------
    // Create new inset vertices
    // ------------------------------------------

    std::vector<std::size_t>
        insetVertices;

    insetVertices.reserve(
        originalFace.vertices.size()
    );

    for (const std::size_t oldIndex :
    originalFace.vertices)
    {
        const glm::vec3 oldPosition =
            mesh.GetVertex(
                oldIndex
            ).position;

        const glm::vec3 newPosition =
            glm::mix(
                oldPosition,
                centre,
                amount
            );

        const std::size_t newIndex =
            mesh.AddVertex(
                newPosition
            );

        insetVertices.push_back(
            newIndex
        );
    }

    // ------------------------------------------
    // Replace original face with
    // the new inset centre face
    // ------------------------------------------

    if (!mesh.SetFace(
        faceIndex,
        insetVertices))
    {
        BOX_LOG_ERROR(
            "FaceInset: Failed to replace face"
        );

        return false;
    }

    // ------------------------------------------
    // Create surrounding ring faces
    // ------------------------------------------

    const std::size_t vertexCount =
        originalFace.vertices.size();

    for (std::size_t i = 0;
        i < vertexCount;
        ++i)
    {
        const std::size_t next =
            (i + 1) %
            vertexCount;

        const std::size_t oldA =
            originalFace.vertices[i];

        const std::size_t oldB =
            originalFace.vertices[next];

        const std::size_t newA =
            insetVertices[i];

        const std::size_t newB =
            insetVertices[next];

        mesh.AddFace(
            {
                oldA,
                oldB,
                newB,
                newA
            }
        );
    }

    mesh.RebuildEdges();

    BOX_LOG_INFO(
        "Face inset. "
        << "Vertices="
        << mesh.GetVertexCount()
        << " Edges="
        << mesh.GetEdgeCount()
        << " Faces="
        << mesh.GetFaceCount()
    );

    return true;

}