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

#include <mesh\modifiers\FaceExtrude.h>
#include <mesh\modifiers\FaceInset.h>
#include <mesh/modifiers/LoopCut.h>
#include <mesh/modifiers/Bevel.h>

Entity::Entity(
    int id,
    const std::string& name)
    : m_id(id),
    m_name(name)
{
    // Slot 0 is always the default material.
    m_materialSlots.emplace_back();

    m_materialSlots[0].SetName("Default");
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
        BOX_LOG_ERROR(
            "Entity::CreateFromMeshData received invalid mesh data"
        );

        return false;
    }

    m_meshData =
        meshData;

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
        m_meshData.vertices.size() *
        sizeof(MeshVertex),
        m_meshData.vertices.data(),
        GL_STATIC_DRAW
    );

    // ------------------------------------------------
    // Create EBO
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

        m_useIndices =
            true;

        m_indexCount =
            static_cast<GLsizei>(
                m_meshData.indices.size()
                );
    }
    else
    {
        m_useIndices =
            false;

        m_indexCount =
            0;
    }


    // ------------------------------------------------
    // Vertex layout
    // ------------------------------------------------

    // ------------------------------------------------
    // Position
    // ------------------------------------------------

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(MeshVertex),
        reinterpret_cast<void*>(
            offsetof(
                MeshVertex,
                position
            )
            )
    );

    // ------------------------------------------------
    // Normal
    // ------------------------------------------------

    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(MeshVertex),
        reinterpret_cast<void*>(
            offsetof(
                MeshVertex,
                normal
            )
            )
    );


    // UV
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(MeshVertex),
        reinterpret_cast<void*>(
            offsetof(
                MeshVertex,
                uv
            )
            )
    );

    // ------------------------------------------------
    // Material index
    // ------------------------------------------------

    glEnableVertexAttribArray(3);

    glVertexAttribIPointer(
        3,
        1,
        GL_UNSIGNED_INT,
        sizeof(MeshVertex),
        reinterpret_cast<void*>(
            offsetof(
                MeshVertex,
                materialIndex
            )
            )
    );

    // ------------------------------------------------
	// Tangent
    // ------------------------------------------------

	glEnableVertexAttribArray(4);

    glVertexAttribPointer(
        4,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(MeshVertex),
        reinterpret_cast<void*>(
            offsetof(
                MeshVertex,
                tangent
            )
            )
	);

    // ------------------------------------------------
    // Counts
    // ------------------------------------------------

    m_vertexCount =
        static_cast<GLsizei>(
            m_meshData.vertices.size()
            );


    glBindVertexArray(
        0
    );

    glBindBuffer(
        GL_ARRAY_BUFFER,
        0
    );


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
        BOX_LOG_ERROR("Entity::CreateFromMeshData failed");

        Destroy();

        return false;
    }

    BOX_LOG_INFO(
        "Created entity from mesh data: "
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

	m_primitiveType = EntityPrimitiveType::Cube;

    m_baseEditableMesh = m_editableMesh;

    if (!m_editableMesh.BuildRenderMesh(m_meshData))
    {
        return false;
    }

    if (!CreateBuffersFromMeshData())
    {
        return false;
    }

    m_aabbMin = glm::vec3(-0.5f);

    m_aabbMax = glm::vec3(0.5f);

    

    return true;

}

bool Entity::CreatePlane()
{
    Destroy();

    if (!m_editableMesh.CreatePlane())
    {
        return false;
    }

    m_primitiveType = EntityPrimitiveType::Plane;

    m_baseEditableMesh = m_editableMesh;

    if (!m_editableMesh.BuildRenderMesh(m_meshData))
    {
        return false;
    }

    if (!CreateBuffersFromMeshData())
    {
        return false;
    }

    m_aabbMin = glm::vec3(-0.5f);

    m_aabbMax = glm::vec3(0.5f);

    return true;

}

