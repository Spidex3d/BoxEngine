#include "entity/Entity.h"
#include <shader/Shader.h>
#include <camera/Camera.h>
#include <miniBoxLog.h>
#include <stb/stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <utility>
#include <vector>
#include <cmath>
#include <algorithm>

Entity::Entity(
    int id,
    const std::string& name)
    : m_id(id),
    m_name(name)
{
}

Entity::~Entity()
{
    Destroy();
}

Entity::Entity(Entity&& other) noexcept
    :m_meshData(std::move(other.m_meshData)),
    m_material(std::move(other.m_material)),
    m_id(other.m_id),
    m_name(std::move(other.m_name)),
    m_position(other.m_position),
    m_rotation(other.m_rotation),
    m_scale(other.m_scale),
    m_visible(other.m_visible),
    m_vao(other.m_vao),
    m_vbo(other.m_vbo),
    m_ebo(other.m_ebo),
    m_vertexCount(other.m_vertexCount),
    m_indexCount(other.m_indexCount),
    m_useIndices(other.m_useIndices)
{
    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_ebo = 0;

    other.m_vertexCount = 0;
    other.m_indexCount = 0;
    other.m_useIndices = false;
    
}

Entity& Entity::operator=(Entity&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    Destroy();

    m_meshData =
        std::move(other.m_meshData);

    m_material =
        std::move(other.m_material);

    m_id = other.m_id;
    m_name = std::move(other.m_name);

    m_position = other.m_position;
    m_rotation = other.m_rotation;
    m_scale = other.m_scale;
    m_visible = other.m_visible;

    m_vao = other.m_vao;
    m_vbo = other.m_vbo;
    m_ebo = other.m_ebo;

    m_vertexCount =
        other.m_vertexCount;

    m_indexCount =
        other.m_indexCount;

    m_useIndices =
        other.m_useIndices;

    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_ebo = 0;

    other.m_vertexCount = 0;
    other.m_indexCount = 0;
    other.m_useIndices = false;

    return *this;

    
}
// Create an entity from existing .mbx mesh data
bool Entity::CreateFromMeshData(const MeshData& meshData)
{
    Destroy();

    if (!meshData.IsValid())
    {
        BOX_LOG_ERROR("Entity::CreateFromMeshData received invalid mesh data");

        return false;
    }

    m_meshData = meshData;

    std::vector<float> interleavedVertices;

    interleavedVertices.reserve(
        m_meshData.vertices.size() * 8
    );

    for (const MeshVertex& vertex :
        m_meshData.vertices)
    {
        // Position
        interleavedVertices.push_back(
            vertex.position.x
        );

        interleavedVertices.push_back(
            vertex.position.y
        );

        interleavedVertices.push_back(
            vertex.position.z
        );

        // Normal
        interleavedVertices.push_back(
            vertex.normal.x
        );

        interleavedVertices.push_back(
            vertex.normal.y
        );

        interleavedVertices.push_back(
            vertex.normal.z
        );

        // UV
        interleavedVertices.push_back(
            vertex.uv.x
        );

        interleavedVertices.push_back(
            vertex.uv.y
        );
    }

    glGenVertexArrays(1, &m_vao);

    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        interleavedVertices.size() *
        sizeof(float),
        interleavedVertices.data(),
        GL_STATIC_DRAW
    );

    if (m_meshData.HasIndices())
    {
        glGenBuffers(1, &m_ebo);

        glBindBuffer(
            GL_ELEMENT_ARRAY_BUFFER,
            m_ebo
        );

        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            m_meshData.indices.size() *
            sizeof(std::uint32_t),
            m_meshData.indices.data(),
            GL_STATIC_DRAW
        );

        m_useIndices = true;

        m_indexCount =
            static_cast<GLsizei>(
                m_meshData.indices.size()
                );
    }
    else
    {
        m_useIndices = false;
        m_indexCount = 0;
    }

    constexpr GLsizei stride =
        8 * sizeof(float);

    // Position
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        stride,
        reinterpret_cast<void*>(0)
    );

    // Normal
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        stride,
        reinterpret_cast<void*>(
            3 * sizeof(float)
            )
    );

    // UV
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        stride,
        reinterpret_cast<void*>(
            6 * sizeof(float)
            )
    );

    glBindVertexArray(0);

    m_vertexCount =
        static_cast<GLsizei>(
            m_meshData.vertices.size()
            );

    const bool valid =
        m_vao != 0 &&
        m_vbo != 0 &&
        m_vertexCount > 0;

    if (!valid)
    {
        BOX_LOG_ERROR("Entity::CreateFromMeshData failed");

        Destroy();
        return false;
    }

    BOX_LOG_INFO("Created entity from imported mesh: "
        << m_name
        << " Vertices="
        << m_vertexCount
        << " Indices="
        << m_indexCount
    );

    return true;
}


