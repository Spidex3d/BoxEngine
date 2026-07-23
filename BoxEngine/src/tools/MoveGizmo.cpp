#include "tools/MoveGizmo.h"
#include "shader/Shader.h"

#include <glm/gtc/matrix_transform.hpp>

MoveGizmo::~MoveGizmo()
{
    Destroy();
}

bool MoveGizmo::Create()
{
    if (m_created)
    {
        return true;
    }

    // Three separate lines:
    //
    // X: origin to +X
    // Y: origin to +Y
    // Z: origin to +Z
    const float axisVertices[] =
    {
        // X axis
        0.0f, 0.0f, 0.0f,
        0.5f, 0.0f, 0.0f,

        // Y axis
        0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f,

        // Z axis
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f
    };

    glGenVertexArrays(
        1,
        &m_lineVAO
    );

    glGenBuffers(
        1,
        &m_lineVBO
    );

    glBindVertexArray(
        m_lineVAO
    );

    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_lineVBO
    );

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(axisVertices),
        axisVertices,
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        nullptr
    );

    glBindVertexArray(0);

    // Small centre cube.
    const float cubeVertices[] =
    {
        // Back
        -0.06f, -0.06f, -0.06f,
         0.06f,  0.06f, -0.06f,
         0.06f, -0.06f, -0.06f,

         0.06f,  0.06f, -0.06f,
        -0.06f, -0.06f, -0.06f,
        -0.06f,  0.06f, -0.06f,

        // Front
        -0.06f, -0.06f, 0.06f,
         0.06f, -0.06f, 0.06f,
         0.06f,  0.06f, 0.06f,

         0.06f,  0.06f, 0.06f,
        -0.06f,  0.06f, 0.06f,
        -0.06f, -0.06f, 0.06f,

        // Left
        -0.06f,  0.06f,  0.06f,
        -0.06f,  0.06f, -0.06f,
        -0.06f, -0.06f, -0.06f,

        -0.06f, -0.06f, -0.06f,
        -0.06f, -0.06f,  0.06f,
        -0.06f,  0.06f,  0.06f,

        // Right
         0.06f,  0.06f,  0.06f,
         0.06f, -0.06f, -0.06f,
         0.06f,  0.06f, -0.06f,

         0.06f, -0.06f, -0.06f,
         0.06f,  0.06f,  0.06f,
         0.06f, -0.06f,  0.06f,

         // Bottom
         -0.06f, -0.06f, -0.06f,
          0.06f, -0.06f, -0.06f,
          0.06f, -0.06f,  0.06f,

          0.06f, -0.06f,  0.06f,
         -0.06f, -0.06f,  0.06f,
         -0.06f, -0.06f, -0.06f,

         // Top
         -0.06f, 0.06f, -0.06f,
          0.06f, 0.06f,  0.06f,
          0.06f, 0.06f, -0.06f,

          0.06f, 0.06f,  0.06f,
         -0.06f, 0.06f, -0.06f,
         -0.06f, 0.06f,  0.06f
    };

    glGenVertexArrays(
        1,
        &m_cubeVAO
    );

    glGenBuffers(
        1,
        &m_cubeVBO
    );

    glBindVertexArray(
        m_cubeVAO
    );

    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_cubeVBO
    );

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(cubeVertices),
        cubeVertices,
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        nullptr
    );

    glBindVertexArray(0);

    m_created = true;

    return true;
}

void MoveGizmo::DrawAxis(
    Shader& shader,
    const glm::mat4& model,
    const glm::vec3& colour,
    int firstVertex)
{
    shader.setMat4(
        "model",
        model
    );

    shader.setVec3(
        "gizmoColor",
        colour
    );

    glDrawArrays(
        GL_LINES,
        firstVertex,
        2
    );
}

void MoveGizmo::Render(
    Shader& shader,
    const glm::vec3& position,
    float scale,
    const glm::mat4& view,
    const glm::mat4& projection)
{
    if (!m_created)
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

    glm::mat4 model(1.0f);

    model = glm::translate(
        model,
        position
    );

    model = glm::scale(
        model,
        glm::vec3(scale)
    );

    // Keep the gizmo visible through the selected object.
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glDisable(GL_CULL_FACE);
    glLineWidth(4.0f);

    glBindVertexArray(
        m_lineVAO
    );

    // X axis: vertices 0 and 1.
    DrawAxis(
        shader,
        model,
        glm::vec3(1.0f, 0.15f, 0.15f),
        0
    );

    // Y axis: vertices 2 and 3.
    DrawAxis(
        shader,
        model,
        glm::vec3(0.15f, 1.0f, 0.15f),
        2
    );

    // Z axis: vertices 4 and 5.
    DrawAxis(
        shader,
        model,
        glm::vec3(0.15f, 0.4f, 1.0f),
        4
    );

    glBindVertexArray(0);

    // Centre cube.
    shader.setMat4(
        "model",
        model
    );

    shader.setVec3(
        "gizmoColor",
        glm::vec3(0.9f)
    );

    glBindVertexArray(
        m_cubeVAO
    );

    glDrawArrays(
        GL_TRIANGLES,
        0,
        36
    );

    glBindVertexArray(0);

    glLineWidth(1.0f);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void MoveGizmo::Destroy()
{
    if (m_cubeVBO != 0)
    {
        glDeleteBuffers(
            1,
            &m_cubeVBO
        );

        m_cubeVBO = 0;
    }

    if (m_cubeVAO != 0)
    {
        glDeleteVertexArrays(
            1,
            &m_cubeVAO
        );

        m_cubeVAO = 0;
    }

    if (m_lineVBO != 0)
    {
        glDeleteBuffers(
            1,
            &m_lineVBO
        );

        m_lineVBO = 0;
    }

    if (m_lineVAO != 0)
    {
        glDeleteVertexArrays(
            1,
            &m_lineVAO
        );

        m_lineVAO = 0;
    }

    m_created = false;
}