bool Entity::CreateCylinder(int sectors, int stacks, float radius, float height)
{
    Destroy();
    if (!m_editableMesh.CreateCylinder(sectors, stacks, radius, height))
    {
        return false;
    }

    m_cylinderSectors = sectors;
    m_cylinderStacks = stacks;
    m_cylinderRadius = radius;
    m_cylinderHeight = height;

    m_primitiveType = EntityPrimitiveType::Cylinder;

    m_baseEditableMesh = m_editableMesh;
    if (!m_editableMesh.BuildRenderMesh(m_meshData))
    {
        return false;
    }
    if (!CreateBuffersFromMeshData())
    {
        return false;
    }
    m_aabbMin = glm::vec3(-radius, -height * 0.5f, -radius);
    m_aabbMax = glm::vec3(radius, height * 0.5f, radius);
	return true;
}

bool Entity::UpdateCylinder()
{
    // -----------------------------------------
    // Recreate editable cylinder using the
    // currently stored primitive properties.
    // -----------------------------------------

    if (!m_editableMesh.CreateCylinder(
        m_cylinderSectors,
        m_cylinderStacks,
        m_cylinderRadius,
        m_cylinderHeight))
    {
        return false;
    }


    // This is now the new base primitive.
    m_baseEditableMesh =
        m_editableMesh;


    if (!m_editableMesh.BuildRenderMesh(
        m_meshData))
    {
        return false;
    }


    // Destroy only existing GPU buffers before
    // recreating them.
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


    if (!CreateBuffersFromMeshData())
    {
        return false;
    }


    m_aabbMin =
        glm::vec3(
            -m_cylinderRadius,
            -m_cylinderHeight * 0.5f,
            -m_cylinderRadius
        );

    m_aabbMax =
        glm::vec3(
            m_cylinderRadius,
            m_cylinderHeight * 0.5f,
            m_cylinderRadius
        );


    return true;
}

// Create a Sphere primitive

bool Entity::CreateSphere(
    int sectors,
    int stacks)
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


    // ------------------------------------------------
    // Create editable sphere topology
    // ------------------------------------------------

    if (!m_editableMesh.CreateSphere(
        sectors,
        stacks))
    {
        BOX_LOG_ERROR(
            "Entity::CreateSphere: "
            "Failed to create editable sphere"
        );

        return false;
    }


    m_primitiveType =
        EntityPrimitiveType::Sphere;


    // Keep untouched primitive topology
    // for the modifier system.
    m_baseEditableMesh =
        m_editableMesh;


    // ------------------------------------------------
    // Build render mesh
    //
    // This also generates:
    //
    // Position
    // Normal
    // UV
    // Material index
    // Tangent
    // ------------------------------------------------

    if (!m_editableMesh.BuildRenderMesh(
        m_meshData))
    {
        BOX_LOG_ERROR(
            "Entity::CreateSphere: "
            "Failed to build render mesh"
        );

        return false;
    }


    // ------------------------------------------------
    // Upload to GPU
    // ------------------------------------------------

    if (!CreateBuffersFromMeshData())
    {
        BOX_LOG_ERROR(
            "Entity::CreateSphere: "
            "Failed to create GPU buffers"
        );

        return false;
    }


    // ------------------------------------------------
    // Bounds
    // ------------------------------------------------

    constexpr float radius =
        0.5f;


    m_aabbMin =
        glm::vec3(-radius);

    m_aabbMax =
        glm::vec3(radius);


    BOX_LOG_INFO(
        "Created editable sphere entity: "
        << m_name
        << " ID="
        << m_id
        << " Vertices="
        << m_editableMesh.GetVertexCount()
        << " Edges="
        << m_editableMesh.GetEdgeCount()
        << " Faces="
        << m_editableMesh.GetFaceCount()
    );


    return true;
}

