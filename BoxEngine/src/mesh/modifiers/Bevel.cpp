#include "mesh/modifiers/Bevel.h"
#include <entity/Entity.h>

#include <mesh/MeshEditing.h>
#include <miniBoxLog.h>

#include <glm/glm.hpp>

#include <vector>
#include <cstddef>
#include <algorithm>


namespace
{
    static constexpr std::size_t InvalidIndex =
        static_cast<std::size_t>(-1);


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


    struct AdjacentFace
    {
        std::size_t faceIndex =
            InvalidIndex;

        // Position of the selected edge
        // inside the face.
        std::size_t edgePosition =
            InvalidIndex;
    };


    /*
     * Records the new bevel vertex created
     * along an edge connected to one endpoint
     * of the selected bevel edge.
     */
    struct BevelCut
    {
        std::size_t centreVertex =
            InvalidIndex;

        std::size_t neighbourVertex =
            InvalidIndex;

        std::size_t newVertex =
            InvalidIndex;
    };


    std::size_t FindCutVertex(
        const std::vector<BevelCut>& cuts,
        std::size_t centreVertex,
        std::size_t neighbourVertex)
    {
        for (const BevelCut& cut : cuts)
        {
            if (cut.centreVertex ==
                centreVertex &&
                cut.neighbourVertex ==
                neighbourVertex)
            {
                return cut.newVertex;
            }
        }

        return InvalidIndex;
    }
}


// ============================================================================
// ONE SEGMENT EDGE BEVEL
// ============================================================================

