#include "mesh/ParticleSystem/GrassBlade.h"

#include <vector>
#include <algorithm>
#include <cstddef>

#include <glm/glm.hpp>


// =====================================================
// Grass vertex structure
// =====================================================

namespace
{
    struct GrassVertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };
}


// =====================================================
// Constructor / Destructor
// =====================================================

GrassBlade::GrassBlade() = default;

GrassBlade::~GrassBlade()
{
    Destroy();
}


// =====================================================
// Create Grass Blade
// =====================================================

bool GrassBlade::CreateGrassBlade()
{
    // Remove any previously generated geometry.
    Destroy();

    constexpr int rows = 7;
    constexpr int columns = 3;

    std::vector<GrassVertex> vertices;
    std::vector<GLuint> indices;

    vertices.reserve(rows * columns);
    indices.reserve((rows - 1) * 2 * 6);

    // =================================================
    // GENERATE VERTICES
    // =================================================

    for (int row = 0; row < rows; ++row)
    {
        // 0.0 = base, 1.0 = tip
        const float t =
            static_cast<float>(row) /
            static_cast<float>(rows - 1);

        // ---------------------------------------------
        // Height
        // ---------------------------------------------

        const float y =
            m_height * t;

        // ---------------------------------------------
        // Width taper
        //
        // Leave a tiny width at the tip to avoid
        // degenerate triangles.
        // ---------------------------------------------

        const float taper =
            1.0f - 0.98f * t;

        const float currentWidth =
            m_width * taper;

        // ---------------------------------------------
        // Curvature
        //
        // Very little bending at the base.
        // Increasing towards the tip.
        // ---------------------------------------------

        const float bend =
            -m_curvature * t * t;

        // ---------------------------------------------
        // Concavity
        //
        // Stronger at the base.
        // Gradually disappears towards the tip.
        // ---------------------------------------------

        const float grooveDepth = m_concavity * (1.0f - t);
        //const float grooveDepth = m_concavity * (1.0f + t);

        // ---------------------------------------------
        // Three vertices across each row
        // ---------------------------------------------

        for (int column = 0; column < columns; ++column)
        {
            const float u =
                static_cast<float>(column) /
                static_cast<float>(columns - 1);

            // -0.5, 0.0, +0.5
            const float across =
                u - 0.5f;

            const float x =
                across * currentWidth;

            // Centre vertex is recessed.
            float groove = 0.0f;

            if (column == 1)
            {
                groove = -grooveDepth;
            }

            const float z =
                bend + groove;

            GrassVertex vertex{};

            vertex.position =
                glm::vec3(x, y, z);

            // Normals calculated after triangles.
            vertex.normal =
                glm::vec3(0.0f);

            // UV coordinates
            vertex.uv =
                glm::vec2(u, t);

            vertices.push_back(vertex);
        }
    }


    // =================================================
    // GENERATE TRIANGLE INDICES
    //
    // Each pair of neighbouring rows creates
    // two quad faces.
    //
    // Each quad becomes two triangles.
    // =================================================

    for (int row = 0; row < rows - 1; ++row)
    {
        for (int column = 0; column < columns - 1; ++column)
        {
            const GLuint bottomLeft =
                row * columns + column;

            const GLuint bottomRight =
                bottomLeft + 1;

            const GLuint topLeft =
                (row + 1) * columns + column;

            const GLuint topRight =
                topLeft + 1;

            // Triangle 1
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
            indices.push_back(topRight);

            // Triangle 2
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topLeft);
        }
    }


    // =================================================
    // CALCULATE SMOOTH VERTEX NORMALS
    // =================================================

    for (std::size_t i = 0;
        i < indices.size();
        i += 3)
    {
        const GLuint i0 = indices[i];
        const GLuint i1 = indices[i + 1];
        const GLuint i2 = indices[i + 2];

        const glm::vec3& p0 =
            vertices[i0].position;

        const glm::vec3& p1 =
            vertices[i1].position;

        const glm::vec3& p2 =
            vertices[i2].position;

        const glm::vec3 faceNormal =
            glm::cross(
                p1 - p0,
                p2 - p0
            );

        // Accumulate area-weighted face normals.
        vertices[i0].normal += faceNormal;
        vertices[i1].normal += faceNormal;
        vertices[i2].normal += faceNormal;
    }

    // Normalise the accumulated normals.
    for (GrassVertex& vertex : vertices)
    {
        const float length =
            glm::length(vertex.normal);

        if (length > 0.000001f)
        {
            vertex.normal /= length;
        }
        else
        {
            vertex.normal =
                glm::vec3(0.0f, 0.0f, 1.0f);
        }
    }


    // =================================================
    // CREATE OPENGL BUFFERS
    // =================================================

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    if (m_vao == 0 || m_vbo == 0 || m_ebo == 0)
    {
        Destroy();
        return false;
    }

    glBindVertexArray(m_vao);

    // ---------------------------------------------
    // Vertex Buffer
    // ---------------------------------------------

    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_vbo
    );

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(GrassVertex),
        vertices.data(),
        GL_STATIC_DRAW
    );

    // ---------------------------------------------
    // Index Buffer
    // ---------------------------------------------

    glBindBuffer(
        GL_ELEMENT_ARRAY_BUFFER,
        m_ebo
    );

    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(GLuint),
        indices.data(),
        GL_STATIC_DRAW
    );

    // ---------------------------------------------
    // Attribute 0: Position
    // ---------------------------------------------

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(GrassVertex),
        reinterpret_cast<void*>(
            offsetof(GrassVertex, position)));

    // =====================================================
    // INSTANCE MATRIX BUFFER
    // =====================================================

    glGenBuffers(1, &m_instanceVbo);

    if (m_instanceVbo == 0)
    {
        glBindVertexArray(0);
        Destroy();
        return false;
    }

    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_instanceVbo
    );

    // Initially allocate no instance data.
    glBufferData(
        GL_ARRAY_BUFFER,
        0,
        nullptr,
        GL_DYNAMIC_DRAW
    );

    // A mat4 occupies four vertex attribute locations.
    //
    // 3 = first column
    // 4 = second column
    // 5 = third column
    // 6 = fourth column

    for (GLuint i = 0; i < 4; ++i)
    {
        const GLuint location = 3 + i;

        glEnableVertexAttribArray(location);

        glVertexAttribPointer(
            location,
            4,
            GL_FLOAT,
            GL_FALSE,
            sizeof(glm::mat4),
            reinterpret_cast<void*>(
                sizeof(glm::vec4) * i
                )
        );

        // Advance once per instance, not per vertex.
        glVertexAttribDivisor(
            location,
            1
        );
    }

    // =====================================================
