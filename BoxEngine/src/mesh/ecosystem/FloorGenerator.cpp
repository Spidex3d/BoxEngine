//#include "mesh/ecosystem/FloorGenerator.h"
//#include <miniBoxLog.h>
//
//
//FloorGenerator::~FloorGenerator() = default;
//
//
//bool FloorGenerator::Initialize()
//{
//    BOX_LOG_INFO(
//        "FloorGenerator initialized"
//    );
//
//    return true;
//}
//
//
//bool FloorGenerator::GenerateFloor(
//    float width,
//    float depth,
//    int subdivisionsX,
//    int subdivisionsZ)
//{
//    // -------------------------------------------------
//    // Validate parameters
//    // -------------------------------------------------
//
//    if (width <= 0.0f ||
//        depth <= 0.0f)
//    {
//        BOX_LOG_ERROR(
//            "FloorGenerator: invalid floor size"
//        );
//
//        return false;
//    }
//
//
//    if (subdivisionsX < 1 ||
//        subdivisionsZ < 1)
//    {
//        BOX_LOG_ERROR(
//            "FloorGenerator: subdivisions must be >= 1"
//        );
//
//        return false;
//    }
//
//
//    // -------------------------------------------------
//    // Clear previous floor
//    // -------------------------------------------------
//
//    m_vertices.clear();
//    m_faces.clear();
//
//
//    // -------------------------------------------------
//    // Work out vertex spacing
//    // -------------------------------------------------
//
//    const float stepX =
//        width /
//        static_cast<float>(
//            subdivisionsX
//            );
//
//    const float stepZ =
//        depth /
//        static_cast<float>(
//            subdivisionsZ
//            );
//
//
//    // Centre floor around world origin.
//    const float startX =
//        -width * 0.5f;
//
//    const float startZ =
//        -depth * 0.5f;
//
//
//    // -------------------------------------------------
//    // Generate vertices
//    //
//    // Example:
//    //
//    // o---o---o
//    // |   |   |
//    // o---o---o
//    // |   |   |
//    // o---o---o
//    //
//    // -------------------------------------------------
//
//    for (int z = 0;
//        z <= subdivisionsZ;
//        ++z)
//    {
//        for (int x = 0; x <= subdivisionsX; ++x)
//        {
//            const float positionX =
//                startX +
//                static_cast<float>(x)
//                * stepX;
//
//            const float positionZ =
//                startZ +
//                static_cast<float>(z)
//                * stepZ;
//
//
//            FloorVertex vertex;
//
//            vertex.position =
//                glm::vec3(
//                    positionX,
//                    0.0f,
//                    positionZ
//                );
//
//
//            m_vertices.push_back(
//                vertex
//            );
//        }
//    }
//
//
//    // -------------------------------------------------
//    // Generate quad faces
//    // -------------------------------------------------
//
//    const int rowSize = subdivisionsX + 1;
//
//
//    for (int z = 0; z < subdivisionsZ; ++z)
//    {
//        for (int x = 0; x < subdivisionsX; ++x)
//        {
//            const unsigned int v0 =
//                z * rowSize + x;
//
//            const unsigned int v1 =
//                v0 + 1;
//
//            const unsigned int v2 =
//                v0 + rowSize + 1;
//
//            const unsigned int v3 =
//                v0 + rowSize;
//
//
//            FloorFace face;
//
//            face.v0 = v0;
//            face.v1 = v1;
//            face.v2 = v2;
//            face.v3 = v3;
//
//
//            m_faces.push_back(
//                face
//            );
//        }
//    }
//
//
//    // -------------------------------------------------
//    // Debug information
//    // -------------------------------------------------
//
//    BOX_LOG_INFO("Floor generated - Vertices: "
//        << m_vertices.size()
//        << " Faces: "
//        << m_faces.size()
//    );
//
//
//    return true;
//}
//
//MeshData FloorGenerator::BuildMeshData() const
//{
//    MeshData meshData;
//
//
//    // -------------------------------------------------
//    // Convert floor vertices to MeshVertex
//    // -------------------------------------------------
//
//    meshData.vertices.reserve(
//        m_vertices.size()
//    );
//
//
//    for (const FloorVertex& floorVertex :
//        m_vertices)
//    {
//        MeshVertex vertex;
//
//        vertex.position =
//            floorVertex.position;
//
//        // Floor points upwards.
//        vertex.normal =
//            glm::vec3(
//                0.0f,
//                1.0f,
//                0.0f
//            );
//
//        vertex.uv =
//            glm::vec2(
//                floorVertex.position.x,
//                floorVertex.position.z
//            );
//
//        vertex.materialIndex = 0;
//
//        vertex.tangent =
//            glm::vec3(
//                1.0f,
//                0.0f,
//                0.0f
//            );
//
//
//        meshData.vertices.push_back(
//            vertex
//        );
//    }
//
//
//    // -------------------------------------------------
//    // Convert each quad into two render triangles.
//    //
//    // Quad:
//    //
//    // v3 -------- v2
//    // |            |
//    // |            |
//    // v0 -------- v1
//    //
//    // -------------------------------------------------
//
//    meshData.indices.reserve(
//        m_faces.size() * 6
//    );
//
//
//    for (const FloorFace& face :
//        m_faces)
//    {
//        // Triangle 1
//        meshData.indices.push_back(
//            face.v0
//        );
//
//        meshData.indices.push_back(
//            face.v2
//        );
//
//        meshData.indices.push_back(
//            face.v1
//        );
//
//
//        // Triangle 2
//        meshData.indices.push_back(
//            face.v0
//        );
//
//        meshData.indices.push_back(
//            face.v3
//        );
//
//        meshData.indices.push_back(
//            face.v2
//        );
//
//
//        // Keep the original quad as a
//        // logical face as well.
//        LogicalFace logicalFace;
//
//        logicalFace.vertexIndices =
//        {
//            face.v0,
//            face.v3,
//            face.v2,
//            face.v1
//        };
//
//        /*logicalFace.vertexIndices =
//        {
//            face.v0,
//            face.v1,
//            face.v2,
//            face.v3
//        };*/
//
//        meshData.faces.push_back(
//            logicalFace
//        );
//    }
//
//
//    BOX_LOG_INFO(
//        "Floor MeshData built - Vertices: "
//        << meshData.vertices.size()
//        << " Quads: "
//        << meshData.faces.size()
//        << " Indices: "
//        << meshData.indices.size()
//    );
//
//
//    return meshData;
//}
