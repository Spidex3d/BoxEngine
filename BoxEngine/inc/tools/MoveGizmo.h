#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader;

class MoveGizmo
{
public:
    MoveGizmo() = default;
    ~MoveGizmo();

    MoveGizmo(
        const MoveGizmo&) = delete;

    MoveGizmo& operator=(
        const MoveGizmo&) = delete;

    bool Create();

    void Render(
        Shader& shader,
        const glm::vec3& position,
        float scale,
        const glm::mat4& view,
        const glm::mat4& projection
    );

    void Destroy();

private:
    void DrawAxis(
        Shader& shader,
        const glm::mat4& model,
        const glm::vec3& colour,
        int firstVertex
    );

private:
    GLuint m_lineVAO = 0;
    GLuint m_lineVBO = 0;

    GLuint m_cubeVAO = 0;
    GLuint m_cubeVBO = 0;

    bool m_created = false;
};