bool Entity::UpdateSphere()
{
    if (!m_editableMesh.CreateSphere(
        m_sphereSectors,
        m_sphereStacks
        ))
    {
        return false;
    }


    // This is now the new base primitive.
    m_baseEditableMesh =  m_editableMesh;


    if (!m_editableMesh.BuildRenderMesh(m_meshData))
    {
        return false;
    }


    // Destroy only existing GPU buffers before
    // recreating them.
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


    if (!CreateBuffersFromMeshData())
    {
        return false;
    }


    constexpr float radius = 0.5f;


    m_aabbMin =
        glm::vec3(-radius);

    m_aabbMax =
        glm::vec3(radius);

    return true;

}

// Create a Pyramid primitive
bool Entity::CreatePyramid()
{
    Destroy();

    if (!m_editableMesh.CreatePyramid())
    {
        return false;
    }

    m_primitiveType = EntityPrimitiveType::Pyramid;

    m_baseEditableMesh = m_editableMesh;

    if (!m_editableMesh.BuildRenderMesh(m_meshData))
    {
        return false;
    }

    if (!CreateBuffersFromMeshData())
    {
        return false;
    }

    m_aabbMin = glm::vec3(-0.5f);

    m_aabbMax = glm::vec3(0.5f);

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
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
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

    if (m_vbo == 0 ||
        m_meshData.vertices.empty())
    {
        return false;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferSubData(GL_ARRAY_BUFFER, 0, m_meshData.vertices.size() * sizeof(MeshVertex), m_meshData.vertices.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;


    /*if (m_vbo == 0 || m_meshData.vertices.empty())
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

    return true;*/
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
// single edge selection
void Entity::SelectEdge(std::size_t index)
{
    
    m_selectedEdges.clear();

    m_selectedEdges.push_back(index);
}
// multi edge selection
void Entity::AddSelectedEdge(
    std::size_t index)
{
    if (index >=
        m_editableMesh.GetEdgeCount())
    {
        return;
    }


    // Don't add the same edge twice.
    if (std::find(
        m_selectedEdges.begin(),
        m_selectedEdges.end(),
        index) ==
        m_selectedEdges.end())
    {
        m_selectedEdges.push_back(
            index
        );
    }
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

void Entity::AddSelectedFace(std::size_t index)
{
    if (index >= m_editableMesh.GetFaceCount())
    {
        return;
    }
    // Don't add the same face twice.
    if (std::find(
        m_selectedFaces.begin(),
        m_selectedFaces.end(),
        index) ==
        m_selectedFaces.end())
    {
        m_selectedFaces.push_back(
            index
        );
	}

}
// shiftHeld
void Entity::ToggleSelectedFace(std::size_t index)
{
    if (index >= m_editableMesh.GetFaceCount())
    {
        return;
    }
    auto it = std::find(
        m_selectedFaces.begin(),
        m_selectedFaces.end(),
        index
    );
    if (it != m_selectedFaces.end())
    {
        // Face is already selected, remove it
        m_selectedFaces.erase(it);
    }
    else
    {
        // Face is not selected, add it
        m_selectedFaces.push_back(index);
    }
}
// Ctrl + A select all faces
void Entity::SelectAllFaces()
{
    m_selectedFaces.clear();

    const std::size_t faceCount =
        m_editableMesh.GetFaceCount();

    m_selectedFaces.reserve(
        faceCount
    );

    for (std::size_t index = 0;
        index < faceCount;
        ++index)
    {
        m_selectedFaces.push_back(
            index
        );
    }
}

bool Entity::SetSelectedFacesMaterial(
    std::size_t materialIndex)
{
    if (materialIndex >=
        m_materialSlots.size())
    {
        return false;
    }

    if (m_selectedFaces.empty())
    {
        return false;
    }

    for (std::size_t faceIndex :
    m_selectedFaces)
    {
        if (faceIndex >=
            m_editableMesh.GetFaceCount())
        {
            continue;
        }

        m_editableMesh
            .GetFace(faceIndex)
            .materialIndex =
            materialIndex;
    }

    return true;
}








// ############################################# Mesh Data ##########################################
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
    // Upload MeshVertex structs directly.
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
        m_meshData.vertices.size() *
        sizeof(MeshVertex),
        m_meshData.vertices.data(),
        GL_STATIC_DRAW
    );


    // ------------------------------------------------
    // Create EBO
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

        m_useIndices =
            true;

        m_indexCount =
            static_cast<GLsizei>(
                m_meshData.indices.size()
                );
    }
    else
    {
        m_useIndices =
            false;

        m_indexCount =
            0;
    }


    // ------------------------------------------------
    // Vertex layout
    // ------------------------------------------------


    // Position
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(MeshVertex),
        reinterpret_cast<void*>(
            offsetof(
                MeshVertex,
                position
            )
            )
    );


    // Normal
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(MeshVertex),
        reinterpret_cast<void*>(
            offsetof(
                MeshVertex,
                normal
            )
            )
    );


    // UV
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(MeshVertex),
        reinterpret_cast<void*>(
            offsetof(
                MeshVertex,
                uv
            )
            )
    );


    // Material index
    glEnableVertexAttribArray(3);

    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(MeshVertex), reinterpret_cast<void*>(offsetof(
                MeshVertex, materialIndex)));

	// Tangent
    glEnableVertexAttribArray(4);

    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<void*>(offsetof(MeshVertex, tangent)));
    


    // ------------------------------------------------
    // Counts
    // ------------------------------------------------

    m_vertexCount =
        static_cast<GLsizei>(
            m_meshData.vertices.size()
            );


    glBindVertexArray(
        0
    );

    glBindBuffer(
        GL_ARRAY_BUFFER,
        0
    );


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

