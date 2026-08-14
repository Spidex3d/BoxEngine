#include "mesh/modifiers/FaceCut.h"
#include <entity/Entity.h>

#include <mesh/MeshEditing.h>
#include <miniBoxLog.h>

#include <glm/glm.hpp>


#include "mesh/modifiers/LoopCut.h"

#include <mesh/MeshEditing.h>
#include <miniBoxLog.h>

#include <glm/glm.hpp>

bool FaceCut::Use(MeshEditing& mesh, std::size_t edgeIndex, float amount)
{
    if (edgeIndex >= mesh.GetEdgeCount())
    {
        BOX_LOG_ERROR("FaceCut: Invalid edge index");
        return false;
    }

    amount =
        glm::clamp(
            amount,
            0.01f,
            0.99f
        );

    const EditEdge selectedEdge =
        mesh.GetEdge(edgeIndex);

    const std::size_t edgeA =
        selectedEdge.vertexA;

    const std::size_t edgeB =
        selectedEdge.vertexB;


    // -----------------------------------------
    // Find a quad containing this edge.
    // -----------------------------------------

    std::size_t faceIndex =
        static_cast<std::size_t>(-1);

    std::size_t edgePosition =
        static_cast<std::size_t>(-1);

    const auto& faces =
        mesh.GetFaces();

    for (std::size_t f = 0;
        f < faces.size();
        ++f)
    {
        const EditFace& face =
            faces[f];

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

            const bool sameEdge =
                (a == edgeA &&
                    b == edgeB) ||
                (a == edgeB &&
                    b == edgeA);

            if (sameEdge)
            {
                faceIndex =
                    f;

                edgePosition =
                    i;

                break;
            }
        }

        if (faceIndex !=
            static_cast<std::size_t>(-1))
        {
            break;
        }
    }


    if (faceIndex ==
        static_cast<std::size_t>(-1))
    {
        BOX_LOG_ERROR(
            "FaceCut: selected edge is not part of a quad"
        );

        return false;
    }


    // Make a copy because we're going
    // to modify the face list.
    const EditFace originalFace = mesh.GetFace(faceIndex);


    /*
     * Reorder the quad so:
     *
     * v0-v1 = selected edge
     * v3-v2 = opposite edge
     */

    const std::size_t v0 =
        originalFace.vertices[
            edgePosition
        ];

    const std::size_t v1 =
        originalFace.vertices[
            (edgePosition + 1) % 4
        ];

    const std::size_t v2 =
        originalFace.vertices[
            (edgePosition + 2) % 4
        ];

    const std::size_t v3 =
        originalFace.vertices[
            (edgePosition + 3) % 4
        ];


    // -----------------------------------------
    // New vertex on selected edge
    // -----------------------------------------

    const glm::vec3 position0 =
        mesh.GetVertex(v0).position;

    const glm::vec3 position1 =
        mesh.GetVertex(v1).position;

    const glm::vec3 cutA =
        glm::mix(
            position0,
            position1,
            amount
        );

    const std::size_t newA =
        mesh.AddVertex(
            cutA
        );


    // -----------------------------------------
    // New vertex on opposite edge
    // -----------------------------------------

    /*
     * Reverse interpolation here so both
     * cut points stay aligned through the quad.
     */
    const glm::vec3 position3 =
        mesh.GetVertex(v3).position;

    const glm::vec3 position2 =
        mesh.GetVertex(v2).position;

    const glm::vec3 cutB =
        glm::mix(
            position3,
            position2,
            amount
        );

    const std::size_t newB =
        mesh.AddVertex(
            cutB
        );


    // -----------------------------------------
    // Split original quad into two quads
    // -----------------------------------------

    if (!mesh.SetFace(
        faceIndex,
        {
            v0,
            newA,
            newB,
            v3
        }))
    {
        return false;
    }

    mesh.AddFace(
        {
            newA,
            v1,
            v2,
            newB
        }
    );


    // Rebuild all real modelling edges.
    mesh.RebuildEdges();


    BOX_LOG_INFO(
        "FaceCut successful. "
        << "Vertices="
        << mesh.GetVertexCount()
        << " Edges="
        << mesh.GetEdgeCount()
        << " Faces="
        << mesh.GetFaceCount()
    );

    return true;
}