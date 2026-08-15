#include "mesh/modifiers/LoopCut.h"

#include <mesh/MeshEditing.h>
#include <miniBoxLog.h>

#include <glm/glm.hpp>

#include <vector>
#include <cstddef>


namespace
{
    // -------------------------------------------------
    // Compare two edges regardless of direction.
    //
    // 0-1 is the same edge as 1-0.
    // -------------------------------------------------
    bool SameEdge(
        std::size_t a0,
        std::size_t a1,
        std::size_t b0,
        std::size_t b1)
    {
        return
            (a0 == b0 && a1 == b1) ||
            (a0 == b1 && a1 == b0);
    }


    // -------------------------------------------------
    // Information about one quad crossed by
    // the loop cut.
    // -------------------------------------------------
    struct LoopFaceStep
    {
        std::size_t faceIndex =
            static_cast<std::size_t>(-1);

        // Edge where we entered this face.
        std::size_t entryA = 0;
        std::size_t entryB = 0;

        // Opposite edge where we leave.
        std::size_t oppositeA = 0;
        std::size_t oppositeB = 0;

        // Position of entry edge in face vertices.
        std::size_t edgePosition = 0;
    };
}


// ============================================================================
// Loop Cut
// ============================================================================

bool LoopCut::Use(
    MeshEditing& mesh,
    std::size_t edgeIndex,
    float amount)
{
    // -------------------------------------------------
    // Validate selected edge.
    // -------------------------------------------------

    if (edgeIndex >=
        mesh.GetEdgeCount())
    {
        BOX_LOG_ERROR(
            "LoopCut: Invalid edge index"
        );

        return false;
    }


    // Prevent the cut sitting exactly
    // on an existing edge.
    amount =
        glm::clamp(
            amount,
            0.01f,
            0.99f
        );


    const EditEdge selectedEdge =
        mesh.GetEdge(edgeIndex);

    const std::size_t startEdgeA =
        selectedEdge.vertexA;

    const std::size_t startEdgeB =
        selectedEdge.vertexB;


    // =================================================
    // PHASE 1
    // Discover the complete quad loop.
    //
    // IMPORTANT:
    // Do not change any topology during this phase.
    // =================================================

    std::vector<LoopFaceStep>
        loopSteps;


    std::size_t currentEdgeA =
        startEdgeA;

    std::size_t currentEdgeB =
        startEdgeB;


    std::size_t previousFace =
        static_cast<std::size_t>(-1);


    const auto& faces =
        mesh.GetFaces();


    while (true)
    {
        std::size_t currentFace =
            static_cast<std::size_t>(-1);

        std::size_t currentEdgePosition =
            static_cast<std::size_t>(-1);


        // ---------------------------------------------
        // Find a quad containing the current edge.
        //
        // Don't immediately return to the face
        // we just came from.
        // ---------------------------------------------

        for (std::size_t f = 0;
            f < faces.size();
            ++f)
        {
            if (f == previousFace)
            {
                continue;
            }


            const EditFace& face =
                faces[f];


            // Loop Cut currently supports
            // quad topology only.
            if (face.vertices.size() != 4)
            {
                continue;
            }


            for (std::size_t i = 0;
                i < 4;
                ++i)
            {
                const std::size_t next =
                    (i + 1) % 4;


                const std::size_t a =
                    face.vertices[i];

                const std::size_t b =
                    face.vertices[next];


                if (SameEdge(
                    a,
                    b,
                    currentEdgeA,
                    currentEdgeB))
                {
                    currentFace =
                        f;

                    currentEdgePosition =
                        i;

                    break;
                }
            }


            if (currentFace !=
                static_cast<std::size_t>(-1))
            {
                break;
            }
        }


        // ---------------------------------------------
        // No next quad means this is an open loop
        // ending at a mesh boundary.
        // ---------------------------------------------

        if (currentFace ==
            static_cast<std::size_t>(-1))
        {
            BOX_LOG_INFO(
                "LoopCut reached mesh boundary"
            );

            break;
        }


        // ---------------------------------------------
        // Have we already visited this face?
        //
        // If yes, we've travelled all the way
        // around a closed loop.
        // ---------------------------------------------

        bool alreadyVisited =
            false;


        for (const LoopFaceStep& existing :
            loopSteps)
        {
            if (existing.faceIndex ==
                currentFace)
            {
                alreadyVisited =
                    true;

                break;
            }
        }


        if (alreadyVisited)
        {
            BOX_LOG_INFO(
                "LoopCut closed loop"
            );

            break;
        }


        // ---------------------------------------------
        // Get the quad in a useful order.
        //
        // v0 ---- v1   <- entry edge
        // |        |
        // |        |
        // v3 ---- v2   <- opposite edge
        // ---------------------------------------------

        const EditFace& face =
            faces[currentFace];


        const std::size_t v0 =
            face.vertices[
                currentEdgePosition
            ];

        const std::size_t v1 =
            face.vertices[
                (currentEdgePosition + 1) % 4
            ];

        const std::size_t v2 =
            face.vertices[
                (currentEdgePosition + 2) % 4
            ];

        const std::size_t v3 =
            face.vertices[
                (currentEdgePosition + 3) % 4
            ];


        // ---------------------------------------------
        // Store this step in the loop.
        // ---------------------------------------------

        LoopFaceStep step;

        step.faceIndex =
            currentFace;

        step.entryA =
            v0;

        step.entryB =
            v1;

        step.oppositeA =
            v3;

        step.oppositeB =
            v2;

        step.edgePosition =
            currentEdgePosition;


        loopSteps.push_back(
            step
        );


        BOX_LOG_INFO(
            "LoopCut face "
            << step.faceIndex
            << " entry "
            << step.entryA
            << "-"
            << step.entryB
            << " opposite "
            << step.oppositeA
            << "-"
            << step.oppositeB
        );


        // ---------------------------------------------
        // Walk across the quad.
        //
        // The opposite edge becomes the entry edge
        // for the next neighbouring quad.
        // ---------------------------------------------

        previousFace =
            currentFace;

        currentEdgeA =
            step.oppositeA;

        currentEdgeB =
            step.oppositeB;
    }


    BOX_LOG_INFO(
        "LoopCut discovered "
        << loopSteps.size()
        << " quad faces"
    );


    if (loopSteps.empty())
    {
        BOX_LOG_ERROR(
            "LoopCut: No quad loop found"
        );

        return false;
    }


    // =================================================
    // PHASE 2
    // Create one cut vertex for each crossed edge,
    // then split every quad in the discovered loop.
    // =================================================


    // -------------------------------------------------
    // Store one new vertex for each crossed edge.
    // -------------------------------------------------



    struct CutEdgeVertex
    {
        std::size_t edgeA = 0;
        std::size_t edgeB = 0;

        std::size_t newVertex =
            static_cast<std::size_t>(-1);
    };


    std::vector<CutEdgeVertex>
        cutVertices;


    // -------------------------------------------------
    // Helper: find an existing cut vertex for an edge.
    // -------------------------------------------------

    const auto FindCutVertex =
        [&](std::size_t edgeA,
            std::size_t edgeB)
        -> std::size_t
    {
        for (const CutEdgeVertex& cut :
            cutVertices)
        {
            if (SameEdge(
                cut.edgeA,
                cut.edgeB,
                edgeA,
                edgeB))
            {
                return cut.newVertex;
            }
        }

        return static_cast<std::size_t>(-1);
    };


    // -------------------------------------------------
    // Helper: create a cut vertex if this edge
    // hasn't already been processed.
    // -------------------------------------------------

    const auto GetOrCreateCutVertex =
        [&](std::size_t edgeA,
            std::size_t edgeB,
            bool reverseAmount)
        -> std::size_t
    {
        const std::size_t existing =
            FindCutVertex(
                edgeA,
                edgeB
            );

        if (existing !=
            static_cast<std::size_t>(-1))
        {
            return existing;
        }


        const glm::vec3 positionA =
            mesh.GetVertex(edgeA).position;

        const glm::vec3 positionB =
            mesh.GetVertex(edgeB).position;


        /*
         * This is important.
         *
         * The winding direction of neighbouring
         * faces can reverse an edge.
         *
         * reverseAmount lets us maintain the same
         * physical cut position around the loop.
         */
        const float t =
            reverseAmount
            ? 1.0f - amount
            : amount;


        const glm::vec3 cutPosition =
            glm::mix(
                positionA,
                positionB,
                t
            );


        const std::size_t newVertex =
            mesh.AddVertex(
                cutPosition
            );


        CutEdgeVertex cut;

        cut.edgeA =
            edgeA;

        cut.edgeB =
            edgeB;

        cut.newVertex =
            newVertex;


        cutVertices.push_back(
            cut
        );


        return newVertex;
    };


    // =================================================
    // PHASE 2A
    // Create all cut vertices BEFORE modifying faces.
    // =================================================

    for (std::size_t stepIndex = 0;
        stepIndex < loopSteps.size();
        ++stepIndex)
    {
        const LoopFaceStep& step =
            loopSteps[stepIndex];


        /*
         * Entry edge.
         *
         * For the first face we use the original
         * amount direction.
         *
         * For later faces, this edge should already
         * exist from the previous face, so normally
         * no new vertex will be created here.
         */
        GetOrCreateCutVertex(
            step.entryA,
            step.entryB,
            false
        );


        /*
         * Opposite edge.
         *
         * We use the face's v3 -> v2 ordering,
         * which keeps the cut aligned across
         * the quad.
         */
        GetOrCreateCutVertex(
            step.oppositeA,
            step.oppositeB,
            false
        );
    }


    // =================================================
    // PHASE 2B
    // Split every face in the loop.
    // =================================================

    for (const LoopFaceStep& step :
        loopSteps)
    {
        /*
         * Important:
         * loopSteps was discovered BEFORE topology
         * modification, so these original face
         * indices are still valid here.
         */

        const EditFace originalFace =
            mesh.GetFace(
                step.faceIndex
            );


        if (originalFace.vertices.size() != 4)
        {
            BOX_LOG_ERROR(
                "LoopCut: Face changed before split"
            );

            return false;
        }


        const std::size_t v0 =
            originalFace.vertices[
                step.edgePosition
            ];

        const std::size_t v1 =
            originalFace.vertices[
                (step.edgePosition + 1) % 4
            ];

        const std::size_t v2 =
            originalFace.vertices[
                (step.edgePosition + 2) % 4
            ];

        const std::size_t v3 =
            originalFace.vertices[
                (step.edgePosition + 3) % 4
            ];


        // -------------------------------------------------
        // Retrieve the shared cut vertices.
        // -------------------------------------------------

        const std::size_t newA =
            FindCutVertex(
                v0,
                v1
            );


        const std::size_t newB =
            FindCutVertex(
                v3,
                v2
            );


        if (newA ==
            static_cast<std::size_t>(-1) ||
            newB ==
            static_cast<std::size_t>(-1))
        {
            BOX_LOG_ERROR(
                "LoopCut: Missing cut vertex"
            );

            return false;
        }


        // -------------------------------------------------
        // Replace original quad with first half.
        //
        // v0 ---- newA
        // |         |
        // |         |
        // v3 ---- newB
        // -------------------------------------------------

        if (!mesh.SetFace(
            step.faceIndex,
            {
                v0,
                newA,
                newB,
                v3
            }))
        {
            BOX_LOG_ERROR(
                "LoopCut: Failed to replace face "
                << step.faceIndex
            );

            return false;
        }


        // -------------------------------------------------
        // Add second half.
        //
        // newA ---- v1
        // |          |
        // |          |
        // newB ---- v2
        // -------------------------------------------------

        mesh.AddFace(
            {
                newA,
                v1,
                v2,
                newB
            }
        );
    }


    // =================================================
    // PHASE 3
    // Rebuild genuine modelling edges.
    // =================================================

    mesh.RebuildEdges();


    BOX_LOG_INFO(
        "LoopCut full ring successful. "
        << "LoopFaces="
        << loopSteps.size()
        << " CutVertices="
        << cutVertices.size()
        << " Vertices="
        << mesh.GetVertexCount()
        << " Edges="
        << mesh.GetEdgeCount()
        << " Faces="
        << mesh.GetFaceCount()
    );


    return true;


}