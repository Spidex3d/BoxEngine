#include "mesh/modifiers/FaceExtrude.h"
#include <entity/Entity.h>

#include <mesh/MeshEditing.h>
#include <miniBoxLog.h>

#include <glm/glm.hpp>

bool FaceExtrude::Use(Entity& entity, std::size_t faceIndex, float distance)
{
    // Implementation of face extrusion goes here
   
    MeshEditing& mesh =
        entity.GetEditableMesh();

    if (faceIndex >=
        mesh.GetFaceCount())
    {
        BOX_LOG_ERROR(
            "FaceExtrude: Invalid face index"
        );

        return false;
    }

    const EditFace originalFace =
        mesh.GetFace(faceIndex);

    if (originalFace.vertices.size() < 3)
    {
        BOX_LOG_ERROR(
            "FaceExtrude: Face has fewer than 3 vertices"
        );

        return false;
    }

    const EditVertex& vertexA =
        mesh.GetVertex(
            originalFace.vertices[0]
        );

    const EditVertex& vertexB =
        mesh.GetVertex(
            originalFace.vertices[1]
        );

    const EditVertex& vertexC =
        mesh.GetVertex(
            originalFace.vertices[2]
        );

    const glm::vec3 edgeAB =
        vertexB.position -
        vertexA.position;

    const glm::vec3 edgeAC =
        vertexC.position -
        vertexA.position;

    glm::vec3 normal =
        glm::cross(
            edgeAB,
            edgeAC
        );

    const float normalLength =
        glm::length(normal);

    if (normalLength <= 0.000001f)
    {
        BOX_LOG_ERROR(
            "FaceExtrude: Degenerate face"
        );

        return false;
    }

    normal /= normalLength;

    std::vector<std::size_t>
        newFaceVertices;

    newFaceVertices.reserve(
        originalFace.vertices.size()
    );

    /*
     * Duplicate each logical vertex and move
     * it along the face normal.
     */
    for (const std::size_t oldIndex :
    originalFace.vertices)
    {
        const glm::vec3 oldPosition =
            mesh.GetVertex(
                oldIndex
            ).position;

        const glm::vec3 newPosition =
            oldPosition +
            normal * distance;

        const std::size_t newIndex =
            mesh.AddVertex(
                newPosition
            );

        newFaceVertices.push_back(
            newIndex
        );
    }

    /*
     * Replace the original face with the new
     * outer face.
     */
    mesh.SetFace(
        faceIndex,
        newFaceVertices
    );

    /*
     * Create one side face for each original
     * face edge.
     */
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
            newFaceVertices[i];

        const std::size_t newB =
            newFaceVertices[next];

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

    MeshData renderMesh;

    if (!mesh.BuildRenderMesh(
        renderMesh))
    {
        BOX_LOG_ERROR(
            "FaceExtrude: Failed to rebuild render mesh"
        );

        return false;
    }

    if (!entity.CreateFromMeshData(
        renderMesh))
    {
        BOX_LOG_ERROR(
            "FaceExtrude: Failed to rebuild entity GPU mesh"
        );

        return false;
    }

    BOX_LOG_INFO(
        "Face extruded. "
        << "Vertices="
        << mesh.GetVertexCount()
        << " Edges="
        << mesh.GetEdgeCount()
        << " Faces="
        << mesh.GetFaceCount()
    );

    return true;
}