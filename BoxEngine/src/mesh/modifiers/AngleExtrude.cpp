#include <mesh/modifiers/AngleExtrude.h>
#include <mesh/MeshEditing.h>
#include <miniBoxLog.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <vector>

bool AngleExtrude::Use(MeshEditing& mesh, std::size_t faceIndex, const AngleExtrudeSettings& settings)
{
    // ---------------------------------------------------------
        // Validate face
        // ---------------------------------------------------------

    if (faceIndex >=
        mesh.GetFaceCount())
    {
        BOX_LOG_ERROR(
            "AngleExtrude: Invalid face index"
        );

        return false;
    }


    const EditFace originalFace =
        mesh.GetFace(faceIndex);


    if (originalFace.vertices.size() < 3)
    {
        BOX_LOG_ERROR(
            "AngleExtrude: Face has fewer than 3 vertices"
        );

        return false;
    }


    // ---------------------------------------------------------
    // Validate settings
    // ---------------------------------------------------------

    if (settings.segments < 1)
    {
        BOX_LOG_ERROR(
            "AngleExtrude: Segments must be at least 1"
        );

        return false;
    }


    if (std::abs(settings.distance) <=
        0.000001f)
    {
        BOX_LOG_ERROR(
            "AngleExtrude: Distance is too small"
        );

        return false;
    }


    if (std::abs(settings.angleDegrees) <=
        0.000001f)
    {
        BOX_LOG_ERROR(
            "AngleExtrude: Angle is too small"
        );

        return false;
    }


    const float extrusionLength =
        glm::length(
            settings.extrusionAxis
        );


    const float rotationLength =
        glm::length(
            settings.rotationAxis
        );


    if (extrusionLength <= 0.000001f ||
        rotationLength <= 0.000001f)
    {
        BOX_LOG_ERROR(
            "AngleExtrude: Invalid axis"
        );

        return false;
    }


    const glm::vec3 extrusionAxis =
        glm::normalize(
            settings.extrusionAxis
        );


    const glm::vec3 rotationAxis =
        glm::normalize(
            settings.rotationAxis
        );


    // ---------------------------------------------------------
    // Extrusion axis and rotation axis must not be parallel.
    //
    // Example:
    //
    // Extrude X
    // Rotate Z
    //
    // is valid.
    //
    // Extrude X
    // Rotate X
    //
    // cannot produce a curved path.
    // ---------------------------------------------------------

    glm::vec3 bendDirection =
        glm::cross(
            rotationAxis,
            extrusionAxis
        );


    const float bendDirectionLength =
        glm::length(
            bendDirection
        );


    if (bendDirectionLength <=
        0.000001f)
    {
        BOX_LOG_ERROR(
            "AngleExtrude: Extrusion axis and rotation axis are parallel"
        );

        return false;
    }


    bendDirection =
        glm::normalize(
            bendDirection
        );


    // ---------------------------------------------------------
    // Negative angle bends in the opposite direction.
    // ---------------------------------------------------------

    const float angleSign =
        settings.angleDegrees < 0.0f
        ? -1.0f
        : 1.0f;


    bendDirection *=
        angleSign;


    const float totalAngle =
        glm::radians(
            std::abs(
                settings.angleDegrees
            )
        );


    const float signedTotalAngle =
        glm::radians(
            settings.angleDegrees
        );


    /*const float radius =
        std::abs(
            settings.distance
        );*/


    // ---------------------------------------------------------
    // Find centre of original face.
    //
    // All rotations happen around this centre so the shape of
    // the selected face is preserved.
    // ---------------------------------------------------------

    glm::vec3 faceCentre(0.0f);

    for (const std::size_t vertexIndex :
    originalFace.vertices)
    {
        faceCentre +=
            mesh.GetVertex(
                vertexIndex
            ).position;
    }

    faceCentre /=
        static_cast<float>(
            originalFace.vertices.size()
            );


    // ---------------------------------------------------------
    // Calculate safe bend radius
    // ---------------------------------------------------------

    float radius =
        std::abs(
            settings.distance
        );

    float maxBendDistance = 0.0f;

    for (const std::size_t vertexIndex :
    originalFace.vertices)
    {
        const glm::vec3 position =
            mesh.GetVertex(
                vertexIndex
            ).position;

        const glm::vec3 fromCentre =
            position -
            faceCentre;

        const float bendDistance =
            std::abs(
                glm::dot(
                    fromCentre,
                    bendDirection
                )
            );

        maxBendDistance =
            std::max(
                maxBendDistance,
                bendDistance
            );
    }

    const float minimumRadius =
        maxBendDistance +
        0.01f;

    if (radius < minimumRadius)
    {
        radius =
            minimumRadius;
    }


    /*glm::vec3 faceCentre(0.0f);


    for (const std::size_t vertexIndex :
    originalFace.vertices)
    {
        faceCentre +=
            mesh.GetVertex(
                vertexIndex
            ).position;
    }


    faceCentre /=
        static_cast<float>(
            originalFace.vertices.size()
            );*/


    // ---------------------------------------------------------
    // previousLoop initially refers to the original selected
    // face.
    // ---------------------------------------------------------

    std::vector<std::size_t>
        previousLoop =
        originalFace.vertices;


    std::vector<std::size_t>
        finalLoop;


    const std::size_t vertexCount =
        originalFace.vertices.size();


    // ---------------------------------------------------------
    // Build each segment.
    // ---------------------------------------------------------

    for (int segment = 1;
        segment <= settings.segments;
        ++segment)
    {
        const float amount =
            static_cast<float>(segment) /
            static_cast<float>(
                settings.segments
                );


        // Absolute angle used for the circular path.
        const float angle =
            totalAngle *
            amount;


        // Signed angle used to rotate the face itself.
        const float signedAngle =
            signedTotalAngle *
            amount;


        // -----------------------------------------------------
        // Circular path for the centre of this face loop.
        //
        // At angle 0:
        //
        //     offset = 0
        //
        // Initially it travels along extrusionAxis, then bends
        // towards bendDirection.
        // -----------------------------------------------------

        const glm::vec3 centreOffset =
            extrusionAxis *
            (std::sin(angle) * radius) +
            bendDirection *
            ((1.0f - std::cos(angle)) *
                radius);


        const glm::vec3 segmentCentre =
            faceCentre +
            centreOffset;


        // -----------------------------------------------------
        // Rotation for this section.
        // -----------------------------------------------------

        const glm::mat4 rotation =
            glm::rotate(
                glm::mat4(1.0f),
                signedAngle,
                rotationAxis
            );


        std::vector<std::size_t>
            newLoop;


        newLoop.reserve(
            vertexCount
        );


        // -----------------------------------------------------
        // Create new face-loop vertices.
        // -----------------------------------------------------

        for (const std::size_t originalVertexIndex :
        originalFace.vertices)
        {
            const glm::vec3 originalPosition =
                mesh.GetVertex(
                    originalVertexIndex
                ).position;


            // Position relative to original face centre.
            const glm::vec3 localOffset =
                originalPosition -
                faceCentre;


            // Rotate the cross section.
            const glm::vec3 rotatedOffset =
                glm::vec3(
                    rotation *
                    glm::vec4(
                        localOffset,
                        0.0f
                    )
                );


            const glm::vec3 newPosition =
                segmentCentre +
                rotatedOffset;


            const std::size_t newVertexIndex =
                mesh.AddVertex(
                    newPosition
                );


            newLoop.push_back(
                newVertexIndex
            );
        }


        // -----------------------------------------------------
        // Connect previous loop to this loop.
        // -----------------------------------------------------

        for (std::size_t i = 0;
            i < vertexCount;
            ++i)
        {
            const std::size_t next =
                (i + 1) %
                vertexCount;


            const std::size_t oldA =
                previousLoop[i];

            const std::size_t oldB =
                previousLoop[next];

            const std::size_t newA =
                newLoop[i];

            const std::size_t newB =
                newLoop[next];


            const std::size_t sideFace =
                mesh.AddFace(
                    {
                        oldA,
                        oldB,
                        newB,
                        newA
                    }
                );


            // Keep the same material as the original face.
            if (sideFace <
                mesh.GetFaceCount())
            {
                mesh.GetFace(
                    sideFace
                ).materialIndex =
                    originalFace.materialIndex;
            }
        }


        previousLoop =
            newLoop;


        finalLoop =
            newLoop;
    }


    // ---------------------------------------------------------
    // Replace original selected face with final end face.
    //
    // This is the same basic idea used by FaceExtrude.
    // ---------------------------------------------------------

    if (finalLoop.empty())
    {
        BOX_LOG_ERROR(
            "AngleExtrude: Failed to create final loop"
        );

        return false;
    }


    if (!mesh.SetFace(
        faceIndex,
        finalLoop))
    {
        BOX_LOG_ERROR(
            "AngleExtrude: Failed to replace original face"
        );

        return false;
    }


    // ---------------------------------------------------------
    // Rebuild modelling edges.
    // ---------------------------------------------------------

    mesh.RebuildEdges();


    BOX_LOG_INFO(
        "Angle Extrude completed. "
        << "Angle="
        << settings.angleDegrees
        << " Segments="
        << settings.segments
        << " Radius="
        << radius
        << " Vertices="
        << mesh.GetVertexCount()
        << " Faces="
        << mesh.GetFaceCount()
    );


    return true;
}