bool Bevel::Use(
    MeshEditing& mesh,
    std::size_t edgeIndex,
    float width, int segments, float profile)
{
   
    // -------------------------------------------------
    // Validate edge
    // -------------------------------------------------

    if (edgeIndex >=
        mesh.GetEdgeCount())
    {
        BOX_LOG_ERROR(
            "Bevel: Invalid edge index"
        );

        return false;
    }


    width = glm::clamp(width, 0.001f, 0.49f);
	segments = glm::clamp(segments, 1, 6);
    profile = glm::clamp(profile, 0.01f, 0.99f);

    const EditEdge selectedEdge =
        mesh.GetEdge(edgeIndex);


    const std::size_t edgeA =
        selectedEdge.vertexA;

    const std::size_t edgeB =
        selectedEdge.vertexB;


    if (edgeA >= mesh.GetVertexCount() ||
        edgeB >= mesh.GetVertexCount())
    {
        BOX_LOG_ERROR(
            "Bevel: Invalid edge vertices"
        );

        return false;
    }


    // =================================================
    // PHASE 1
    // Find the two faces sharing selected edge.
    // =================================================

    std::vector<AdjacentFace>
        adjacentFaces;


    const auto& faces =
        mesh.GetFaces();


    for (std::size_t faceIndex = 0;
        faceIndex < faces.size();
        ++faceIndex)
    {
        const EditFace& face =
            faces[faceIndex];


        const std::size_t count =
            face.vertices.size();


        if (count < 3)
        {
            continue;
        }


        for (std::size_t i = 0;
            i < count;
            ++i)
        {
            const std::size_t next =
                (i + 1) % count;


            const std::size_t a =
                face.vertices[i];

            const std::size_t b =
                face.vertices[next];


            if (SameEdge(
                a,
                b,
                edgeA,
                edgeB))
            {
                AdjacentFace adjacent;

                adjacent.faceIndex =
                    faceIndex;

                adjacent.edgePosition =
                    i;


                adjacentFaces.push_back(
                    adjacent
                );

                break;
            }
        }
    }


    /*
     * For this first bevel implementation,
     * require a normal manifold edge:
     *
     * exactly two faces sharing it.
     */
    if (adjacentFaces.size() != 2)
    {
        BOX_LOG_ERROR(
            "Bevel: selected edge must have exactly two adjacent faces"
        );

        return false;
    }


    // =================================================
    // PHASE 2
    // Find the four neighbouring vertices.
    //
    // Each adjacent face contributes:
    //
    // neighbour at endpoint A
    // neighbour at endpoint B
    // =================================================

    std::vector<BevelCut>
        bevelCuts;


    bevelCuts.reserve(4);


    // These remember the cuts belonging to
    // the two directly adjacent faces.
    std::size_t face0CutA =
        InvalidIndex;

    std::size_t face0CutB =
        InvalidIndex;

    std::size_t face1CutA =
        InvalidIndex;

    std::size_t face1CutB =
        InvalidIndex;


    for (std::size_t adjacentIndex = 0;
        adjacentIndex < 2;
        ++adjacentIndex)
    {
        const AdjacentFace& adjacent =
            adjacentFaces[adjacentIndex];


        const EditFace originalFace =
            mesh.GetFace(
                adjacent.faceIndex
            );


        const std::size_t count =
            originalFace.vertices.size();


        const std::size_t i =
            adjacent.edgePosition;


        const std::size_t current =
            originalFace.vertices[i];

        const std::size_t next =
            originalFace.vertices[
                (i + 1) % count
            ];


        /*
         * Work out the face ordering around
         * the selected edge.
         */
        std::size_t neighbourA =
            InvalidIndex;

        std::size_t neighbourB =
            InvalidIndex;


        if (current == edgeA &&
            next == edgeB)
        {
            neighbourA =
                originalFace.vertices[
                    (i + count - 1) % count
                ];

            neighbourB =
                originalFace.vertices[
                    (i + 2) % count
                ];
        }
        else
        {
            /*
             * Face contains edge as B -> A.
             */
            neighbourB =
                originalFace.vertices[
                    (i + count - 1) % count
                ];

            neighbourA =
                originalFace.vertices[
                    (i + 2) % count
                ];
        }


        // -----------------------------------------
        // Create cut near A
        // -----------------------------------------

        const glm::vec3 positionA =
            mesh.GetVertex(edgeA).position;

        const glm::vec3 neighbourAPosition =
            mesh.GetVertex(
                neighbourA
            ).position;


        const glm::vec3 directionA =
            neighbourAPosition -
            positionA;


        const float lengthA =
            glm::length(directionA);


        if (lengthA <= 0.000001f)
        {
            BOX_LOG_ERROR(
                "Bevel: Degenerate edge near endpoint A"
            );

            return false;
        }


        const float widthA =
            glm::min(
                width,
                lengthA * 0.45f
            );


        const glm::vec3 cutPositionA =
            positionA +
            glm::normalize(directionA) *
            widthA;


        const std::size_t newA =
            mesh.AddVertex(
                cutPositionA
            );


        bevelCuts.push_back(
            {
                edgeA,
                neighbourA,
                newA
            }
        );


        // -----------------------------------------
        // Create cut near B
        // -----------------------------------------

        const glm::vec3 positionB =
            mesh.GetVertex(edgeB).position;

        const glm::vec3 neighbourBPosition =
            mesh.GetVertex(
                neighbourB
            ).position;


        const glm::vec3 directionB =
            neighbourBPosition -
            positionB;


        const float lengthB =
            glm::length(directionB);


        if (lengthB <= 0.000001f)
        {
            BOX_LOG_ERROR(
                "Bevel: Degenerate edge near endpoint B"
            );

            return false;
        }


        const float widthB =
            glm::min(
                width,
                lengthB * 0.45f
            );


        const glm::vec3 cutPositionB =
            positionB +
            glm::normalize(directionB) *
            widthB;


        const std::size_t newB =
            mesh.AddVertex(
                cutPositionB
            );


        bevelCuts.push_back(
            {
                edgeB,
                neighbourB,
                newB
            }
        );


        if (adjacentIndex == 0)
        {
            face0CutA =
                newA;

            face0CutB =
                newB;
        }
        else
        {
            face1CutA =
                newA;

            face1CutB =
                newB;
        }
    }


    // =================================================
    // PHASE 3
    // Trim the two faces directly touching
    // the selected edge.
    // =================================================

    for (std::size_t adjacentIndex = 0;
        adjacentIndex < 2;
        ++adjacentIndex)
    {
        const AdjacentFace& adjacent =
            adjacentFaces[adjacentIndex];


        const EditFace originalFace =
            mesh.GetFace(
                adjacent.faceIndex
            );


        std::vector<std::size_t>
            newFaceVertices;


        newFaceVertices.reserve(
            originalFace.vertices.size()
        );


        for (const std::size_t vertexIndex :
        originalFace.vertices)
        {
            if (vertexIndex == edgeA)
            {
                newFaceVertices.push_back(
                    adjacentIndex == 0
                    ? face0CutA
                    : face1CutA
                );
            }
            else if (vertexIndex == edgeB)
            {
                newFaceVertices.push_back(
                    adjacentIndex == 0
                    ? face0CutB
                    : face1CutB
                );
            }
            else
            {
                newFaceVertices.push_back(
                    vertexIndex
                );
            }
        }


        if (!mesh.SetFace(
            adjacent.faceIndex,
            newFaceVertices))
        {
            BOX_LOG_ERROR(
                "Bevel: Failed to trim adjacent face"
            );

            return false;
        }
    }


    // =================================================
    // PHASE 4
    // Clip the faces at both ends of the bevel.
    //
    // Example cube side face:
    //
    //      old corner
    //          *
    //
    // becomes
    //
    //       *-----*
    //
    // so a quad normally becomes a pentagon.
    // =================================================

    for (std::size_t faceIndex = 0;
        faceIndex < faces.size();
        ++faceIndex)
    {
        /*
         * Skip the two faces we just trimmed.
         */
        if (faceIndex ==
            adjacentFaces[0].faceIndex ||
            faceIndex ==
            adjacentFaces[1].faceIndex)
        {
            continue;
        }


        const EditFace originalFace =
            mesh.GetFace(
                faceIndex
            );


        const std::size_t count =
            originalFace.vertices.size();


        std::vector<std::size_t>
            newFaceVertices;


        bool changed =
            false;


        for (std::size_t i = 0;
            i < count;
            ++i)
        {
            const std::size_t vertex =
                originalFace.vertices[i];


            if (vertex != edgeA &&
                vertex != edgeB)
            {
                newFaceVertices.push_back(
                    vertex
                );

                continue;
            }


            const std::size_t previous =
                originalFace.vertices[
                    (i + count - 1) %
                        count
                ];


            const std::size_t next =
                originalFace.vertices[
                    (i + 1) %
                        count
                ];


            const std::size_t previousCut =
                FindCutVertex(
                    bevelCuts,
                    vertex,
                    previous
                );


            const std::size_t nextCut =
                FindCutVertex(
                    bevelCuts,
                    vertex,
                    next
                );


            /*
             * This should be the face covering
             * the bevel endpoint.
             */
            if (previousCut != InvalidIndex &&
                nextCut != InvalidIndex)
            {
                newFaceVertices.push_back(
                    previousCut
                );

                newFaceVertices.push_back(
                    nextCut
                );


                changed =
                    true;
            }
            else
            {
                /*
                 * If it isn't an endpoint face,
                 * leave the vertex alone.
                 */
                newFaceVertices.push_back(
                    vertex
                );
            }
        }


        if (changed)
        {
            if (!mesh.SetFace(
                faceIndex,
                newFaceVertices))
            {
                BOX_LOG_ERROR(
                    "Bevel: Failed to clip endpoint face"
                );

                return false;
            }
        }
    }


    // =================================================
    // PHASE 5
    // Create the new bevel face.
    //
    // face0:
    // A0 -------- B0
    //
    // face1:
    // A1 -------- B1
    //
    // bevel:
    //
    // A0 -------- B0
    // |            |
    // A1 -------- B1
    // =================================================

   /* mesh.AddFace(
        {
            face0CutA,
            face0CutB,
            face1CutB,
            face1CutA
        }
    );*/
    /*mesh.AddFace(
        {
            face1CutA,
            face1CutB,
            face0CutB,
            face0CutA
        }
    );*/
    //=========================================================================================
	// ============================ New code for multiple segments ============================
    //=========================================================================================
    const glm::vec3 startA =
        mesh.GetVertex(
            face1CutA
        ).position;

    const glm::vec3 startB =
        mesh.GetVertex(
            face1CutB
        ).position;

    const glm::vec3 endA =
        mesh.GetVertex(
            face0CutA
        ).position;

    const glm::vec3 endB =
        mesh.GetVertex(
            face0CutB
        ).position;
    // =========================================
    std::vector<std::size_t>bevelRowA;

    std::vector<std::size_t>bevelRowB;
    // =========================================
    bevelRowA.push_back(face1CutA);

    bevelRowB.push_back(face1CutB);
    
    const glm::vec3 originalA =
        mesh.GetVertex(
            edgeA
        ).position;

    const glm::vec3 originalB =
        mesh.GetVertex(
            edgeB
        ).position;

    const glm::vec3 straightMidA =
        (startA + endA) * 0.5f;

    const glm::vec3 straightMidB =
        (startB + endB) * 0.5f;

   /* const float profileStrength =
        (profile - 0.5f) * 2.0f;*/

    // =========================================
    for (int segment = 1;
        segment < segments;
        ++segment)
    {
        glm::vec3 controlA;
        glm::vec3 controlB;

        if (profile <= 0.5f)
        {
            const float amount =
                profile * 2.0f;

            controlA =
                glm::mix(
                    straightMidA,
                    originalA,
                    amount
                );

            controlB =
                glm::mix(
                    straightMidB,
                    originalB,
                    amount
                );
        }
        else
        {
            const float amount =
                (profile - 0.5f) * 2.0f;

            const glm::vec3 awayA =
                straightMidA +
                (straightMidA - originalA);

            const glm::vec3 awayB =
                straightMidB +
                (straightMidB - originalB);

            controlA =
                glm::mix(
                    originalA,
                    awayA,
                    amount
                );

            controlB =
                glm::mix(
                    originalB,
                    awayB,
                    amount
                );
        }


        const float t =
            static_cast<float>(segment) /
            static_cast<float>(segments);

        const float oneMinusT =
            1.0f - t;


        // Quadratic Bezier curve.
        const glm::vec3 positionA =
            oneMinusT *
            oneMinusT *
            startA
            +
            2.0f *
            oneMinusT *
            t *
            controlA
            +
            t *
            t *
            endA;

        const glm::vec3 positionB =
            oneMinusT *
            oneMinusT *
            startB
            +
            2.0f *
            oneMinusT *
            t *
            controlB
            +
            t *
            t *
            endB;


        const std::size_t newA =
            mesh.AddVertex(
                positionA
            );

        const std::size_t newB =
            mesh.AddVertex(
                positionB
            );

        bevelRowA.push_back(
            newA
        );

        bevelRowB.push_back(
            newB
        );
    }
    /*for (int segment = 1; segment < segments; ++segment)
    {

        glm::vec3 controlA;
        glm::vec3 controlB;

        if (profile <= 0.5f)
        {
            const float amount =
                profile * 2.0f;

            controlA =
                glm::mix(
                    straightMidA,
                    originalA,
                    amount
                );

            controlB =
                glm::mix(
                    straightMidB,
                    originalB,
                    amount
                );
        }
        else
        {
            const float amount =
                (profile - 0.5f) * 2.0f;

            const glm::vec3 awayA =
                straightMidA +
                (straightMidA - originalA);

            const glm::vec3 awayB =
                straightMidB +
                (straightMidB - originalB);

            controlA =
                glm::mix(
                    originalA,
                    awayA,
                    amount
                );

            controlB =
                glm::mix(
                    originalB,
                    awayB,
                    amount
                );
        }


        const float t =
            static_cast<float>(segment) /
            static_cast<float>(segments);

        const glm::vec3 positionA =
            glm::mix(
                startA,
                endA,
                t
            );

        const glm::vec3 positionB =
            glm::mix(
                startB,
                endB,
                t
            );

        const std::size_t newA =
            mesh.AddVertex(
                positionA
            );

        const std::size_t newB =
            mesh.AddVertex(
                positionB
            );

        bevelRowA.push_back(
            newA
        );

        bevelRowB.push_back(
            newB
        );
    }*/
    // =================================================
    bevelRowA.push_back(face0CutA);

    bevelRowB.push_back(face0CutB);
    // =================================================
    for (std::size_t row = 0;
        row + 1 < bevelRowA.size();
        ++row)
    {
        mesh.AddFace(
            {
                bevelRowA[row],
                bevelRowB[row],
                bevelRowB[row + 1],
                bevelRowA[row + 1]
            }
        );
    }


    // =================================================
    // PHASE 6
    // Rebuild real modelling edges.
    // =================================================

    mesh.RebuildEdges();


    BOX_LOG_INFO(
        "Bevel successful. "
        << "Width="
        << width
        << " Vertices="
        << mesh.GetVertexCount()
        << " Edges="
        << mesh.GetEdgeCount()
        << " Faces="
        << mesh.GetFaceCount()
    );


    return true;
}


//struct BevelModifierData
//{
//	std::size_t edgeIndex = 0;
//
//	float width = 0.1f;
//
//	int segments = 1;
//
//	float profile = 0.5f;
//};
//
//// create a bevel modifier that takes an edge index and an amount, and modifies the mesh by beveling the edge by the given amount.
//// The bevel should create new vertices and faces along the edge, and adjust the positions of the existing vertices accordingly.
//// The function should return true if the operation was successful, and false otherwise.
//
//bool Bevel::Use(MeshEditing& mesh, std::size_t edgeIndex, float width, int segments, float profile)
//{
//    if (edgeIndex >= mesh.GetEdgeCount())
//    {
//        BOX_LOG_ERROR("Bevel: Invalid edge index");
//        return false;
//    }
//	
//
//
//
//	// 6 segments is the maximum number of segments for the bevel
//	segments = glm::clamp(segments, 1, 6); 
//	// Clamp the width to a max range of 0.01 to 0.5, to prevent the bevel from being too small or too large.
//	width = glm::clamp(width, 0.01f, 0.5f);
//	// 1.0 is the maximum profile value for the bevel 1 = bevel out 0 = bevel in
//	profile = glm::clamp(profile, 0.01f, 1.0f); 
//	
//
//
//	return true;
//}