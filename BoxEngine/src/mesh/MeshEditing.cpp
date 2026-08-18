#include <mesh\MeshEditing.h>
#include <miniBoxLog.h>

void MeshEditing::Clear()
{
    m_vertices.clear();
    m_edges.clear();
    m_faces.clear();
}

bool MeshEditing::CreateCube()
{
    Clear();

    // 8 logical cube corners.
    m_vertices =
    {
        {{-0.5f, -0.5f,  0.5f}}, // 0 front bottom left
        {{ 0.5f, -0.5f,  0.5f}}, // 1 front bottom right
        {{ 0.5f,  0.5f,  0.5f}}, // 2 front top right
        {{-0.5f,  0.5f,  0.5f}}, // 3 front top left

        {{-0.5f, -0.5f, -0.5f}}, // 4 back bottom left
        {{ 0.5f, -0.5f, -0.5f}}, // 5 back bottom right
        {{ 0.5f,  0.5f, -0.5f}}, // 6 back top right
        {{-0.5f,  0.5f, -0.5f}}  // 7 back top left
    };

    // 12 real modelling edges.
    m_edges =
    {
        // Front
        {0, 1},
        {1, 2},
        {2, 3},
        {3, 0},

        // Back
        {4, 5},
        {5, 6},
        {6, 7},
        {7, 4},

        // Front to back
        {0, 4},
        {1, 5},
        {2, 6},
        {3, 7}
    };

    // 6 quad faces.
    m_faces =
    {
        {{0, 1, 2, 3}}, // Front
        {{5, 4, 7, 6}}, // Back

        {{4, 0, 3, 7}}, // Left
        {{1, 5, 6, 2}}, // Right

        {{3, 2, 6, 7}}, // Top
        {{4, 5, 1, 0}}  // Bottom

    };
        m_faces[0].materialIndex = 1;
       

    return
        m_vertices.size() == 8 &&
        m_edges.size() == 12 &&
        m_faces.size() == 6;
}
// plane creation
bool MeshEditing::CreatePlane()
{
    Clear();
    // 4 logical plane corners.
    m_vertices =
    {
        {{-0.5f, 0.0f, -0.5f}}, // 0 bottom left
        {{ 0.5f, 0.0f, -0.5f}}, // 1 bottom right
        {{ 0.5f, 0.0f,  0.5f}}, // 2 top right
        {{-0.5f, 0.0f,  0.5f}}  // 3 top left
    };
    // 4 real modelling edges.
    m_edges =
    {
        {0, 1},
        {1, 2},
        {2, 3},
        {3, 0}
    };
    // Single quad face.
    m_faces =
    {
        //{{0, 1, 2, 3}}
        {{0, 3, 2, 1}}
    };
    return
        m_vertices.size() == 4 &&
        m_edges.size() == 4 &&
		m_faces.size() == 1;
}

bool MeshEditing::CreatePyramid()
{
	Clear();
    m_vertices =
        {
        {{-0.5f, 0.0f, -0.5f}}, // 0 bottom left
        {{ 0.5f, 0.0f, -0.5f}}, // 1 bottom right
        {{ 0.5f, 0.0f,  0.5f}}, // 2 top right
        {{-0.5f, 0.0f,  0.5f}}, // 3 top left
        {{ 0.0f, 1.0f,  0.0f}}  // 4 apex
	};
    m_edges =
    {
        {0, 1},
        {1, 2},
        {2, 3},
        {3, 0},
        {0, 4},
        {1, 4},
        {2, 4},
        {3, 4}
	};
    m_faces =
    {
    {{1, 0, 4}}, // Front
    {{2, 1, 4}}, // Right
    {{3, 2, 4}}, // Back
    {{0, 3, 4}}, // Left
    {{0, 1, 2,3}} // Bottom correct
	};

    return
        m_vertices.size() == 5 &&
        m_edges.size() == 8 &&
		m_faces.size() == 5;
}




std::size_t MeshEditing::GetVertexCount() const
{
    return m_vertices.size();
}

std::size_t MeshEditing::GetEdgeCount() const
{
    return m_edges.size();
}

std::size_t MeshEditing::GetFaceCount() const
{
    return m_faces.size();
}
// ############################################ Mesh Editing get vertex, edge, face  #########################################
EditVertex& MeshEditing::GetVertex(
    std::size_t index)
{
    return m_vertices.at(index);
}