bool Entity::CreateCube()
{
    Destroy();

    if (!m_editableMesh.CreateCube())
    {
        return false;
    }

    if (!m_editableMesh.BuildRenderMesh(
        m_meshData))
    {
        return false;
    }

    if (!CreateBuffersFromMeshData())
    {
        return false;
    }

    m_aabbMin =
        glm::vec3(-0.5f);

    m_aabbMax =
        glm::vec3(0.5f);

    return true;




 //   Destroy();

 //   m_useIndices = false;
 //   m_indexCount = 0;

 //   m_aabbMin = glm::vec3(-0.5f);
 //   m_aabbMax = glm::vec3(0.5f);

 //   // Position XYZ, Normal XYZ
 //   const float vertices[] =
	//
 //   {
 //       // Front face +Z
 //       -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
 //        0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
 //        0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f, 1.0f, 1.0f,

	//    -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
	//     0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
	//    -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f, 0.0f, 1.0f,

 //       // Back face -Z
 //        0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
	//    -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
	//    -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f, 0.0f, 1.0f,

	//     0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
	//    -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
	//     0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f, 1.0f, 1.0f,

 //        // Left face -X
 //        -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
 //        -0.5f, -0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
 //        -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
 //        
 //        -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
 //        -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
 //        -0.5f,  0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
 //        
 //        // Right face +X
 //         0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
 //         0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
 //         0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
 //        
 //         0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
 //         0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
 //         0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f,     

 //         // Top face +Y
	//      -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
	//       0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
	//       0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f, 1.0f, 0.0f,

	//      -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
	//       0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
	//      -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f, 0.0f, 0.0f,

 //         // Bottom face -Y
	//      -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
	//       0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
	//       0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f, 1.0f, 1.0f,

	//      -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
	//       0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
	//      -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f, 0.0f, 1.0f
 //   };

 //  // m_vertexCount = 36;

	//// ############################################### Mesh Data ####################################################
 //   m_meshData.vertices.clear();
 //   m_meshData.indices.clear();

 //   constexpr std::size_t floatsPerVertex = 8;

 //   const std::size_t totalFloatCount =
 //       sizeof(vertices) /
 //       sizeof(vertices[0]);

 //   const std::size_t cubeVertexCount =
 //       totalFloatCount /
 //       floatsPerVertex;

 //   m_meshData.vertices.reserve(
 //       cubeVertexCount
 //   );

 //   for (std::size_t index = 0;
 //       index < totalFloatCount;
 //       index += floatsPerVertex)
 //   {
 //       MeshVertex vertex;

 //       vertex.position =
 //           glm::vec3(
 //               vertices[index + 0],
 //               vertices[index + 1],
 //               vertices[index + 2]
 //           );

 //       vertex.normal =
 //           glm::vec3(
 //               vertices[index + 3],
 //               vertices[index + 4],
 //               vertices[index + 5]
 //           );

 //       vertex.uv =
 //           glm::vec2(
 //               vertices[index + 6],
 //               vertices[index + 7]
 //           );

 //       m_meshData.vertices.push_back(
 //           vertex 
 //       );
 //   }

 //   m_vertexCount = static_cast<GLsizei>(m_meshData.vertices.size());

 //   // ##############################

 //   if (!m_editableMesh.CreateCube())
 //   {
 //       BOX_LOG_ERROR("Failed to create editable cube topology");

 //       return false;
 //   }
 //   MeshData testMesh;

 //   if (m_editableMesh.BuildRenderMesh(
 //       testMesh))
 //   {
 //       BOX_LOG_INFO(
 //           "Editable render mesh: "
 //           << testMesh.vertices.size()
 //           << " vertices, "
 //           << testMesh.indices.size()
 //           << " indices"
 //       );
 //   }
 //   // ##############################
 //  

 //   glGenVertexArrays(1, &m_vao);
 //   glGenBuffers(1, &m_vbo);

 //   glBindVertexArray(m_vao);
 //   glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

 //   glBufferData(
 //       GL_ARRAY_BUFFER,
 //       sizeof(vertices),
 //       vertices,
 //       GL_STATIC_DRAW
 //   );

 //   // Position
 //   glEnableVertexAttribArray(0);
 //   glVertexAttribPointer(
 //       0,
 //       3,
 //       GL_FLOAT,
 //       GL_FALSE,
 //       8 * sizeof(float),
 //       reinterpret_cast<void*>(0)
 //   );

 //   // Normal
 //   glEnableVertexAttribArray(1);
 //   glVertexAttribPointer(
 //       1,
 //       3,
 //       GL_FLOAT,
 //       GL_FALSE,
 //       8 * sizeof(float),
 //       reinterpret_cast<void*>(3 * sizeof(float))
 //   );

 //   // Texture coordinates
 //   glEnableVertexAttribArray(2);

 //   glVertexAttribPointer(
 //       2,
 //       2,
 //       GL_FLOAT,
 //       GL_FALSE,
 //       8 * sizeof(float),
 //       reinterpret_cast<void*>(
 //           6 * sizeof(float)
 //           )
 //   );




 //   glBindBuffer(GL_ARRAY_BUFFER, 0);
 //   glBindVertexArray(0);

 //   const bool valid = m_vao != 0 && m_vbo != 0;

 //   if (!valid)
 //   {
 //       BOX_LOG_ERROR(
 //           "Entity::CreateCube failed for entity "
 //           << m_name
 //       );

 //       Destroy();
 //       return false;
 //   }

 //   BOX_LOG_INFO(
 //       "Created cube entity: "
 //       << m_name
 //       << " ID=" << m_id
 //       << " Mesh vertices="
 //       << m_meshData.vertices.size()
 //   );
	//// Log the editable mesh information 8, 12, 6
 //   BOX_LOG_INFO(
 //       "Editable cube: "
 //       << m_editableMesh.GetVertexCount()
 //       << " vertices, "
 //       << m_editableMesh.GetEdgeCount()
 //       << " edges, "
 //       << m_editableMesh.GetFaceCount()
 //       << " faces"
 //   );

 //   return true;
}

