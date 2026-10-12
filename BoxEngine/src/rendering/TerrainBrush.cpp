#include <rendering/TerrainBrush.h>

#include <rendering/Terrain.h>
#include <shader/Shader.h>

#include <glm/gtc/constants.hpp>

#include <cmath>

bool TerrainBrush::Initialize()
{
    glGenVertexArrays(
        1,
        &m_VAO
    );

    glGenBuffers(
        1,
        &m_VBO
    );

    glBindVertexArray(
        m_VAO
    );

    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_VBO
    );

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(glm::vec3) * 64,
        nullptr,
        GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        reinterpret_cast<void*>(0)
    );

    glEnableVertexAttribArray(0);

    glBindBuffer(
        GL_ARRAY_BUFFER,
        0
    );

    glBindVertexArray(0);

    return true;
}

void TerrainBrush::Update(
    const Terrain& terrain,
    const glm::vec3& position,
    float radius)
{
    constexpr int segments = 64;

    m_vertices.clear();

    m_vertices.reserve(
        segments
    );

    for (int i = 0;
        i < segments;
        ++i)
    {
        const float angle =
            glm::two_pi<float>() *
            static_cast<float>(i) /
            static_cast<float>(segments);

        const float x =
            position.x +
            std::cos(angle) *
            radius;

        const float z =
            position.z +
            std::sin(angle) *
            radius;

        float y = 0.0f;

        if (!terrain.GetHeightAtWorldPosition(
            x,
            z,
            y))
        {
            continue;
        }

        // Lift slightly above terrain to prevent
        // z-fighting with the terrain surface.
        y += 0.05f;

        m_vertices.push_back(
            glm::vec3(
                x,
                y,
                z
            )
        );
    }

    m_visible =
        m_vertices.size() >= 3;

    if (!m_visible)
    {
        return;
    }

    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_VBO
    );

    glBufferData(
        GL_ARRAY_BUFFER,
        m_vertices.size() *
        sizeof(glm::vec3),
        m_vertices.data(),
        GL_DYNAMIC_DRAW
    );

    glBindBuffer(
        GL_ARRAY_BUFFER,
        0
    );
}

void TerrainBrush::Render(
    Shader& shader,
    const glm::mat4& view,
    const glm::mat4& projection)
{
    if (!m_visible ||
        m_vertices.empty())
    {
        return;
    }

    shader.Use();

    shader.setMat4(
        "view",
        view
    );

    shader.setMat4(
        "projection",
        projection
    );

    shader.setVec3(
        "brushColor",
        glm::vec3(
            1.0f,
            0.75f,
            0.1f
        )
    );

    glBindVertexArray(
        m_VAO
    );

    glLineWidth(
        2.0f
    );

    glDrawArrays(
        GL_LINE_LOOP,
        0,
        static_cast<GLsizei>(
            m_vertices.size()
            )
    );

    glLineWidth(
        1.0f
    );

    glBindVertexArray(0);
}

void TerrainBrush::Destroy()
{
    if (m_VBO != 0)
    {
        glDeleteBuffers(
            1,
            &m_VBO
        );

        m_VBO = 0;
    }

    if (m_VAO != 0)
    {
        glDeleteVertexArrays(
            1,
            &m_VAO
        );

        m_VAO = 0;
    }

    m_vertices.clear();
    m_visible = false;
}