const EditVertex& MeshEditing::GetVertex(
    std::size_t index) const
{
    return m_vertices.at(index);
}

EditEdge& MeshEditing::GetEdge(
    std::size_t index)
{
    return m_edges.at(index);
}

const EditEdge& MeshEditing::GetEdge(
    std::size_t index) const
{
    return m_edges.at(index);
}

EditFace& MeshEditing::GetFace(
    std::size_t index)
{
    return m_faces.at(index);
}

const EditFace& MeshEditing::GetFace(
    std::size_t index) const
{
    return m_faces.at(index);
}




// ############################################ Mesh Editing add vertex, face,  #########################################
std::size_t MeshEditing::AddVertex(
    const glm::vec3& position)
{
    m_vertices.push_back(
        EditVertex{ position }
    );

    return m_vertices.size() - 1;
}

std::size_t MeshEditing::AddFace(
    const std::vector<std::size_t>& vertices)
{
    m_faces.push_back(
        EditFace{ vertices }
    );

    return m_faces.size() - 1;
}

bool MeshEditing::SetFace(
    std::size_t faceIndex,
    const std::vector<std::size_t>& vertices)
{
    if (faceIndex >= m_faces.size())
    {
        return false;
    }

    m_faces[faceIndex].vertices =
        vertices;

    return true;
}

void MeshEditing::RebuildEdges()
{
    m_edges.clear();

    auto EdgeExists =
        [&](std::size_t a,
            std::size_t b)
    {
        for (const EditEdge& edge :
            m_edges)
        {
            const bool same =
                edge.vertexA == a &&
                edge.vertexB == b;

            const bool reversed =
                edge.vertexA == b &&
                edge.vertexB == a;

            if (same || reversed)
            {
                return true;
            }
        }

        return false;
    };

    for (const EditFace& face :
        m_faces)
    {
        if (face.vertices.size() < 2)
        {
            continue;
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

            if (!EdgeExists(a, b))
            {
                m_edges.push_back(
                    { a, b }
                );
            }
        }
    }
}
// ####################################################################################################################
// ############################################ Mesh Editing add edge LoopCut #########################################
// ####################################################################################################################