bool Entity::CreatePlane()
{
    Destroy();

    m_useIndices = false;
    m_indexCount = 0;
    m_aabbMin = glm::vec3(-0.5f, 0.0f, -0.5f);
    m_aabbMax = glm::vec3(0.5f, 0.0f, 0.5f);

    // Position XYZ, Normal XYZ, UV XY
    const float vertices[] =
    {
        // Positions             // Normals         // UVs
        // Top face +Y
          -0.5f,  0.5f,  0.5f,   0.0f,  1.0f, 0.0f,  0.0f, 1.0f,
           0.5f,  0.5f,  0.5f,   0.0f,  1.0f, 0.0f,  1.0f, 1.0f,
           0.5f,  0.5f, -0.5f,   0.0f,  1.0f, 0.0f,  1.0f, 0.0f,

          -0.5f,  0.5f,  0.5f,   0.0f,  1.0f, 0.0f,  0.0f, 1.0f,
           0.5f,  0.5f, -0.5f,   0.0f,  1.0f, 0.0f,  1.0f, 0.0f,
          -0.5f,  0.5f, -0.5f,   0.0f,  1.0f, 0.0f,  0.0f, 0.0f
    };

    m_meshData.vertices.clear();
    m_meshData.indices.clear();

    constexpr std::size_t floatsPerVertex = 8;

    const std::size_t totalFloatCount = sizeof(vertices) / sizeof(vertices[0]);

    const std::size_t planeVertexCount = totalFloatCount / floatsPerVertex;

    m_meshData.vertices.reserve(planeVertexCount);

    for (std::size_t index = 0;
        index < totalFloatCount;
        index += floatsPerVertex)
    {
        MeshVertex vertex;

        vertex.position =
            glm::vec3(
                vertices[index + 0],
                vertices[index + 1],
                vertices[index + 2]
            );

        vertex.normal =
            glm::vec3(
                vertices[index + 3],
                vertices[index + 4],
                vertices[index + 5]
            );

        vertex.uv =
            glm::vec2(
                vertices[index + 6],
                vertices[index + 7]
            );

        m_meshData.vertices.push_back(vertex);
    }

    m_vertexCount = static_cast<GLsizei>(m_meshData.vertices.size());

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(0));

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
        

    // Texture coordinates
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    const bool valid = m_vao != 0 && m_vbo != 0;

    if (!valid)
    {
        BOX_LOG_ERROR(
            "Entity::CreatePlane failed for entity "
            << m_name
        );

        Destroy();
        return false;
    }

    BOX_LOG_INFO(
        "Created plane entity: "
        << m_name
        << " ID=" << m_id
        << " Mesh vertices="
        << m_meshData.vertices.size()
    );

    return true;

}

