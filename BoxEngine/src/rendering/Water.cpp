#include "rendering/Water.h"
#include <shader/Shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>	
#include <stb/stb_image.h>
#include <miniboxlog.h>



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
        // Position                     // UV
        -halfSize, 0.0f, -halfSize,     0.0f, 0.0f,
         halfSize, 0.0f, -halfSize,     1.0f, 0.0f,
         halfSize, 0.0f,  halfSize,     1.0f, 1.0f,
        -halfSize, 0.0f,  halfSize,     0.0f, 1.0f
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
        5 * sizeof(float),
        reinterpret_cast<void*>(0)
    );

    glEnableVertexAttribArray(0);

    
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        5 * sizeof(float),
        reinterpret_cast<void*>(
            3 * sizeof(float)
            )
    );

    glEnableVertexAttribArray(1);


    glBindVertexArray(0);

    glBindBuffer(
        GL_ARRAY_BUFFER,
        0
    );

    if (!LoadNormalMap(
        "assets/textures/terrain/water/Normal Map.png"))
    {
        return false;
    }


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
	// Normal map texture unit
    shader.SetUniformInt(
        "uNormalMap",
        0
    );

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, m_normalMap);

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

	// normal map texture unit
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, 0);

}

void Water::Update(float deltaTime)
{
    m_time += deltaTime;
}




bool Water::LoadNormalMap(
    const char* path)
{
    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* data =
        stbi_load(
            path,
            &width,
            &height,
            &channels,
            0
        );

    if (!data)
    {
		BOX_LOG_ERROR("Failed to load water normal map: " << path);

       

        return false;
    }


    GLenum format = GL_RGB;

    if (channels == 4)
    {
        format = GL_RGBA;
    }
    else if (channels == 3)
    {
        format = GL_RGB;
    }
    else
    {
		BOX_LOG_ERROR("Unsupported water normal map format : " << path);

       
        stbi_image_free(data);

        return false;
    }


    glGenTextures(
        1,
        &m_normalMap
    );

    glBindTexture(
        GL_TEXTURE_2D,
        m_normalMap
    );


    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_REPEAT
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_REPEAT
    );


    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );


    glPixelStorei(
        GL_UNPACK_ALIGNMENT,
        1
    );


    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );


    glGenerateMipmap(
        GL_TEXTURE_2D
    );


    stbi_image_free(data);

    glBindTexture(
        GL_TEXTURE_2D,
        0
    );

	BOX_LOG_INFO("Loaded water normal map: " + std::string(path));

    


    return true;
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
	// Delete the normal map texture if it exists
    if (m_normalMap != 0)
    {
        glDeleteTextures(
            1,
            &m_normalMap
        );

        m_normalMap = 0;
    }
}