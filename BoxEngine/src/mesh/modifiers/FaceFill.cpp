#include "mesh/modifiers/FaceFill.h"
#include <mesh/MeshEditing.h>
#include <miniBoxLog.h>
#include <algorithm>
#include <cmath>
#include <vector>


namespace
{
    // ---------------------------------------------------------
    // Check whether a face contains the directed edge A -> B.
    // ---------------------------------------------------------

    bool FaceHasDirectedEdge(
        const EditFace& face,
        std::size_t vertexA,
        std::size_t vertexB)
    {
        if (face.vertices.size() < 2)
        {
            return false;
        }

        for (std::size_t i = 0;
            i < face.vertices.size();
            ++i)
        {
            const std::size_t a =
                face.vertices[i];

            const std::size_t b =
                face.vertices[
                    (i + 1) %
                        face.vertices.size()
                ];

            if (a == vertexA &&
                b == vertexB)
            {
                return true;
            }
        }

        return false;
    }


    // ---------------------------------------------------------
    // Work out whether the newly ordered face needs reversing.
    //
    // Adjacent faces should travel along their shared edge
    // in opposite directions.
    // ---------------------------------------------------------

    bool ShouldReverseFace(
        const MeshEditing& mesh,
        const std::vector<std::size_t>& vertices)
    {
        if (vertices.size() < 3)
        {
            return false;
        }

        for (std::size_t i = 0;
            i < vertices.size();
            ++i)
        {
            const std::size_t vertexA =
                vertices[i];

            const std::size_t vertexB =
                vertices[
                    (i + 1) %
                        vertices.size()
                ];

            for (std::size_t faceIndex = 0;
                faceIndex < mesh.GetFaceCount();
                ++faceIndex)
            {
                const EditFace& existingFace =
                    mesh.GetFace(faceIndex);

                // Existing face travels A -> B.
                //
                // Our new face is also travelling A -> B,
                // therefore the new face is backwards.
                if (FaceHasDirectedEdge(
                    existingFace,
                    vertexA,
                    vertexB))
                {
                    return true;
                }

                // Existing face travels B -> A.
                //
                // Our new face travels A -> B, which is
                // exactly what we want.
                if (FaceHasDirectedEdge(
                    existingFace,
                    vertexB,
                    vertexA))
                {
                    return false;
                }
            }
        }

        // No neighbouring face gave us a winding clue.
        return false;
    }
}


// =============================================================
// FACE FILL
// =============================================================