// shader for the selection outline effect
void Entity::DrawMesh() const
{
    if (m_vao == 0)
    {
        return;
    }

    glBindVertexArray(m_vao);

    if (m_useIndices)
    {
        glDrawElements(
            GL_TRIANGLES,
            m_indexCount,
            GL_UNSIGNED_INT,
            nullptr
        );
    }
    else
    {
        glDrawArrays(
            GL_TRIANGLES,
            0,
            m_vertexCount
        );
    }

    glBindVertexArray(0);

  
}

// ############################################################################################
// ####################################### mesh editing functions #############################
// ################################################ vertex ####################################

std::size_t Entity::GetVertexCount() const
{
    return m_meshData.vertices.size();
}
const MeshVertex* Entity::GetVertex(
    std::size_t index) const
{
    if (index >=
        m_meshData.vertices.size())
    {
        return nullptr;
    }

    return &m_meshData.vertices[index];
}

bool Entity::SetVertexPosition(
    std::size_t index,
    const glm::vec3& position)
{
    if (index >=
        m_meshData.vertices.size())
    {
        return false;
    }

    m_meshData.vertices[index].position =
        position;

    return true;
}

bool Entity::UploadMeshData()
{
    if (m_vbo == 0 || m_meshData.vertices.empty())
    {
        return false;
    }

    std::vector<float> vertices;

    vertices.reserve(m_meshData.vertices.size() * 8);

    for (const MeshVertex& vertex : m_meshData.vertices)
    {
        vertices.push_back(vertex.position.x);

        vertices.push_back(vertex.position.y);

        vertices.push_back(vertex.position.z);

        vertices.push_back(vertex.normal.x);

        vertices.push_back(vertex.normal.y);

        vertices.push_back(vertex.normal.z);

        vertices.push_back(vertex.uv.x);

        vertices.push_back(vertex.uv.y);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;
}

void Entity::AddSelectedVertex(std::size_t index)
{
    if (index >= m_meshData.vertices.size())
    {
        return;
    }

    if (std::find(
        m_selectedVertices.begin(),
        m_selectedVertices.end(),
        index) ==
        m_selectedVertices.end())
    {
        m_selectedVertices.push_back(
            index
        );
    }
}

void Entity::ClearSelectedVertices()
{
    m_selectedVertices.clear();
}
void Entity::SelectVertex(std::size_t index)
{
    if (index >= m_meshData.vertices.size())
    {
        return;
    }

    m_selectedVertices.clear();

    m_selectedVertices.push_back(
        index
    );
}
bool Entity::IsVertexSelected(std::size_t index) const
{
    return std::find(m_selectedVertices.begin(),
        m_selectedVertices.end(), index) != m_selectedVertices.end();
}
const std::vector<std::size_t>&Entity::GetSelectedVertices() const
{
    return m_selectedVertices;
}
// ############################################ Edge ##########################################

bool Entity::SetEdgePosition(std::size_t index, const glm::vec3& positionA, const glm::vec3& positionB)
{
    
	return false; // Edge functionality is not implemented in this version
}

void Entity::ClearSelectedEdges()
{
    m_selectedEdges.clear();
}

void Entity::SelectEdge(std::size_t index)
{
    
    m_selectedEdges.clear();

    m_selectedEdges.push_back(index);
}

bool Entity::IsEdgeSelected(std::size_t index) const
{
    return std::find(m_selectedEdges.begin(),
        m_selectedEdges.end(), index) != m_selectedEdges.end();
}



// ############################################ Face ##########################################
void Entity::ClearSelectedFace()
{
    m_selectedFaces.clear();
}
void Entity::SelectFace(std::size_t index)
{

    m_selectedFaces.clear();

    m_selectedFaces.push_back(index);
}

bool Entity::IsFaceSelected(std::size_t index) const
{
    return std::find(m_selectedFaces.begin(),
        m_selectedFaces.end(), index) != m_selectedFaces.end();
}
// ############################################# Mesh Data##########################################
bool Entity::CreateBuffersFromMeshData()
{
    if (m_meshData.vertices.empty())
    {
        BOX_LOG_ERROR(
            "CreateBuffersFromMeshData: No vertices"
        );

        return false;
    }

    // ------------------------------------------------
    // Build interleaved GPU vertex data
    // Position XYZ
    // Normal   XYZ
    // UV       XY
    // ------------------------------------------------

    std::vector<float> interleavedVertices;

    interleavedVertices.reserve(
        m_meshData.vertices.size() * 8
    );

    for (const MeshVertex& vertex :
        m_meshData.vertices)
    {
        // Position
        interleavedVertices.push_back(
            vertex.position.x
        );

        interleavedVertices.push_back(
            vertex.position.y
        );

        interleavedVertices.push_back(
            vertex.position.z
        );

        // Normal
        interleavedVertices.push_back(
            vertex.normal.x
        );

        interleavedVertices.push_back(
            vertex.normal.y
        );

        interleavedVertices.push_back(
            vertex.normal.z
        );

        // UV
        interleavedVertices.push_back(
            vertex.uv.x
        );

        interleavedVertices.push_back(
            vertex.uv.y
        );
    }

    // ------------------------------------------------
    // Create VAO
    // ------------------------------------------------

    glGenVertexArrays(
        1,
        &m_vao
    );

    glBindVertexArray(
        m_vao
    );

    // ------------------------------------------------
    // Create VBO
    // ------------------------------------------------

    glGenBuffers(
        1,
        &m_vbo
    );

    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_vbo
    );

    glBufferData(
        GL_ARRAY_BUFFER,
        interleavedVertices.size() *
        sizeof(float),
        interleavedVertices.data(),
        GL_STATIC_DRAW
    );

    // ------------------------------------------------
    // Create EBO if mesh is indexed
    // ------------------------------------------------

    if (m_meshData.HasIndices())
    {
        glGenBuffers(
            1,
            &m_ebo
        );

        glBindBuffer(
            GL_ELEMENT_ARRAY_BUFFER,
            m_ebo
        );

        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            m_meshData.indices.size() *
            sizeof(std::uint32_t),
            m_meshData.indices.data(),
            GL_STATIC_DRAW
        );

        m_useIndices = true;

        m_indexCount =
            static_cast<GLsizei>(
                m_meshData.indices.size()
                );
    }
    else
    {
        m_useIndices = false;
        m_indexCount = 0;
    }

    // ------------------------------------------------
    // Vertex layout
    //
    // XYZ XYZ UV
    // 3 + 3 + 2 = 8 floats
    // ------------------------------------------------

    constexpr GLsizei stride =
        8 * sizeof(float);

    // Position
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        stride,
        reinterpret_cast<void*>(0)
    );

    // Normal
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        stride,
        reinterpret_cast<void*>(
            3 * sizeof(float)
            )
    );

    // UV
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        stride,
        reinterpret_cast<void*>(
            6 * sizeof(float)
            )
    );

    // ------------------------------------------------
    // Counts
    // ------------------------------------------------

    m_vertexCount =
        static_cast<GLsizei>(
            m_meshData.vertices.size()
            );

    // IMPORTANT:
    // Do not unbind the EBO before the VAO.
    // The EBO binding belongs to the VAO.

    glBindVertexArray(0);

    glBindBuffer(
        GL_ARRAY_BUFFER,
        0
    );

    // ------------------------------------------------
    // Validation
    // ------------------------------------------------

    const bool valid =
        m_vao != 0 &&
        m_vbo != 0 &&
        m_vertexCount > 0 &&
        (
            !m_useIndices ||
            (
                m_ebo != 0 &&
                m_indexCount > 0
                )
            );

    if (!valid)
    {
        BOX_LOG_ERROR(
            "Entity::CreateBuffersFromMeshData failed"
        );

        Destroy();

        return false;
    }

    BOX_LOG_INFO(
        "GPU mesh created: "
        << m_vertexCount
        << " vertices, "
        << m_indexCount
        << " indices"
    );

    return true;
}




