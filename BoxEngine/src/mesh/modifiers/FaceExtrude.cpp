#include "mesh/modifiers/FaceExtrude.h"
#include <entity/Entity.h>

#include <mesh/MeshEditing.h>
#include <miniBoxLog.h>

#include <glm/glm.hpp>

//bool FaceExtrude::Use(Entity& entity, std::size_t faceIndex, float distance)
bool FaceExtrude::Use(MeshEditing& mesh, std::size_t faceIndex, const glm::vec3& direction, float distance)
{
    // Implementation of face extrusion goes here
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

    // ------------------------------------------
    // Validate direction
    // ------------------------------------------

    const float directionLength =
        glm::length(direction);

    if (directionLength <= 0.000001f)
    {
        BOX_LOG_ERROR(
            "FaceExtrude: Invalid extrusion direction"
        );

        return false;
    }

    const glm::vec3 extrusionDirection =
        glm::normalize(direction);

    // ------------------------------------------
    // Create duplicated outer face vertices
    // ------------------------------------------

    std::vector<std::size_t>
        newFaceVertices;

    newFaceVertices.reserve(
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
            oldPosition +
            extrusionDirection *
            distance;

        const std::size_t newIndex =
            mesh.AddVertex(
                newPosition
            );

        newFaceVertices.push_back(
            newIndex
        );
    }

    // ------------------------------------------
    // Replace original face with new outer face
    // ------------------------------------------

    if (!mesh.SetFace(
        faceIndex,
        newFaceVertices))
    {
        BOX_LOG_ERROR(
            "FaceExtrude: Failed to replace face"
        );

        return false;
    }

    // ------------------------------------------
    // Create side faces
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

    // ------------------------------------------
    // Rebuild modelling edges
    // ------------------------------------------

    mesh.RebuildEdges();

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