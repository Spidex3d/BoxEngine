#include "entity/Entity.h"
#include <shader/Shader.h>
#include <miniBoxLog.h>
#include <stb/stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <utility>

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
    : m_id(other.m_id),
    m_name(std::move(other.m_name)),
    m_position(other.m_position),
    m_rotation(other.m_rotation),
    m_scale(other.m_scale),
    m_visible(other.m_visible),
    m_vao(other.m_vao),
    m_vbo(other.m_vbo),
    m_vertexCount(other.m_vertexCount)
{
    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_vertexCount = 0;
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
    glBindVertexArray(m_vao);

    glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);

    glBindVertexArray(0);
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

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    glBindVertexArray(0);
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