// ############################################ Normals ##########################################

void Entity::RecalculateNormals()
{
    if (m_meshData.vertices.empty())
    {
        return;
    }

    if (!m_meshData.HasIndices())
    {
        for (std::size_t index = 0;
            index + 2 < m_meshData.vertices.size();
            index += 3)
        {
            MeshVertex& a =
                m_meshData.vertices[index];

            MeshVertex& b =
                m_meshData.vertices[index + 1];

            MeshVertex& c =
                m_meshData.vertices[index + 2];

            const glm::vec3 edge1 =
                b.position - a.position;

            const glm::vec3 edge2 =
                c.position - a.position;

            const glm::vec3 crossProduct =
                glm::cross(
                    edge1,
                    edge2
                );

            const float length =
                glm::length(crossProduct);

            if (length <= 0.000001f)
            {
                continue;
            }

            const glm::vec3 normal =
                crossProduct / length;

            a.normal = normal;
            b.normal = normal;
            c.normal = normal;
        }

        return;
    }

    /*
     * Indexed mesh version.
     */
    for (std::size_t index = 0;
        index + 2 < m_meshData.indices.size();
        index += 3)
    {
        const std::uint32_t ia =
            m_meshData.indices[index];

        const std::uint32_t ib =
            m_meshData.indices[index + 1];

        const std::uint32_t ic =
            m_meshData.indices[index + 2];

        if (ia >= m_meshData.vertices.size() ||
            ib >= m_meshData.vertices.size() ||
            ic >= m_meshData.vertices.size())
        {
            continue;
        }

        MeshVertex& a =
            m_meshData.vertices[ia];

        MeshVertex& b =
            m_meshData.vertices[ib];

        MeshVertex& c =
            m_meshData.vertices[ic];

        const glm::vec3 edge1 =
            b.position - a.position;

        const glm::vec3 edge2 =
            c.position - a.position;

        const glm::vec3 crossProduct =
            glm::cross(
                edge1,
                edge2
            );

        const float length =
            glm::length(crossProduct);

        if (length <= 0.000001f)
        {
            continue;
        }

        const glm::vec3 normal =
            crossProduct / length;

        a.normal = normal;
        b.normal = normal;
        c.normal = normal;
    }
}

