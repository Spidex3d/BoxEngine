#include <mesh\MeshEditing.h>
#include <miniBoxLog.h>
#include <cmath>
#include <tools\EdgeEditController.h>
#include <algorithm>

#include <glm\gtc\constants.hpp>
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

bool MeshEditing::CreateSphere(
    int sectors,
    int stacks)
{
    Clear();


    if (sectors < 3)
    {
        sectors = 3;
    }

    if (stacks < 2)
    {
        stacks = 2;
    }


    constexpr float radius =
        0.5f;


    // =================================================
    // TOP POLE
    // =================================================

    const std::size_t topPole =
        AddVertex(
            glm::vec3(
                0.0f,
                radius,
                0.0f
            )
        );


    // =================================================
    // INTERMEDIATE RINGS
    //
    // We deliberately do NOT duplicate the UV seam.
    //
    // This is modelling topology rather than
    // render topology, so the first and last vertex
    // of each ring are connected using modulo.
    // =================================================

    for (int stack = 1;
        stack < stacks;
        ++stack)
    {
        const float stackAngle =
            glm::half_pi<float>() -
            static_cast<float>(stack) *
            glm::pi<float>() /
            static_cast<float>(stacks);


        const float ringRadius =
            radius *
            std::cos(stackAngle);


        const float y =
            radius *
            std::sin(stackAngle);


        for (int sector = 0;
            sector < sectors;
            ++sector)
        {
            const float sectorAngle =
                static_cast<float>(sector) *
                glm::two_pi<float>() /
                static_cast<float>(sectors);


            const float x =
                ringRadius *
                std::cos(sectorAngle);


            const float z =
                ringRadius *
                std::sin(sectorAngle);


            AddVertex(
                glm::vec3(
                    x,
                    y,
                    z
                )
            );
        }
    }


    // =================================================
    // BOTTOM POLE
    // =================================================

    const std::size_t bottomPole =
        AddVertex(
            glm::vec3(
                0.0f,
                -radius,
                0.0f
            )
        );


    // =================================================
    // TOP CAP
    // =================================================

    const std::size_t firstRing =
        1;


    for (int sector = 0;
        sector < sectors;
        ++sector)
    {
        const int next =
            (sector + 1) %
            sectors;


        const std::size_t currentVertex =
            firstRing +
            static_cast<std::size_t>(
                sector
                );


        const std::size_t nextVertex =
            firstRing +
            static_cast<std::size_t>(
                next
                );


        AddFace(
            {
                topPole,
                nextVertex,
                currentVertex
            }
        );
    }


    // =================================================
    // MIDDLE QUADS
    // =================================================

    const int ringCount =
        stacks - 1;


    for (int ring = 0;
        ring < ringCount - 1;
        ++ring)
    {
        const std::size_t upperRing =
            1 +
            static_cast<std::size_t>(
                ring * sectors
                );


        const std::size_t lowerRing =
            upperRing +
            static_cast<std::size_t>(
                sectors
                );


        for (int sector = 0;
            sector < sectors;
            ++sector)
        {
            const int next =
                (sector + 1) %
                sectors;


            const std::size_t upperA =
                upperRing +
                static_cast<std::size_t>(
                    sector
                    );


            const std::size_t upperB =
                upperRing +
                static_cast<std::size_t>(
                    next
                    );


            const std::size_t lowerA =
                lowerRing +
                static_cast<std::size_t>(
                    sector
                    );


            const std::size_t lowerB =
                lowerRing +
                static_cast<std::size_t>(
                    next
                    );


            AddFace(
                {
                    upperA,
                    upperB,
                    lowerB,
                    lowerA
                }
            );
        }
    }


    // =================================================
    // BOTTOM CAP
    // =================================================

    const std::size_t lastRing =
        1 +
        static_cast<std::size_t>(
            (ringCount - 1) *
            sectors
            );


    for (int sector = 0;
        sector < sectors;
        ++sector)
    {
        const int next =
            (sector + 1) %
            sectors;


        const std::size_t currentVertex =
            lastRing +
            static_cast<std::size_t>(
                sector
                );


        const std::size_t nextVertex =
            lastRing +
            static_cast<std::size_t>(
                next
                );


        AddFace(
            {
                currentVertex,
                nextVertex,
                bottomPole
            }
        );
    }


    // =================================================
    // BUILD EDITABLE EDGES
    // =================================================

    RebuildEdges();


    BOX_LOG_INFO(
        "Created editable sphere. "
        << "Sectors="
        << sectors
        << " Stacks="
        << stacks
        << " Vertices="
        << GetVertexCount()
        << " Edges="
        << GetEdgeCount()
        << " Faces="
        << GetFaceCount()
    );


    return
        !m_vertices.empty() &&
        !m_edges.empty() &&
        !m_faces.empty();
}

