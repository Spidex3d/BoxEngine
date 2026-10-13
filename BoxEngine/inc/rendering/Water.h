#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader;

class Water
{
public:
    bool Initialize(float size);

    void SetHeight(float height)
    {
        m_height = height;
    }

    void Render(Shader& shader, const glm::mat4& view, const glm::mat4& projection);

    void Destroy();

private:
    GLuint m_VAO = 0;
    GLuint m_VBO = 0;
    GLuint m_EBO = 0;

    float m_height = 0.0f;
    float m_size = 300.0f;
};