// #####################################################################################################################
// ################################################### Last Extrude ####################################################
// #####################################################################################################################
void Entity::SetLastExtrude(std::size_t faceIndex,
    ModifierAxis axis,
    float amount,
    const MeshEditing& meshBeforeExtrude)
{
    m_lastExtrude.faceIndex =
        faceIndex;

    m_lastExtrude.axis =
        axis;

    m_lastExtrude.extrudeAmount =
        amount;

    m_lastExtrudeBaseMesh =
        meshBeforeExtrude;

    m_hasLastExtrude = true;

	m_lastOperationType = LastOperationType::Extrude; // so we know the last operation was an extrusion
}

bool Entity::UpdateLastExtrude(
    ModifierAxis axis,
    float amount)
{
    if (!m_hasLastExtrude)
    {
        return false;
    }

    // Restore the mesh from immediately
    // before the last extrusion.
    m_editableMesh =
        m_lastExtrudeBaseMesh;

    glm::vec3 direction(0.0f);

    switch (axis)
    {
    case ModifierAxis::X:
        direction =
            glm::vec3(
                1.0f,
                0.0f,
                0.0f
            );
        break;

    case ModifierAxis::Y:
        direction =
            glm::vec3(
                0.0f,
                1.0f,
                0.0f
            );
        break;

    case ModifierAxis::Z:
        direction =
            glm::vec3(
                0.0f,
                0.0f,
                1.0f
            );
        break;
    }

    FaceExtrude extrude;

    if (!extrude.Use(
        m_editableMesh,
        m_lastExtrude.faceIndex,
        direction,
        amount))
    {
        return false;
    }

    MeshData renderMesh;

    if (!m_editableMesh.BuildRenderMesh(
        renderMesh))
    {
        return false;
    }

    if (!CreateFromMeshData(
        renderMesh))
    {
        return false;
    }

    m_lastExtrude.axis =
        axis;

    m_lastExtrude.extrudeAmount =
        amount;

    return true;
}
// ###############################################################################################################
// ############################################ Inset ##########################################################
// ###############################################################################################################
void Entity::SetLastInset(std::size_t faceIndex, float amount, const MeshEditing& meshBeforeInset)
{
    m_lastInset.faceIndex = faceIndex;

    m_lastInset.insetAmount = amount;

    m_lastInsetBaseMesh = meshBeforeInset;

    m_hasLastInset = true;

	m_lastOperationType = LastOperationType::Inset; // so we know the last operation was an inset
}