// ############################################################################################
// ############################################### Rendering ##################################
// ############################################################################################

void Entity::RenderInternal(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPosition)
{
    if (!m_visible || m_vao == 0)
    {
        return;
    }

    shader.Use();

    // --------------------------------
    // Transform
    // --------------------------------

    shader.setMat4(
        "uModel",
        CalculateModelMatrix()
    );

    shader.setMat4(
        "uView",
        view
    );

    shader.setMat4(
        "uProjection",
        projection
    );

    shader.setVec3(
        "uCameraPosition",
        cameraPosition
    );

    // --------------------------------
    // Material
    // --------------------------------

    glm::vec4 renderColor =
        m_material.GetBaseColor();

    renderColor.a =
        m_material.GetAlpha();

    shader.setVec4(
        "uBaseColor",
        renderColor
    );

    shader.SetUniformFloat(
        "uMetallic",
        m_material.GetMetallic()
    );

    shader.SetUniformFloat(
        "uRoughness",
        m_material.GetRoughness()
    );

    // --------------------------------
    // Base-colour texture
    // --------------------------------

    const GLuint textureID =
        m_material.GetBaseColorTexture();

	// Check if the material uses a base color or has a texture
    const bool useTexture =
        m_material.UsesBaseColorTexture();

    shader.SetUniformInt(
        "uUseBaseColorTexture",
        useTexture ? 1 : 0
    );

    shader.SetUniformInt(
        "uBaseColorTexture",
        0
    );

    glActiveTexture(
        GL_TEXTURE0
    );

    glBindTexture(
        GL_TEXTURE_2D,
        useTexture ? textureID : 0
    );

    

    // --------------------------------
    // Lighting
    // --------------------------------

    shader.setVec3(
        "uLightPosition",
        glm::vec3(
            3.0f,
            4.0f,
            3.0f
        )
    );

    shader.setVec3(
        "uLightColor",
        glm::vec3(1.0f)
    );

    // Cube, sphere, or any other mesh
    DrawMesh();

    glBindTexture(GL_TEXTURE_2D, 0);

}

