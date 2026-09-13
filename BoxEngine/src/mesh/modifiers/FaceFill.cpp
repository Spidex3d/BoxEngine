#include "mesh/modifiers/FaceFill.h"
#include <mesh/MeshEditing.h>
#include <miniBoxLog.h>
#include <algorithm>



bool FaceFill::Use(MeshEditing& mesh, const std::vector<std::size_t>& vertices)
{

    
        // A face needs at least three vertices.
        if (vertices.size() < 3)
        {
            BOX_LOG_WARNING("FaceFill: At least 3 vertices are required");

            return false;
        }

        // Make sure every selected vertex exists.
        for (const std::size_t vertexIndex :
        vertices)
        {
            if (vertexIndex >= mesh.GetVertexCount())
            {
                BOX_LOG_ERROR("FaceFill: Invalid vertex index");

                return false;
            }
        }

        //mesh.AddFace(vertices);

        std::vector<std::size_t> faceVertices = vertices;

        std::reverse(faceVertices.begin(), faceVertices.end());

        mesh.AddFace(faceVertices);

        mesh.RebuildEdges();

        BOX_LOG_INFO("FaceFill: Created face with " << vertices.size() << " vertices");

        return true;
}