bool Entity::UpdateLastInset(float amount)
{
    if (!m_hasLastInset)
    {
        return false;
    }

    // Restore mesh immediately before
    // the last inset operation.
    m_editableMesh =
        m_lastInsetBaseMesh;

    FaceInset inset;

    if (!inset.Use(
        m_editableMesh,
        m_lastInset.faceIndex,
        amount))
    {
        BOX_LOG_ERROR(
            "Failed to update last inset"
        );

        return false;
    }

    MeshData renderMesh;

    if (!m_editableMesh.BuildRenderMesh(
        renderMesh))
    {
        return false;
    }

    if (!CreateFromMeshData(
        renderMesh))
    {
        return false;
    }

    m_lastInset.insetAmount =
        amount;

    return true;
}

// ###############################################################################################################
// ############################################ Loop Cut #########################################################
// ###############################################################################################################
void Entity::SetLastLoopCut(std::size_t edgeIndex, float amount,
    const MeshEditing& meshBeforeLoopCut)
{
    m_lastLoopCut.edgeIndex = edgeIndex;
    m_lastLoopCut.cutAmount = amount;
    m_lastLoopCutBaseMesh = meshBeforeLoopCut;
    m_hasLastLoopCut = true;
	m_lastOperationType = LastOperationType::LoopCut; // so we know the last operation was a loop cut
}

bool Entity::UpdateLastLoopCut(float amount)
{
    if (!m_hasLastLoopCut)
    {
        return false;
    }

    // Restore the topology to exactly
    // how it was before this Loop Cut.
    m_editableMesh =
        m_lastLoopCutBaseMesh;

    LoopCut loopCut;

    if (!loopCut.Use(
        m_editableMesh,
        m_lastLoopCut.edgeIndex,
        amount))
    {
        BOX_LOG_ERROR(
            "UpdateLastLoopCut: "
            "Loop Cut failed"
        );

        return false;
    }

    MeshData renderMesh;

    if (!m_editableMesh.BuildRenderMesh(renderMesh))
    {
        BOX_LOG_ERROR(
            "UpdateLastLoopCut: "
            "Failed to build render mesh"
        );

        return false;
    }

    if (!CreateFromMeshData(
        renderMesh))
    {
        BOX_LOG_ERROR(
            "UpdateLastLoopCut: "
            "Failed to update entity mesh"
        );

        return false;
    }

    m_lastLoopCut.cutAmount =
        amount;

    return true;
}
// ###############################################################################################################
// ############################################ Bevel ############################################################
// ###############################################################################################################

void Entity::SetLastBevel(std::size_t edgeIndex, float Width, int Segments, float Profile,
    const MeshEditing& meshBeforeBevel)
{
    m_lastBevel.edgeIndex = edgeIndex;
    m_lastBevel.segments = Segments;
    m_lastBevel.width = Width;
    m_lastBevel.profile = Profile;
    m_lastBevelBaseMesh = meshBeforeBevel;
    m_hasLastBevel = true;
	m_lastOperationType = LastOperationType::Bevel; // so we know the last operation was a bevel
}

//bool Entity::UpdateLastBevel(int segments, float width, float profile)
bool Entity::UpdateLastBevel(float width, int segments, float profile)
{
    if (!m_hasLastBevel)
    {
        return false;
    }
    // Restore the mesh to exactly how it was
    // before the last bevel operation.
    m_editableMesh = m_lastBevelBaseMesh;
    Bevel bevel;
    if (!bevel.Use(
        m_editableMesh,
        m_lastBevel.edgeIndex,
        width, segments, profile))
        //segments,
        //profile))
    {
        BOX_LOG_ERROR(
            "UpdateLastBevel: "
            "Bevel failed"
        );
        return false;
    }
    MeshData renderMesh;
    if (!m_editableMesh.BuildRenderMesh(renderMesh))
    {
        BOX_LOG_ERROR(
            "UpdateLastBevel: "
            "Failed to build render mesh"
        );
        return false;
    }
    if (!CreateFromMeshData(
        renderMesh))
    {
        BOX_LOG_ERROR(
            "UpdateLastBevel: "
            "Failed to update entity mesh"
        );
        return false;
    }
    m_lastBevel.width =
        width;
    m_lastBevel.segments =
        segments;
    m_lastBevel.profile =
        profile;

	return true;
}