bool FaceFill::Use(
    MeshEditing& mesh,
    const std::vector<std::size_t>& selectedVertices)
{
    if (selectedVertices.size() < 3)
    {
        BOX_LOG_WARNING(
            "FaceFill: At least 3 vertices are required"
        );

        return false;
    }


    // ---------------------------------------------------------
    // Copy the selection.
    //
    // From this point on we do NOT care what order the user
    // selected the vertices in.
    // ---------------------------------------------------------

    std::vector<std::size_t> vertices =
        selectedVertices;


    // ---------------------------------------------------------
    // Remove accidental duplicate indices.
    // ---------------------------------------------------------

    std::sort(
        vertices.begin(),
        vertices.end()
    );

    vertices.erase(
        std::unique(
            vertices.begin(),
            vertices.end()
        ),
        vertices.end()
    );


    if (vertices.size() < 3)
    {
        BOX_LOG_WARNING(
            "FaceFill: Not enough unique vertices"
        );

        return false;
    }


    // ---------------------------------------------------------
    // Validate vertex indices.
    // ---------------------------------------------------------

    for (const std::size_t vertexIndex :
    vertices)
    {
        if (vertexIndex >=
            mesh.GetVertexCount())
        {
            BOX_LOG_ERROR(
                "FaceFill: Invalid vertex index"
            );

            return false;
        }
    }


    // ---------------------------------------------------------
    // Calculate centre of selected vertices.
    // ---------------------------------------------------------

    glm::vec3 centre(0.0f);

    for (const std::size_t vertexIndex :
    vertices)
    {
        centre +=
            mesh.GetVertex(
                vertexIndex
            ).position;
    }

    centre /=
        static_cast<float>(
            vertices.size()
            );


    // ---------------------------------------------------------
    // Find a useful first direction from the centre.
    //
    // Use the vertex furthest from the centre.
    // ---------------------------------------------------------

    glm::vec3 axisU(0.0f);

    float longestDistanceSquared =
        0.0f;


    for (const std::size_t vertexIndex :
    vertices)
    {
        const glm::vec3 direction =
            mesh.GetVertex(
                vertexIndex
            ).position -
            centre;

        const float distanceSquared =
            glm::dot(
                direction,
                direction
            );

        if (distanceSquared >
            longestDistanceSquared)
        {
            longestDistanceSquared =
                distanceSquared;

            axisU =
                direction;
        }
    }


    if (longestDistanceSquared <=
        0.000001f)
    {
        BOX_LOG_ERROR(
            "FaceFill: Selected vertices are degenerate"
        );

        return false;
    }


    axisU =
        glm::normalize(axisU);


    // ---------------------------------------------------------
    // Find the face plane normal.
    //
    // We find the selected vertex which produces the
    // strongest cross product against axisU.
    // ---------------------------------------------------------

    glm::vec3 faceNormal(0.0f);

    float strongestNormalSquared =
        0.0f;


    for (const std::size_t vertexIndex :
    vertices)
    {
        const glm::vec3 direction =
            mesh.GetVertex(
                vertexIndex
            ).position -
            centre;

        const glm::vec3 candidateNormal =
            glm::cross(
                axisU,
                direction
            );

        const float normalSquared =
            glm::dot(
                candidateNormal,
                candidateNormal
            );

        if (normalSquared >
            strongestNormalSquared)
        {
            strongestNormalSquared =
                normalSquared;

            faceNormal =
                candidateNormal;
        }
    }


    if (strongestNormalSquared <=
        0.000001f)
    {
        BOX_LOG_ERROR(
            "FaceFill: Vertices are collinear"
        );

        return false;
    }


    faceNormal =
        glm::normalize(
            faceNormal
        );


    // ---------------------------------------------------------
    // Build the second axis for our local 2D plane.
    // ---------------------------------------------------------

    glm::vec3 axisV =
        glm::normalize(
            glm::cross(
                faceNormal,
                axisU
            )
        );


    // ---------------------------------------------------------
    // Sort every vertex by its angle around the centre.
    //
    // THIS is what makes vertex selection order irrelevant.
    // ---------------------------------------------------------

    std::sort(
        vertices.begin(),
        vertices.end(),
        [&](std::size_t vertexA,
            std::size_t vertexB)
    {
        const glm::vec3 directionA =
            mesh.GetVertex(
                vertexA
            ).position -
            centre;

        const glm::vec3 directionB =
            mesh.GetVertex(
                vertexB
            ).position -
            centre;


        const float angleA =
            std::atan2(
                glm::dot(
                    directionA,
                    axisV
                ),
                glm::dot(
                    directionA,
                    axisU
                )
            );


        const float angleB =
            std::atan2(
                glm::dot(
                    directionB,
                    axisV
                ),
                glm::dot(
                    directionB,
                    axisU
                )
            );


        return angleA < angleB;
    }
    );


    // ---------------------------------------------------------
    // Correct the winding using surrounding topology.
    // ---------------------------------------------------------

    if (ShouldReverseFace(
        mesh,
        vertices))
    {
        std::reverse(
            vertices.begin(),
            vertices.end()
        );
    }


    // ---------------------------------------------------------
    // Create the face.
    // ---------------------------------------------------------

    mesh.AddFace(
        vertices
    );


    mesh.RebuildEdges();


    BOX_LOG_INFO(
        "FaceFill: Created ordered face with "
        << vertices.size()
        << " vertices"
    );


    return true;
}
