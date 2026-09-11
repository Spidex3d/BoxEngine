#include <mesh\modifiers\RoundInset.h>
#include <entity/Entity.h>

#include <mesh/MeshEditing.h>
#include <miniBoxLog.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>

namespace
{
    glm::vec3 RotateAroundAxis(
        const glm::vec3& vector,
        const glm::vec3& axis,
        float angle)
    {
        const float c =
            std::cos(angle);

        const float s =
            std::sin(angle);

        return
            vector * c +
            glm::cross(axis, vector) * s +
            axis *
            glm::dot(axis, vector) *
            (1.0f - c);
    }
}


bool RoundInset::Use(MeshEditing& mesh, std::size_t faceIndex, float amount, int segments, float roundness)
{
    // =================================================
    // Validate
    // =================================================

    if (faceIndex >= mesh.GetFaceCount())
    {
        BOX_LOG_ERROR(
            "RoundInset: Invalid face index"
        );

        return false;
    }


    amount =
        glm::clamp(
            amount,
            0.0f,
            0.95f
        );


    segments =
        std::max(
            segments,
            1
        );


    const EditFace originalFace =
        mesh.GetFace(faceIndex);


    // Stage 1 / 2:
    // Round Inset currently supports quads only.
    if (originalFace.vertices.size() != 4)
    {
        BOX_LOG_ERROR(
            "RoundInset: Currently supports quad faces only"
        );

        return false;
    }


    // =================================================
    // Get original positions
    // =================================================

    glm::vec3 originalPositions[4];


    for (std::size_t i = 0;
        i < 4;
        ++i)
    {
        originalPositions[i] =
            mesh.GetVertex(
                originalFace.vertices[i]
            ).position;
    }


    // =================================================
    // Calculate face centre
    // =================================================

    glm::vec3 centre(0.0f);


    for (const glm::vec3& position :
        originalPositions)
    {
        centre += position;
    }


    centre /= 4.0f;


    // =================================================
    // Calculate face normal
    // =================================================

    const glm::vec3 edgeA =
        originalPositions[1] -
        originalPositions[0];

    const glm::vec3 edgeB =
        originalPositions[2] -
        originalPositions[0];


    glm::vec3 faceNormal =
        glm::cross(
            edgeA,
            edgeB
        );


    const float normalLength =
        glm::length(
            faceNormal
        );


    if (normalLength <= 0.000001f)
    {
        BOX_LOG_ERROR(
            "RoundInset: Invalid face normal"
        );

        return false;
    }


    faceNormal /=
        normalLength;


    // =================================================
    // Create the basic inset quad positions
    // =================================================

    glm::vec3 insetCorners[4];


    for (std::size_t i = 0;
        i < 4;
        ++i)
    {
        insetCorners[i] =
            glm::mix(
                originalPositions[i],
                centre,
                amount
            );
    }


    // =================================================
    // Store generated rounded corner vertices
    //
    // Each corner stores:
    //
    // start -> ...rounded points... -> end
    //
    // Example segments = 3:
    //
    // start - p1 - p2 - end
    // =================================================

    std::vector<std::vector<std::size_t>>
        cornerVertices(4);


    for (std::size_t corner = 0;
        corner < 4;
        ++corner)
    {
        const std::size_t previous =
            (corner + 3) % 4;

        const std::size_t next =
            (corner + 1) % 4;


        const glm::vec3 cornerPosition =
            insetCorners[corner];


        // ---------------------------------------------
        // Directions along the two adjoining edges
        // ---------------------------------------------

        glm::vec3 previousDirection =
            insetCorners[previous] -
            cornerPosition;

        glm::vec3 nextDirection =
            insetCorners[next] -
            cornerPosition;


        const float previousLength =
            glm::length(
                previousDirection
            );

        const float nextLength =
            glm::length(
                nextDirection
            );


        if (previousLength <= 0.000001f ||
            nextLength <= 0.000001f)
        {
            BOX_LOG_ERROR(
                "RoundInset: Invalid corner edge"
            );

            return false;
        }


        previousDirection /=
            previousLength;

        nextDirection /=
            nextLength;


        // ---------------------------------------------
        // Corner radius
        //
        // For now use 25% of the shortest edge.
        //
        // Later Roundness will control this.
        // ---------------------------------------------

        const float cornerRadius =
            std::min(
                previousLength,
                nextLength
            ) * 0.25f;


        // ---------------------------------------------
        // Find signed angle between both directions
        // around the face normal.
        // ---------------------------------------------
        const glm::vec3 startPoint =
            cornerPosition +
            previousDirection *
            cornerRadius;

        const glm::vec3 endPoint =
            cornerPosition +
            nextDirection *
            cornerRadius;


        // ---------------------------------------------
        // Arc centre
        //
        // Move away from the original corner along
        // both adjoining edges.
        //
        // For a square corner:
        //
        //      corner
        //         +
        //        / \
        //       S   E
        //        \ /
        //         C   <- arc centre
        //
        // The generated arc then curves outward toward
        // the original corner rather than inward.
        // ---------------------------------------------

        const glm::vec3 arcCentre =
            cornerPosition +
            previousDirection *
            cornerRadius +
            nextDirection *
            cornerRadius;


        // ---------------------------------------------
        // Directions from arc centre to tangent points
        // ---------------------------------------------

        glm::vec3 startDirection =
            startPoint -
            arcCentre;

        glm::vec3 endDirection =
            endPoint -
            arcCentre;


        const float startLength =
            glm::length(
                startDirection
            );

        const float endLength =
            glm::length(
                endDirection
            );


        if (startLength <= 0.000001f ||
            endLength <= 0.000001f)
        {
            return false;
        }


        startDirection /=
            startLength;

        endDirection /=
            endLength;


        // ---------------------------------------------
        // Signed angle around face normal
        // ---------------------------------------------

        const float dotValue =
            glm::clamp(
                glm::dot(
                    startDirection,
                    endDirection
                ),
                -1.0f,
                1.0f
            );


        const float crossValue =
            glm::dot(
                faceNormal,
                glm::cross(
                    startDirection,
                    endDirection
                )
            );


        const float angle =
            std::atan2(
                crossValue,
                dotValue
            );


        // ---------------------------------------------
        // Generate rounded corner
        // ---------------------------------------------

        cornerVertices[corner].reserve(
            static_cast<std::size_t>(
                segments + 1
                )
        );
        //const float dotValue =
        //    glm::clamp(
        //        glm::dot(
        //            previousDirection,
        //            nextDirection
        //        ),
        //        -1.0f,
        //        1.0f
        //    );


        //const float crossValue =
        //    glm::dot(
        //        faceNormal,
        //        glm::cross(
        //            previousDirection,
        //            nextDirection
        //        )
        //    );


        //const float angle =
        //    std::atan2(
        //        crossValue,
        //        dotValue
        //    );


        //// ---------------------------------------------
        //// Create points around this corner.
        ////
        //// segments 1 = 2 points
        //// segments 2 = 3 points
        //// segments 3 = 4 points
        //// etc.
        //// ---------------------------------------------

        //cornerVertices[corner].reserve(
        //    static_cast<std::size_t>(
        //        segments + 1
        //        )
        //);



        for (int segment = 0;
            segment <= segments;
            ++segment)
        {
            const float t =
                static_cast<float>(segment) /
                static_cast<float>(segments);


            const glm::vec3 direction =
                glm::normalize(
                    RotateAroundAxis(
                        startDirection,
                        faceNormal,
                        angle * t
                    )
                );


            const glm::vec3 newPosition =
                arcCentre +
                direction *
                cornerRadius;


            const std::size_t vertexIndex =
                mesh.AddVertex(
                    newPosition
                );


            cornerVertices[corner]
                .push_back(
                    vertexIndex
                );
        }


        /*for (int segment = 0;
            segment <= segments;
            ++segment)
        {
            const float t =
                static_cast<float>(segment) /
                static_cast<float>(segments);


            const float currentAngle =
                angle * t;


            glm::vec3 direction =
                RotateAroundAxis(
                    previousDirection,
                    faceNormal,
                    currentAngle
                );


            direction =
                glm::normalize(
                    direction
                );


            const glm::vec3 newPosition =
                cornerPosition +
                direction *
                cornerRadius;


            const std::size_t vertexIndex =
                mesh.AddVertex(
                    newPosition
                );


            cornerVertices[corner]
                .push_back(
                    vertexIndex
                );
        }*/
    }


    // =================================================
    // Build the new rounded centre face
    // =================================================

    std::vector<std::size_t>
        centreVertices;


    centreVertices.reserve(
        static_cast<std::size_t>(
            4 * (segments + 1)
            )
    );


    for (std::size_t corner = 0;
        corner < 4;
        ++corner)
    {
        for (const std::size_t vertexIndex :
        cornerVertices[corner])
        {
            centreVertices.push_back(
                vertexIndex
            );
        }
    }


    // Replace original face with the
    // new rounded centre polygon.

    EditFace& centreFace =
        mesh.GetFace(
            faceIndex
        );


    centreFace.vertices =
        centreVertices;


    centreFace.materialIndex =
        originalFace.materialIndex;


    // =================================================
    // Create four side strip faces
    //
    // Outer edge
    //
    // A ------------ B
    //    inner edge
    //      x ---- x
    // =================================================

    for (std::size_t side = 0;
        side < 4;
        ++side)
    {
        const std::size_t next =
            (side + 1) % 4;


        // Last point of this corner points
        // toward the next corner.
        const std::size_t innerA =
            cornerVertices[side].back();


        // First point of next corner points
        // back toward this corner.
        const std::size_t innerB =
            cornerVertices[next].front();


        const std::size_t newFaceIndex =
            mesh.AddFace(
                {
                    originalFace.vertices[side],
                    originalFace.vertices[next],
                    innerB,
                    innerA
                }
            );


        mesh.GetFace(
            newFaceIndex
        ).materialIndex =
            originalFace.materialIndex;
    }


    // =================================================
    // Fill the four corner regions
    //
    // Each corner becomes an n-gon:
    //
    // Outer vertex
    //      |
    // end--arc--start
    //
    // We reverse the arc because this face sits
    // outside the centre polygon.
    // =================================================

    for (std::size_t corner = 0;
        corner < 4;
        ++corner)
    {
        std::vector<std::size_t>
            cornerFace;


        cornerFace.reserve(
            static_cast<std::size_t>(
                segments + 2
                )
        );


        // Original outer corner.
        cornerFace.push_back(
            originalFace.vertices[corner]
        );


        // Add rounded arc backwards:
        //
        // end -> ... -> start
        //
        for (auto it =
            cornerVertices[corner].rbegin();

            it !=
            cornerVertices[corner].rend();

            ++it)
        {
            cornerFace.push_back(
                *it
            );
        }


        const std::size_t newFaceIndex =
            mesh.AddFace(
                cornerFace
            );


        mesh.GetFace(
            newFaceIndex
        ).materialIndex =
            originalFace.materialIndex;
    }


    // =================================================
    // Rebuild edge topology
    // =================================================

    mesh.RebuildEdges();


    BOX_LOG_INFO(
        "RoundInset: Amount="
        << amount
        << " Segments="
        << segments
    );


    return true;
}