// ###############################################################################################################
// ############################################ Normals ##########################################################
// ###############################################################################################################

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

// ###############################################################################################################
// ########################################## Rebuild Modifiers ##################################################
// ###############################################################################################################
bool Entity::RebuildModifiers()
{
    // Always restart from untouched topology.
    m_editableMesh = m_baseEditableMesh;

    FaceExtrude extrude;

	// I would like to test which modifiers is enabled.
    

    for (const ModifierData& modifier : m_modifiers)
    {
        if (!modifier.modEnabled)
        {
            continue;
        }

        switch (modifier.type)
        {
        case ModifierType::Extrude:
        {
            glm::vec3 direction(0.0f);

            switch (
                modifier.extrude.axis)
            {
            case ModifierAxis::X:
                direction =
                    glm::vec3(1, 0, 0);
                break;

            case ModifierAxis::Y:
                direction =
                    glm::vec3(0, 1, 0);
                break;

            case ModifierAxis::Z:
                direction =
                    glm::vec3(0, 0, 1);
                break;
            }

            if (!extrude.Use(
                m_editableMesh,
                modifier.extrude.faceIndex,
                direction,
                modifier.extrude.extrudeAmount))
            {
                return false;
            }

            break;
        }

        default:
            break;
        }
    }

    MeshData renderMesh;

    if (!m_editableMesh.BuildRenderMesh(
        renderMesh))
    {
        return false;
    }

    return CreateFromMeshData(
        renderMesh
    );
}