bool MeshEditing::BuildRenderMesh(MeshData& meshData) const
{
    if (m_vertices.empty() || m_faces.empty())
    {
        BOX_LOG_ERROR("MeshEditing::BuildRenderMesh: No vertices, edges, or faces to build mesh");
        return false;
	}
    
    meshData.vertices.clear();
    meshData.indices.clear();

    /*
     * Build every modelling face separately.
     *
     * This deliberately creates duplicated
     * render vertices so each face can have
     * its own flat normal and UV coordinates.
     */
    for (const EditFace& face : m_faces)
    {
        if (face.vertices.size() < 3)
        {
            continue;
        }

        /*
         * Validate every logical vertex index.
         */
        bool validFace = true;

        for (const std::size_t vertexIndex :
        face.vertices)
        {
            if (vertexIndex >=
                m_vertices.size())
            {
                validFace = false;
                break;
            }
        }

        if (!validFace)
        {
            BOX_LOG_ERROR(
                "MeshEditing::BuildRenderMesh: "
                "Face contains invalid vertex index"
            );

            return false;
        }

        /*
         * Calculate one flat normal for the
         * complete modelling face.
         */
        const glm::vec3& positionA =
            m_vertices[
                face.vertices[0]
            ].position;

        const glm::vec3& positionB =
            m_vertices[
                face.vertices[1]
            ].position;

        const glm::vec3& positionC =
            m_vertices[
                face.vertices[2]
            ].position;

        const glm::vec3 edgeAB =
            positionB - positionA;

        const glm::vec3 edgeAC =
            positionC - positionA;

        const glm::vec3 crossProduct =
            glm::cross(
                edgeAB,
                edgeAC
            );

        const float normalLength =
            glm::length(
                crossProduct
            );

        if (normalLength <= 0.000001f)
        {
            BOX_LOG_WARNING("MeshEditing::BuildRenderMesh: "
                "Skipping degenerate face");

            continue;
        }

        const glm::vec3 faceNormal =
            crossProduct /
            normalLength;

        /*
         * Remember where this face starts in
         * the render-vertex array.
         */
        const std::uint32_t baseVertex =
            static_cast<std::uint32_t>(
                meshData.vertices.size()
                );

        /*
         * Create one render vertex for each
         * logical corner of this face.
         */
        for (std::size_t corner = 0;
            corner < face.vertices.size();
            ++corner)
        {
            MeshVertex renderVertex;

            renderVertex.position =
                m_vertices[
                    face.vertices[corner]
                ].position;

            renderVertex.normal =
                faceNormal;
            // ################### new for matirials ##########################
            renderVertex.materialIndex = face.materialIndex;

            /*
             * Simple quad UV mapping.
             *
             * For our first editable cube
             * every face contains four corners.
             */
            if (face.vertices.size() == 4)
            {
                switch (corner)
                {
                case 0:
                    renderVertex.uv =
                        glm::vec2(
                            0.0f,
                            0.0f
                        );
                    break;

                case 1:
                    renderVertex.uv =
                        glm::vec2(
                            1.0f,
                            0.0f
                        );
                    break;

                case 2:
                    renderVertex.uv =
                        glm::vec2(
                            1.0f,
                            1.0f
                        );
                    break;

                case 3:
                    renderVertex.uv =
                        glm::vec2(
                            0.0f,
                            1.0f
                        );
                    break;
                }
            }
            else if (face.vertices.size() == 3)
            {
                switch (corner)
                {
                case 0:
                    renderVertex.uv =
                        glm::vec2(0.0f, 0.0f);
                    break;

                case 1:
                    renderVertex.uv =
                        glm::vec2(1.0f, 0.0f);
                    break;

                case 2:
                    renderVertex.uv =
                        glm::vec2(0.5f, 1.0f);
                    break;
                }
            }
            else
            {
               
                const glm::vec3& p = renderVertex.position;

                /*
                 * Temporary planar UV projection
                 * for n-gons.
                 *
                 * Choose projection plane from the
                 * dominant face-normal direction.
                 */

                const glm::vec3 absNormal =
                    glm::abs(faceNormal);

                if (absNormal.y >= absNormal.x &&
                    absNormal.y >= absNormal.z)
                {
                    // Mostly horizontal face:
                    // project X/Z.
                    renderVertex.uv =
                        glm::vec2(
                            p.x + 0.5f,
                            p.z + 0.5f
                        );
                }
                else if (
                    absNormal.x >= absNormal.y &&
                    absNormal.x >= absNormal.z)
                {
                    // Mostly X-facing:
                    // project Z/Y.
                    renderVertex.uv =
                        glm::vec2(
                            p.z + 0.5f,
                            p.y + 0.5f
                        );
                }
                else
                {
                    // Mostly Z-facing:
                    // project X/Y.
                    renderVertex.uv =
                        glm::vec2(
                            p.x + 0.5f,
                            p.y + 0.5f
                        );
                }








            }
           
            meshData.vertices.push_back(renderVertex);
        }

        /*
         * Triangulate the face using a
         * triangle fan:
         *
         * quad:
         *
         * 0----1
         * |   /|
         * | /  |
         * 3----2
         *
         * triangles:
         * 0,1,2
         * 0,2,3
         *
         * This also works for faces with
         * more than four vertices.
         */
        for (std::size_t corner = 1;
            corner + 1 <
            face.vertices.size();
            ++corner)
        {
            meshData.indices.push_back(
                baseVertex
            );

            meshData.indices.push_back(
                baseVertex +
                static_cast<std::uint32_t>(
                    corner
                    )
            );

            meshData.indices.push_back(
                baseVertex +
                static_cast<std::uint32_t>(
                    corner + 1
                    )
            );
        }
    }

    if (meshData.vertices.empty())
    {
        BOX_LOG_ERROR(
            "MeshEditing::BuildRenderMesh: "
            "No render vertices were generated"
        );

        return false;
    }

    if (meshData.indices.empty())
    {
        BOX_LOG_ERROR(
            "MeshEditing::BuildRenderMesh: "
            "No triangle indices were generated"
        );

        return false;
    }

    BOX_LOG_INFO(
        "Built render mesh: "
        << meshData.vertices.size()
        << " vertices, "
        << meshData.indices.size()
        << " indices"
    );

    return true;
   
}

const std::vector<EditVertex>&
MeshEditing::GetVertices() const
{
    return m_vertices;
}

const std::vector<EditEdge>&
MeshEditing::GetEdges() const
{
    return m_edges;
}

const std::vector<EditFace>&
MeshEditing::GetFaces() const
{
    return m_faces;
}

