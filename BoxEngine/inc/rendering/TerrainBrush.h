#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>

class Terrain;
class Shader;

class TerrainBrush
{
public:

    bool Initialize();

    void Update(
        const Terrain& terrain,
        const glm::vec3& position,
        float radius
    );

    void Render(
        Shader& shader,
        const glm::mat4& view,
        const glm::mat4& projection
    );

    void Hide()
    {
        m_visible = false;
    }

    void Destroy();

private:

    GLuint m_VAO = 0;
    GLuint m_VBO = 0;

    std::vector<glm::vec3> m_vertices;

    bool m_visible = false;
};