// RESTORE GRASS VERTEX BUFFER
// =====================================================

// The instance matrix attributes use m_instanceVbo.
// Normal and UV attributes must use the grass VBO.

    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_vbo
    );


    // ---------------------------------------------
    // Attribute 1: Normal
    // ---------------------------------------------

    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(GrassVertex),
        reinterpret_cast<void*>(
            offsetof(GrassVertex, normal)
            )
    );

    // ---------------------------------------------
    // Attribute 2: UV
    // ---------------------------------------------

    glEnableVertexAttribArray(2);

    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(GrassVertex),
        reinterpret_cast<void*>(
            offsetof(GrassVertex, uv)
            )
    );

    // ---------------------------------------------
    // Unbind
    // ---------------------------------------------

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_indexCount =
        static_cast<GLsizei>(indices.size());

    return m_indexCount == 72;
}


// =====================================================
// Draw Grass Blade
//
// A suitable shader must already be active.
// =====================================================

void GrassBlade::Draw() const
{
    if (m_vao == 0 || m_indexCount == 0)
        return;

    glBindVertexArray(m_vao);

    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
}


// =====================================================
// Position
// =====================================================

void GrassBlade::SetPosition(
    const glm::vec3& position)
{
    m_position = position;
}

const glm::vec3& GrassBlade::GetPosition() const
{
    return m_position;
}


