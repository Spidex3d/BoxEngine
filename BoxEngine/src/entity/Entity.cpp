#include "entity/Entity.h"
#include <shader/Shader.h>
#include <miniBoxLog.h>
#include <stb/stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <utility>
#include <vector>
#include <cmath>

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
    : m_material(std::move(other.m_material)),
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
        return *this;

    Destroy();

    m_id = other.m_id;
    m_name = std::move(other.m_name);

    m_position = other.m_position;
    m_rotation = other.m_rotation;
    m_scale = other.m_scale;
    m_visible = other.m_visible;

    m_vao = other.m_vao;
    m_vbo = other.m_vbo;
    m_vertexCount = other.m_vertexCount;

    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_vertexCount = 0;

    return *this;
}

bool Entity::CreateCube()
{
    Destroy();

    m_useIndices = false;
    m_indexCount = 0;

    m_aabbMin = glm::vec3(-0.5f);
    m_aabbMax = glm::vec3(0.5f);

    // Position XYZ, Normal XYZ
    const float vertices[] =
    {
        // Front face +Z
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,

        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,

        // Back face -Z
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,

         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,

         // Left face -X
         -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
         -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
         -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,

         -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
         -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
         -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,

         // Right face +X
          0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
          0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
          0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,

          0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
          0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
          0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,

          // Top face +Y
          -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
           0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
           0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,

          -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
           0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
          -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,

          // Bottom face -Y
          -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
           0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
           0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,

          -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
           0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
          -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f
    };

    m_vertexCount = 36;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_STATIC_DRAW
    );

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        reinterpret_cast<void*>(0)
    );

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        reinterpret_cast<void*>(3 * sizeof(float))
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    const bool valid = m_vao != 0 && m_vbo != 0;

    if (!valid)
    {
        BOX_LOG_ERROR(
            "Entity::CreateCube failed for entity "
            << m_name
        );

        Destroy();
        return false;
    }

    BOX_LOG_INFO(
        "Created cube entity: "
        << m_name
        << " ID=" << m_id
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

   /* glBindVertexArray(m_vao);

    glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);

    glBindVertexArray(0); */
}


void Entity::Render(
    const Shader& shader,
    const glm::mat4& view,
    const glm::mat4& projection)
{
    if (!m_visible || m_vao == 0)
        return;

    const glm::mat4 model = GetModelMatrix(); // piking

    shader.Use();

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

    glm::vec4 renderColor =
        m_material.GetBaseColor();

    renderColor.a =
        m_material.GetAlpha();

    shader.setVec4(
        "uBaseColor",
        renderColor
    );

    /*shader.setVec4(
        "uBaseColor",
        m_material.GetBaseColor()
    );*/

    

	DrawMesh();

    
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
                indices.push_back(second);
                indices.push_back(first + 1);
            }

            // Do not create triangles below
            // the lower pole.
            if (stack != stacks - 1)
            {
                indices.push_back(first + 1);
                indices.push_back(second);
                indices.push_back(second + 1);
            }
        }
    }

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
        6 * sizeof(float),
        reinterpret_cast<void*>(0)
    );

    // Normal: XYZ
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        reinterpret_cast<void*>(
            3 * sizeof(float)
            )
    );

    glBindVertexArray(0);

    m_vertexCount =
        static_cast<GLsizei>(
            vertices.size() / 6
            );

    m_indexCount =
        static_cast<GLsizei>(
            indices.size()
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
        << " Vertices=" << m_vertexCount
        << " Indices=" << m_indexCount
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
}