// ########################################################################################################
// ############################################### Rendering ##############################################
// ########################################################################################################

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

    
    // --------------------------------
    // Material Slot Color
    // --------------------------------
    constexpr std::size_t
        MaxMaterialSlots = 8;


    for (std::size_t index = 0;
        index < MaxMaterialSlots;
        ++index)
    {
        glm::vec4 color(1.0f);

        GLuint textureID = 0;

		GLuint normalTextureID = 0; // tangent space normal map texture ID

        float metallic = 0.0f;
        float roughness = 0.5f;
		// --------------------------------
        // Emission
		// --------------------------------

        glm::vec3 emissionColor(0.0f);
        float emissionStrength = 0.0f;

        bool useTexture = false;

		bool useNormalTexture = false; // tangent space normal map usage flag

		float normalStrength = 1.0f; // tangent space normal map strength


        // --------------------------------
        // Get material slot information.
        // --------------------------------

        if (index < m_materialSlots.size())
        {
            const Material& material =
                m_materialSlots[index];

            color =
                material.GetBaseColor();

            textureID =
                material.GetBaseColorTexture();

            useTexture =
                material.UsesBaseColorTexture();

            metallic =
                material.GetMetallic();

            roughness =
                material.GetRoughness();

            emissionColor =
                material.GetEmissionColor();

            emissionStrength =
                material.GetEmissionStrength();

			// Normal map tangent space texture information
            normalTextureID =
                material.GetNormalTexture();

            useNormalTexture =
                material.UsesNormalTexture();

            normalStrength =
                material.GetNormalStrength();
        }
		// --------------------------------
		// Metallic
		// --------------------------------

        const std::string metallicUniform =
            "uMaterialMetallic[" +
            std::to_string(index) +
            "]";

        shader.SetUniformFloat(
            metallicUniform.c_str(),
            metallic
        );
		// --------------------------------
		// Roughness
		// --------------------------------
        const std::string roughnessUniform =
            "uMaterialRoughness[" +
            std::to_string(index) +
            "]";

        shader.SetUniformFloat(
            roughnessUniform.c_str(),
            roughness
        );

        // ===================================
		// tangent space normal map strength
		// ===================================

        const std::string normalTextureUniform =
            "uMaterialNormalTextures[" +
            std::to_string(index) +
            "]";

        const std::string useNormalTextureUniform =
            "uMaterialUsesNormalTexture[" +
            std::to_string(index) +
            "]";

        const std::string normalStrengthUniform =
            "uMaterialNormalStrength[" +
            std::to_string(index) +
            "]";


        shader.SetUniformInt(
            normalTextureUniform.c_str(),
            static_cast<int>(index + 8)
        );

        shader.SetUniformInt(
            useNormalTextureUniform.c_str(),
            useNormalTexture ? 1 : 0
        );

        shader.SetUniformFloat(
            normalStrengthUniform.c_str(),
            normalStrength
        );
		// --------------------------------
		// Emission strength
		// --------------------------------

        const std::string
            emissionColorUniform =
            "uMaterialEmissionColor[" +
            std::to_string(index) +
            "]";

        shader.setVec3(
            emissionColorUniform.c_str(),
            emissionColor
        );

        const std::string
            emissionStrengthUniform =
            "uMaterialEmissionStrength[" +
            std::to_string(index) +
            "]";

        shader.SetUniformFloat(
            emissionStrengthUniform.c_str(),
            emissionStrength
        );


        // --------------------------------
        // Material colour
        // --------------------------------

        const std::string colorUniform =
            "uMaterialColors[" +
            std::to_string(index) +
            "]";


        shader.setVec4(
            colorUniform.c_str(),
            color
        );


        // --------------------------------
        // Does this slot use a texture?
        // --------------------------------

        const std::string useTextureUniform =
            "uMaterialUsesTexture[" +
            std::to_string(index) +
            "]";


        shader.SetUniformInt(
            useTextureUniform.c_str(),
            useTexture ? 1 : 0
        );


        // --------------------------------
        // Tell sampler which texture unit
        // belongs to this material.
        // --------------------------------

        const std::string textureUniform =
            "uMaterialTextures[" +
            std::to_string(index) +
            "]";


        shader.SetUniformInt(
            textureUniform.c_str(),
            static_cast<int>(index)
        );


        // --------------------------------
        // Bind texture.
        //
        // Slot 0 -> GL_TEXTURE0
        // Slot 1 -> GL_TEXTURE1
        // etc.
        // --------------------------------

        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(index));

        glBindTexture(GL_TEXTURE_2D, useTexture ? textureID : 0);

        // --------------------------------
        // Normal Map
        // --------------------------------

        glActiveTexture(
            GL_TEXTURE0 +
            static_cast<GLenum>(
                index + MaxMaterialSlots
                )
        );

        glBindTexture(
            GL_TEXTURE_2D,
            useNormalTexture
            ? normalTextureID
            : 0
        );


    }

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

    // Cube, plane, sphere, Cylinder or any other mesh
    DrawMesh();

	// cleanup: unbind textures to avoid affecting other objects
    for (std::size_t index = 0;
        index < MaxMaterialSlots;
        ++index)
    {
        glActiveTexture(
            GL_TEXTURE0 +
            static_cast<GLenum>(index)
        );

        glBindTexture(
            GL_TEXTURE_2D,
            0
        );


        glActiveTexture(
            GL_TEXTURE0 +
            static_cast<GLenum>(index + 8)
        );

        glBindTexture(GL_TEXTURE_2D, 0 );
    }
    
    // Restore default texture unit.
    glActiveTexture(GL_TEXTURE0);
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
// ##################################################################################################
// ########################################### Matirials ############################################
// ##################################################################################################
bool Entity::SetFaceMaterial(std::size_t faceIndex, std::size_t materialIndex)
{
    if (faceIndex >=
        m_editableMesh.GetFaceCount())
    {
        return false;
    }

    if (materialIndex >=
        m_materialSlots.size())
    {
        return false;
    }

    m_editableMesh.GetFace(faceIndex).materialIndex = materialIndex;

    return true;
}

std::size_t Entity::AddMaterialSlot(const Material& material)
{
    m_materialSlots.push_back(material);

    return m_materialSlots.size() - 1;
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

