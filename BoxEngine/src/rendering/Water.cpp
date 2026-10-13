#include "rendering/Water.h"
#include <shader/Shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>	



bool Water::Initialize(float size)
{
    m_size = size;

    const float halfSize =
        m_size * 0.5f;


    // -------------------------------------------------
    // One large flat quad
    // -------------------------------------------------

    const float vertices[] =
    {
        -halfSize, 0.0f, -halfSize,
         halfSize, 0.0f, -halfSize,
         halfSize, 0.0f,  halfSize,
        -halfSize, 0.0f,  halfSize
    };


    const unsigned int indices[] =
    {
        0, 2, 1,
        0, 3, 2
    };


    // -------------------------------------------------
    // VAO
    // -------------------------------------------------

    glGenVertexArrays(
        1,
        &m_VAO
    );

    glBindVertexArray(
        m_VAO
    );


    // -------------------------------------------------
    // VBO
    // -------------------------------------------------

    glGenBuffers(
        1,
        &m_VBO
    );

    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_VBO
    );

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_STATIC_DRAW
    );


    // -------------------------------------------------
    // EBO
    // -------------------------------------------------

    glGenBuffers(
        1,
        &m_EBO
    );

    glBindBuffer(
        GL_ELEMENT_ARRAY_BUFFER,
        m_EBO
    );

    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(indices),
        indices,
        GL_STATIC_DRAW
    );


    // -------------------------------------------------
    // Position
    // -------------------------------------------------

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        reinterpret_cast<void*>(0)
    );

    glEnableVertexAttribArray(0);


    glBindVertexArray(0);

    glBindBuffer(
        GL_ARRAY_BUFFER,
        0
    );


    return true;
}


void Water::Render(
    Shader& shader,
    const glm::mat4& view,
    const glm::mat4& projection)
{
    if (m_VAO == 0)
    {
        return;
    }


    shader.Use();


    // -------------------------------------------------
    // Water position
    // -------------------------------------------------

    glm::mat4 model =
        glm::translate(
            glm::mat4(1.0f),
            glm::vec3(
                0.0f,
                m_height,
                0.0f
            )
        );


    shader.setMat4(
        "model",
        model
    );

    shader.setMat4(
        "view",
        view
    );

    shader.setMat4(
        "projection",
        projection
    );


    // -------------------------------------------------
    // Transparency
    // -------------------------------------------------

    glEnable(GL_BLEND);

    glBlendFunc(
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA
    );


    // Don't let transparent water write into
    // the depth buffer.
    glDepthMask(GL_FALSE);


    glBindVertexArray(
        m_VAO
    );

    glDrawElements(
        GL_TRIANGLES,
        6,
        GL_UNSIGNED_INT,
        nullptr
    );

    glBindVertexArray(0);


    glDepthMask(GL_TRUE);

    glDisable(GL_BLEND);
}


void Water::Destroy()
{
    if (m_EBO != 0)
    {
        glDeleteBuffers(
            1,
            &m_EBO
        );

        m_EBO = 0;
    }

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
}