bool MeshEditing::CreateCylinder(
    int sectors,
    int stacks,
    float radius,
    float height)
{
    Clear();

    if (sectors < 3)
    {
        sectors = 3;
    }

    if (stacks < 1)
    {
        stacks = 1;
    }

    if (radius <= 0.0f ||
        height <= 0.0f)
    {
        BOX_LOG_ERROR(
            "MeshEditing::CreateCylinder: "
            "Invalid radius or height"
        );

        return false;
    }

    const float halfHeight =
        height * 0.5f;


    // =================================================
    // CREATE HORIZONTAL RINGS
    //
    // stacks = 1  -> 2 rings
    // stacks = 2  -> 3 rings
    // stacks = 4  -> 5 rings
    // =================================================

    for (int stack = 0;
        stack <= stacks;
        ++stack)
    {
        const float t =
            static_cast<float>(stack) /
            static_cast<float>(stacks);

        const float y =
            -halfHeight +
            t * height;


        for (int sector = 0;
            sector < sectors;
            ++sector)
        {
            const float angle =
                static_cast<float>(sector) *
                2.0f *
                pi /
                static_cast<float>(sectors);

            const float x =
                std::cos(angle) *
                radius;

            const float z =
                std::sin(angle) *
                radius;

            AddVertex(
                glm::vec3(
                    x,
                    y,
                    z
                )
            );
        }
    }


    // =================================================
    // CREATE SIDE QUADS
    // =================================================

    for (int stack = 0;
        stack < stacks;
        ++stack)
    {
        const std::size_t lowerRing =
            static_cast<std::size_t>(
                stack * sectors
                );

        const std::size_t upperRing =
            static_cast<std::size_t>(
                (stack + 1) * sectors
                );


        for (int sector = 0;
            sector < sectors;
            ++sector)
        {
            const int next =
                (sector + 1) %
                sectors;


            const std::size_t bottomA =
                lowerRing +
                static_cast<std::size_t>(
                    sector
                    );

            const std::size_t bottomB =
                lowerRing +
                static_cast<std::size_t>(
                    next
                    );

            const std::size_t topA =
                upperRing +
                static_cast<std::size_t>(
                    sector
                    );

            const std::size_t topB =
                upperRing +
                static_cast<std::size_t>(
                    next
                    );


            // Outward winding.
            AddFace(
                {
                    bottomA,
                    topA,
                    topB,
                    bottomB
                }
            );
        }
    }


    // =================================================
    // TOP CAP
    // =================================================

    std::vector<std::size_t>
        topFace;

    topFace.reserve(
        sectors
    );

    const std::size_t topRing =
        static_cast<std::size_t>(
            stacks * sectors
            );


    // Reverse order so normal points +Y.
    for (int sector = sectors - 1;
        sector >= 0;
        --sector)
    {
        topFace.push_back(
            topRing +
            static_cast<std::size_t>(
                sector
                )
        );
    }

    AddFace(
        topFace
    );


    // =================================================
    // BOTTOM CAP
    // =================================================

    std::vector<std::size_t>
        bottomFace;

    bottomFace.reserve(
        sectors
    );


    // Forward order so normal points -Y.
    for (int sector = 0;
        sector < sectors;
        ++sector)
    {
        bottomFace.push_back(
            static_cast<std::size_t>(
                sector
                )
        );
    }

    AddFace(
        bottomFace
    );


    // =================================================
    // EDGES
    // =================================================

    RebuildEdges();


    BOX_LOG_INFO(
        "Created editable cylinder. "
        << "Sectors="
        << sectors
        << " Stacks="
        << stacks
        << " Vertices="
        << GetVertexCount()
        << " Edges="
        << GetEdgeCount()
        << " Faces="
        << GetFaceCount()
    );


    return
        !m_vertices.empty() &&
        !m_faces.empty();
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

    

    // =================================================
    // CALCULATE TANGENTS
    // =================================================

    // Start all tangents at zero.
    for (MeshVertex& vertex : meshData.vertices)
    {
        vertex.tangent =
            glm::vec3(0.0f);
    }


    // Calculate a tangent for every triangle.
    for (std::size_t index = 0;
        index + 2 < meshData.indices.size();
        index += 3)
    {
        const std::uint32_t i0 =
            meshData.indices[index + 0];

        const std::uint32_t i1 =
            meshData.indices[index + 1];

        const std::uint32_t i2 =
            meshData.indices[index + 2];


        if (i0 >= meshData.vertices.size() ||
            i1 >= meshData.vertices.size() ||
            i2 >= meshData.vertices.size())
        {
            continue;
        }


        MeshVertex& v0 =
            meshData.vertices[i0];

        MeshVertex& v1 =
            meshData.vertices[i1];

        MeshVertex& v2 =
            meshData.vertices[i2];


        const glm::vec3 edge1 =
            v1.position -
            v0.position;

        const glm::vec3 edge2 =
            v2.position -
            v0.position;


        const glm::vec2 deltaUV1 =
            v1.uv -
            v0.uv;

        const glm::vec2 deltaUV2 =
            v2.uv -
            v0.uv;


        const float denominator =
            deltaUV1.x * deltaUV2.y -
            deltaUV2.x * deltaUV1.y;


        // Degenerate UV mapping.
        if (std::abs(denominator) <=
            0.000001f)
        {
            continue;
        }


        const float inverse =
            1.0f /
            denominator;


        glm::vec3 tangent =
            inverse *
            (
                edge1 * deltaUV2.y -
                edge2 * deltaUV1.y
                );


        v0.tangent += tangent;
        v1.tangent += tangent;
        v2.tangent += tangent;
    }


    // =================================================
    // NORMALIZE TANGENTS
    // =================================================
    for (MeshVertex& vertex :
        meshData.vertices)
    {
        // Make tangent perpendicular
        // to the vertex normal.
        vertex.tangent =
            vertex.tangent -
            vertex.normal *
            glm::dot(
                vertex.normal,
                vertex.tangent
            );


        const float tangentLength =
            glm::length(
                vertex.tangent
            );


        if (tangentLength >
            0.000001f)
        {
            vertex.tangent /=
                tangentLength;
        }
        else
        {
            glm::vec3 reference =
                std::abs(vertex.normal.y) <
                0.999f
                ? glm::vec3(0.0f, 1.0f, 0.0f)
                : glm::vec3(1.0f, 0.0f, 0.0f);


            vertex.tangent =
                glm::normalize(
                    glm::cross(
                        reference,
                        vertex.normal
                    )
                );
        }
    }
   
	// ================================================= End Tangent Calculation =================================================

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
// ####################################################################################################################
// ############################################ Mesh Editing add edge LoopCut #########################################
// ####################################################################################################################
std::vector<std::size_t> MeshEditing::FindEdgeRing(std::size_t startEdgeIndex) const
//std::vector<std::size_t> MeshEditing::FindEdgeLoop(std::size_t startEdgeIndex) const

{
    //std::vector<std::size_t> loopEdges;
    std::vector<std::size_t> ringEdges;

    // -------------------------------------------------
    // Validate starting edge.
    // -------------------------------------------------

    if (startEdgeIndex >= m_edges.size())
    {
        return ringEdges;
    }


    // Always include the clicked edge.
    ringEdges.push_back(
        startEdgeIndex
    );


    // -------------------------------------------------
    // Walk in one direction from an edge.
    // -------------------------------------------------

    auto WalkDirection =
        [&](std::size_t firstEdge,
            std::size_t firstFace)
    {
        std::size_t currentEdge =
            firstEdge;

       // std::size_t previousFace = static_cast<std::size_t>(-1);

        std::size_t currentFace =
            firstFace;


        // Safety guard against malformed topology.
        const std::size_t maxSteps =
            m_edges.size() + 1;


        for (std::size_t step = 0;
            step < maxSteps;
            ++step)
        {
            if (currentFace >= m_faces.size())
            {
                break;
            }


            const EditFace& face =
                m_faces[currentFace];


            // -----------------------------------------
            // For now edge loops only travel cleanly
            // through quad faces.
            // -----------------------------------------

            if (face.vertices.size() != 4)
            {
                break;
            }


            const EditEdge& edge =
                m_edges[currentEdge];


            // -----------------------------------------
            // Find which side of this quad is our
            // current edge.
            // -----------------------------------------

            int edgeSide = -1;

            for (int side = 0;
                side < 4;
                ++side)
            {
                const std::size_t a =
                    face.vertices[side];

                const std::size_t b =
                    face.vertices[
                        (side + 1) % 4
                    ];


                if ((a == edge.vertexA &&
                    b == edge.vertexB) ||
                    (a == edge.vertexB &&
                        b == edge.vertexA))
                {
                    edgeSide = side;
                    break;
                }
            }


            if (edgeSide == -1)
            {
                break;
            }


            // -----------------------------------------
            // Opposite side of a quad is two sides
            // away.
            //
            // 0 -> 2
            // 1 -> 3
            // 2 -> 0
            // 3 -> 1
            // -----------------------------------------

            const int oppositeSide =
                (edgeSide + 2) % 4;


            const std::size_t oppositeA =
                face.vertices[
                    oppositeSide
                ];

            const std::size_t oppositeB =
                face.vertices[
                    (oppositeSide + 1) % 4
                ];


            const std::size_t oppositeEdge =
                FindEdgeIndex(
                    oppositeA,
                    oppositeB
                );


            if (oppositeEdge ==
                static_cast<std::size_t>(-1))
            {
                break;
            }


            // We've returned to the beginning.
            if (oppositeEdge ==
                startEdgeIndex)
            {
                break;
            }


            // Prevent duplicates.
            if (std::find(
                ringEdges.begin(),
                ringEdges.end(),
                oppositeEdge) ==
                ringEdges.end())
            {
                ringEdges.push_back(
                    oppositeEdge
                );
            }
            else
            {
                break;
            }


            // -----------------------------------------
            // Find the next quad connected to the
            // opposite edge.
            // -----------------------------------------

            const EditEdge& nextEdge =
                m_edges[oppositeEdge];

            std::size_t nextFace =
                static_cast<std::size_t>(-1);


            for (std::size_t faceIndex = 0;
                faceIndex < m_faces.size();
                ++faceIndex)
            {
                if (faceIndex == currentFace)
                {
                    continue;
                }


                const EditFace& candidate =
                    m_faces[faceIndex];


                if (FaceContainsEdge(
                    candidate,
                    nextEdge.vertexA,
                    nextEdge.vertexB))
                {
                    nextFace =
                        faceIndex;

                    break;
                }
            }


            if (nextFace ==
                static_cast<std::size_t>(-1))
            {
                break;
            }


            // previousFace = currentFace;

            currentFace =
                nextFace;

            currentEdge =
                oppositeEdge;
        }
    };


    // -------------------------------------------------
    // Find every face touching the starting edge.
    //
    // Normally:
    // interior edge = 2 faces
    // boundary edge = 1 face
    // -------------------------------------------------

    const EditEdge& startEdge =
        m_edges[startEdgeIndex];


    std::vector<std::size_t>
        startFaces;


    for (std::size_t faceIndex = 0;
        faceIndex < m_faces.size();
        ++faceIndex)
    {
        if (FaceContainsEdge(
            m_faces[faceIndex],
            startEdge.vertexA,
            startEdge.vertexB))
        {
            startFaces.push_back(
                faceIndex
            );
        }
    }


    // -------------------------------------------------
    // Walk away from the clicked edge through each
    // attached face.
    //
    // Walking both directions is important for an
    // open edge loop.
    // -------------------------------------------------

    for (const std::size_t faceIndex :
    startFaces)
    {
        WalkDirection(
            startEdgeIndex,
            faceIndex
        );
    }


    

    return ringEdges;
}

std::vector<std::size_t> MeshEditing::FindEdgeLoop(std::size_t startEdgeIndex) const
{
   
    std::vector<std::size_t> loopEdges;

    const std::size_t invalid =
        static_cast<std::size_t>(-1);


    if (startEdgeIndex >= m_edges.size())
    {
        return loopEdges;
    }


    // Always include clicked edge.
    loopEdges.push_back(
        startEdgeIndex
    );


    const EditEdge& startEdge =
        m_edges[startEdgeIndex];

    // =================================================
    // N-GON BOUNDARY LOOP
    //
    // Cylinder top/bottom caps are n-gons.
    // If the selected edge belongs to one,
    // select the complete boundary of that face.
    // =================================================

    for (const EditFace& face :
        m_faces)
    {
        // Regular quad topology is handled
        // by the normal edge-loop code below.
        if (face.vertices.size() <= 4)
        {
            continue;
        }


        // Does this n-gon contain our clicked edge?
        if (!FaceContainsEdge(
            face,
            startEdge.vertexA,
            startEdge.vertexB))
        {
            continue;
        }


        // We found the n-gon containing the edge.
        // Replace the current result with the
        // complete boundary of that face.
        loopEdges.clear();


        for (std::size_t index = 0;
            index < face.vertices.size();
            ++index)
        {
            const std::size_t vertexA =
                face.vertices[index];

            const std::size_t vertexB =
                face.vertices[
                    (index + 1) %
                        face.vertices.size()
                ];


            const std::size_t edgeIndex =
                FindEdgeIndex(
                    vertexA,
                    vertexB
                );


            if (edgeIndex != invalid)
            {
                loopEdges.push_back(
                    edgeIndex
                );
            }
        }


        // We have the complete n-gon boundary,
        // so no normal traversal is required.
        return loopEdges;
    }


    // -------------------------------------------------
    // Returns true if two edges belong to the
    // same face.
    // -------------------------------------------------

    auto EdgesShareFace =
        [&](std::size_t edgeAIndex,
            std::size_t edgeBIndex)
    {
        if (edgeAIndex >= m_edges.size() ||
            edgeBIndex >= m_edges.size())
        {
            return false;
        }


        const EditEdge& edgeA =
            m_edges[edgeAIndex];

        const EditEdge& edgeB =
            m_edges[edgeBIndex];


        for (const EditFace& face :
            m_faces)
        {
            const bool containsA =
                FaceContainsEdge(
                    face,
                    edgeA.vertexA,
                    edgeA.vertexB
                );

            if (!containsA)
            {
                continue;
            }


            const bool containsB =
                FaceContainsEdge(
                    face,
                    edgeB.vertexA,
                    edgeB.vertexB
                );


            if (containsB)
            {
                return true;
            }
        }


        return false;
    };


    // -------------------------------------------------
    // Walk from one endpoint of the clicked edge.
    // -------------------------------------------------

    auto WalkDirection =
        [&](std::size_t startVertex)
    {
        std::size_t currentVertex =
            startVertex;

        std::size_t incomingEdge =
            startEdgeIndex;


        const std::size_t maxSteps =
            m_edges.size() + 1;


        for (std::size_t step = 0;
            step < maxSteps;
            ++step)
        {
            std::size_t nextEdge =
                invalid;


            // -----------------------------------------
            // Examine every edge attached to the
            // current vertex.
            // -----------------------------------------

            for (std::size_t edgeIndex = 0;
                edgeIndex < m_edges.size();
                ++edgeIndex)
            {
                if (edgeIndex == incomingEdge)
                {
                    continue;
                }


                const EditEdge& candidate =
                    m_edges[edgeIndex];


                const bool connected =
                    candidate.vertexA ==
                    currentVertex ||
                    candidate.vertexB ==
                    currentVertex;


                if (!connected)
                {
                    continue;
                }


                // -------------------------------------
                // A real loop continuation should be
                // the edge opposite the incoming edge
                // in the topology.
                //
                // Therefore it must NOT share a face
                // with the incoming edge.
                // -------------------------------------

                if (!EdgesShareFace(
                    incomingEdge,
                    edgeIndex))
                {
                    // More than one valid candidate
                    // means ambiguous / irregular
                    // topology.
                    if (nextEdge != invalid)
                    {
                        nextEdge =
                            invalid;

                        break;
                    }


                    nextEdge =
                        edgeIndex;
                }
            }


            // No clean continuation.
            if (nextEdge == invalid)
            {
                break;
            }


            // Closed loop.
            if (nextEdge ==
                startEdgeIndex)
            {
                break;
            }


            // Prevent duplicates / infinite walks.
            if (std::find(
                loopEdges.begin(),
                loopEdges.end(),
                nextEdge) !=
                loopEdges.end())
            {
                break;
            }


            loopEdges.push_back(
                nextEdge
            );


            // -----------------------------------------
            // Move to the other end of nextEdge.
            // -----------------------------------------

            const EditEdge& edge =
                m_edges[nextEdge];


            const std::size_t nextVertex =
                edge.vertexA == currentVertex
                ? edge.vertexB
                : edge.vertexA;


            currentVertex =
                nextVertex;

            incomingEdge =
                nextEdge;
        }
    };


    // Walk both ways from the clicked edge.
    WalkDirection(
        startEdge.vertexA
    );

    WalkDirection(
        startEdge.vertexB
    );


    return loopEdges;
}


std::size_t MeshEditing::FindEdgeIndex(std::size_t vertexA, std::size_t vertexB) const
{
    for (std::size_t edgeIndex = 0;
        edgeIndex < m_edges.size();
        ++edgeIndex)
    {
        const EditEdge& edge =
            m_edges[edgeIndex];

        const bool same =
            edge.vertexA == vertexA &&
            edge.vertexB == vertexB;

        const bool reversed =
            edge.vertexA == vertexB &&
            edge.vertexB == vertexA;

        if (same || reversed)
        {
            return edgeIndex;
        }
    }

    return static_cast<std::size_t>(-1);
}


bool MeshEditing::FaceContainsEdge(const EditFace & face, std::size_t vertexA, std::size_t vertexB) const
{
        if (face.vertices.size() < 2)
        {
            return false;
        }

        for (std::size_t index = 0;
            index < face.vertices.size();
            ++index)
        {
            const std::size_t a =
                face.vertices[index];

            const std::size_t b =
                face.vertices[
                    (index + 1) %
                        face.vertices.size()
                ];

            if ((a == vertexA &&
                b == vertexB) ||
                (a == vertexB &&
                    b == vertexA))
            {
                return true;
            }
        }
    return false;
}


