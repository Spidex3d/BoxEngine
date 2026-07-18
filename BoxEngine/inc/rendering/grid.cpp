#include <rendering\grid.h>

#include <shader/Shader.h>
#include <miniBoxLog.h>

#include <glm/gtc/matrix_transform.hpp>

#include <utility>
#include <vector>

Grid::Grid() = default;

Grid::~Grid()
{
    Destroy();
}

Grid::Grid(Grid&& other) noexcept
    : m_vao(other.m_vao),
    m_vbo(other.m_vbo),
    m_vertexCount(other.m_vertexCount),
    m_halfSize(other.m_halfSize),
    m_spacing(other.m_spacing),
    m_position(other.m_position),
    m_visible(other.m_visible)
{
    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_vertexCount = 0;
}

Grid& Grid::operator=(Grid&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    Destroy();

    m_vao = other.m_vao;
    m_vbo = other.m_vbo;
    m_vertexCount = other.m_vertexCount;

    m_halfSize = other.m_halfSize;
    m_spacing = other.m_spacing;
    m_position = other.m_position;
    m_visible = other.m_visible;

    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_vertexCount = 0;

    return *this;
}

bool Grid::Create(
    int halfSize,
    float spacing)
{
    Destroy();

    if (halfSize <= 0)
    {
        BOX_LOG_ERROR(
            "Grid::Create requires halfSize greater than zero"
        );

        return false;
    }

    if (spacing <= 0.0f)
    {
        BOX_LOG_ERROR(
            "Grid::Create requires spacing greater than zero"
        );

        return false;
    }

    m_halfSize = halfSize;
    m_spacing = spacing;

    std::vector<float> vertices;

    const float extent =
        static_cast<float>(m_halfSize) * m_spacing;

    /*
        For every step, create:

        One line parallel to the Z axis
        One line parallel to the X axis
    */
    for (int i = -m_halfSize; i <= m_halfSize; ++i)
    {
        const float coordinate =
            static_cast<float>(i) * m_spacing;

        // Line parallel to Z.
        vertices.push_back(coordinate);
        vertices.push_back(0.0f);
        vertices.push_back(-extent);

        vertices.push_back(coordinate);
        vertices.push_back(0.0f);
        vertices.push_back(extent);

        // Line parallel to X.
        vertices.push_back(-extent);
        vertices.push_back(0.0f);
        vertices.push_back(coordinate);

        vertices.push_back(extent);
        vertices.push_back(0.0f);
        vertices.push_back(coordinate);
    }

    m_vertexCount =
        static_cast<GLsizei>(
            vertices.size() / 3
            );

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

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

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        reinterpret_cast<void*>(0)
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    if (m_vao == 0 || m_vbo == 0)
    {
        BOX_LOG_ERROR(
            "Grid::Create failed to create OpenGL buffers"
        );

        Destroy();
        return false;
    }

    BOX_LOG_INFO(
        "Created grid. Half size: "
        << m_halfSize
        << ", spacing: "
        << m_spacing
        << ", vertices: "
        << m_vertexCount
    );

    return true;
}

void Grid::Render(
    const Shader& shader,
    const glm::mat4& view,
    const glm::mat4& projection) const
{
    if (!m_visible || m_vao == 0)
    {
        return;
    }

    shader.Use();

    glm::mat4 model(1.0f);

    model = glm::translate(
        model,
        m_position
    );

    shader.setMat4(
        "uModel",
        model
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

    glDrawArrays(
        GL_LINES,
        0,
        m_vertexCount
    );

    glBindVertexArray(0);
}

void Grid::Destroy()
{
    if (m_vbo != 0)
    {
        glDeleteBuffers(
            1,
            &m_vbo
        );

        m_vbo = 0;
    }

    if (m_vao != 0)
    {
        glDeleteVertexArrays(
            1,
            &m_vao
        );

        m_vao = 0;
    }

    m_vertexCount = 0;
}