// =====================================================
// Size
// =====================================================

void GrassBlade::SetSize(
    float width,
    float height)
{
    m_width = std::max(width, 0.0001f);
    m_height = std::max(height, 0.001f);
}

float GrassBlade::GetWidth() const
{
    return m_width;
}

float GrassBlade::GetHeight() const
{
    return m_height;
}


// =====================================================
// Curvature
// =====================================================

void GrassBlade::SetCurvature(float curvature)
{
    m_curvature = curvature;
}


// =====================================================
// Concavity
// =====================================================

void GrassBlade::SetConcavity(float concavity)
{
    m_concavity = std::max(concavity, 0.0f);
}


// =====================================================
// Colour
// =====================================================

const glm::vec4& GrassBlade::GetColor() const
{
    return m_color;
}

void GrassBlade::SetColor(
    const glm::vec4& color)
{
    m_color = color;
}


// =====================================================
// UPLOAD INSTANCE MATRICES TO GPU
// =====================================================

void GrassBlade::UploadInstanceMatrices(
    const std::vector<glm::mat4>& instanceMatrices
)
{
    if (m_instanceVbo == 0)
    {
        m_instanceCount = 0;
        return;
    }

    m_instanceCount = 0;

    if (instanceMatrices.empty())
        return;

    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_instanceVbo
    );

    glBufferData(
        GL_ARRAY_BUFFER,
        instanceMatrices.size() * sizeof(glm::mat4),
        instanceMatrices.data(),
        GL_DYNAMIC_DRAW
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_instanceCount =
        static_cast<GLsizei>(instanceMatrices.size());
}


// =====================================================
// DRAW CACHED GPU INSTANCES
// =====================================================

void GrassBlade::DrawCachedInstances() const
{
    if (m_vao == 0 ||
        m_instanceVbo == 0 ||
        m_indexCount == 0 ||
        m_instanceCount == 0)
    {
        return;
    }

    glBindVertexArray(m_vao);

    glDrawElementsInstanced(
        GL_TRIANGLES,
        m_indexCount,
        GL_UNSIGNED_INT,
        nullptr,
        m_instanceCount
    );

    glBindVertexArray(0);
}


// =====================================================
// ORIGINAL INSTANCED DRAWING FUNCTION
// =====================================================

void GrassBlade::DrawInstanced(
    const std::vector<glm::mat4>& instanceMatrices
)
{
    UploadInstanceMatrices(instanceMatrices);

    DrawCachedInstances();
}



//void GrassBlade::DrawInstanced(
//    const std::vector<glm::mat4>& instanceMatrices)
//{
//    if (m_vao == 0 ||
//        m_instanceVbo == 0 ||
//        m_indexCount == 0 ||
//        instanceMatrices.empty())
//    {
//        return;
//    }
//
//    // ================================================
//    // Upload instance matrices
//    // ================================================
//
//    glBindBuffer(
//        GL_ARRAY_BUFFER,
//        m_instanceVbo
//    );
//
//    glBufferData(
//        GL_ARRAY_BUFFER,
//        instanceMatrices.size() * sizeof(glm::mat4),
//        instanceMatrices.data(),
//        GL_DYNAMIC_DRAW
//    );
//
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//    // ================================================
//    // Draw all instances
//    // ================================================
//
//    glBindVertexArray(m_vao);
//
//    glDrawElementsInstanced(
//        GL_TRIANGLES,
//        m_indexCount,
//        GL_UNSIGNED_INT,
//        nullptr,
//        static_cast<GLsizei>(
//            instanceMatrices.size()
//            )
//    );
//
//    glBindVertexArray(0);
//}



// =====================================================
// Destroy OpenGL Resources
// =====================================================

void GrassBlade::Destroy()
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

    if (m_instanceVbo != 0)
    {
        glDeleteBuffers(
            1,
            &m_instanceVbo
        );

        m_instanceVbo = 0;
    }

    m_indexCount = 0;
    m_instanceCount = 0;
}