// New RenderPreview function that takes Shader and Camera objects as parameters
void Entity::RenderScene(const Shader& shader, const Camera& camera, float aspectRatio)
{
	
    RenderInternal(shader, camera.GetViewMatrix(), camera.GetProjectionMatrix(aspectRatio),
        camera.Position);   

}
// revised RenderPreview function that takes view and projection matrices as parameters
void Entity::RenderPreview(const Shader& shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPosition)
{

    RenderInternal(shader, view, projection, cameraPosition);

}

bool Entity::CreateSphere(int sectors, int stacks)
{
    Destroy();

    if (sectors < 3)
    {
        sectors = 3;
    }

    if (stacks < 2)
    {
        stacks = 2;
    }

    constexpr float radius = 0.5f;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    const float sectorStep =
        glm::two_pi<float>() /
        static_cast<float>(sectors);

    const float stackStep =
        glm::pi<float>() /
        static_cast<float>(stacks);

    // Build sphere vertices.
    for (unsigned int stack = 0;
        stack <= stacks;
        ++stack)
    {
        const float stackAngle =
            glm::half_pi<float>() -
            static_cast<float>(stack) *
            stackStep;

        const float xy =
            radius *
            std::cos(stackAngle);

        const float y =
            radius *
            std::sin(stackAngle);

        for (unsigned int sector = 0;
            sector <= sectors;
            ++sector)
        {
            const float sectorAngle =
                static_cast<float>(sector) *
                sectorStep;

            const float x =
                xy *
                std::cos(sectorAngle);

            const float z =
                xy *
                std::sin(sectorAngle);

            const float u =
                static_cast<float>(sector) /
                static_cast<float>(sectors);

            const float v =
                static_cast<float>(stack) /
                static_cast<float>(stacks);

            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normal
            const glm::vec3 normal =
                glm::normalize(
                    glm::vec3(x, y, z)
                );

            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);

            // UV
            vertices.push_back(u);
            vertices.push_back(v);
        }
        
    }

    // Build triangle indices.
    for (unsigned int stack = 0;
        stack < stacks;
        ++stack)
    {
        unsigned int first =
            stack * (sectors + 1);

        unsigned int second =
            first + sectors + 1;

        for (unsigned int sector = 0;
            sector < sectors;
            ++sector,
            ++first,
            ++second)
        {
            // Do not create triangles above
            // the upper pole.
            if (stack != 0)
            {
                indices.push_back(first);
                indices.push_back(first + 1);
                indices.push_back(second);
            }
            

            // Do not create triangles below
            // the lower pole.
            if (stack != stacks - 1)
            {
                indices.push_back(first + 1);
                indices.push_back(second + 1);
                indices.push_back(second);
            }
            
        }
    }

	// ############################################### Mesh Data ####################################################
    m_meshData.vertices.clear();
    m_meshData.indices.clear();

    constexpr std::size_t floatsPerVertex = 8;

    m_meshData.vertices.reserve(
        vertices.size() /
        floatsPerVertex
    );

    for (std::size_t index = 0;
        index < vertices.size();
        index += floatsPerVertex)
    {
        MeshVertex vertex;

        vertex.position =
            glm::vec3(
                vertices[index + 0],
                vertices[index + 1],
                vertices[index + 2]
            );

        vertex.normal =
            glm::vec3(
                vertices[index + 3],
                vertices[index + 4],
                vertices[index + 5]
            );

        vertex.uv =
            glm::vec2(
                vertices[index + 6],
                vertices[index + 7]
            );

        m_meshData.vertices.push_back(
            vertex
        );
    }

    m_meshData.indices.assign(
        indices.begin(),
        indices.end()
    );


    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_vbo
    );

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(
        GL_ELEMENT_ARRAY_BUFFER,
        m_ebo
    );

    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices.size() *
        sizeof(unsigned int),
        indices.data(),
        GL_STATIC_DRAW
    );

    // Position: XYZ
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(float),
        reinterpret_cast<void*>(0)
    );

    // Normal: XYZ
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(float),
        reinterpret_cast<void*>(
            3 * sizeof(float)
            )
    );

    // UV
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(float),
        reinterpret_cast<void*>(
            6 * sizeof(float)
            ));

    glBindVertexArray(0);

    m_vertexCount =
        static_cast<GLsizei>(
            m_meshData.vertices.size()
            );
   
    m_indexCount =
        static_cast<GLsizei>(
            m_meshData.indices.size()
            );

   

    m_useIndices = true;

    // Local bounds for picking.
    m_aabbMin =
        glm::vec3(-radius);

    m_aabbMax =
        glm::vec3(radius);

    const bool valid =
        m_vao != 0 &&
        m_vbo != 0 &&
        m_ebo != 0 &&
        m_indexCount > 0;

    if (!valid)
    {
        BOX_LOG_ERROR(
            "Entity::CreateSphere failed for entity "
            << m_name
        );

        Destroy();
        return false;
    }

    BOX_LOG_INFO(
        "Created sphere entity: "
        << m_name
        << " ID=" << m_id
        << " Vertices="
        << m_meshData.vertices.size()
        << " Indices="
        << m_meshData.indices.size()
    );

    return true;
}

glm::mat4 Entity::GetModelMatrix() const
{
    glm::mat4 model =
        glm::mat4(1.0f);

    model = glm::translate(
        model,
        m_position
    );

    model = glm::rotate(
        model,
        glm::radians(m_rotation.x),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    model = glm::rotate(
        model,
        glm::radians(m_rotation.y),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    model = glm::rotate(
        model,
        glm::radians(m_rotation.z),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );

    model = glm::scale(
        model,
        m_scale
    );

    return model;
}

const glm::vec3& Entity::GetAABBMin() const
{
    return m_aabbMin;
}

const glm::vec3& Entity::GetAABBMax() const
{
    return m_aabbMax;
}

glm::mat4 Entity::CalculateModelMatrix() const
{
    glm::mat4 model(1.0f);

    model = glm::translate(
        model,
        m_position
    );

    model = glm::rotate(
        model,
        glm::radians(m_rotation.x),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    model = glm::rotate(
        model,
        glm::radians(m_rotation.y),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    model = glm::rotate(
        model,
        glm::radians(m_rotation.z),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );

    model = glm::scale(
        model,
        m_scale
    );

    return model;
}

void Entity::Destroy()
{
    if (m_ebo != 0)
    {
        glDeleteBuffers(1, &m_ebo);

        m_ebo = 0;
    }

    if (m_vbo != 0)
    {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }

    if (m_vao != 0)
    {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }

    m_vertexCount = 0;
    m_indexCount = 0;
    m_useIndices = false;

    m_meshData.vertices.clear();
    m_meshData.indices.clear();
}

