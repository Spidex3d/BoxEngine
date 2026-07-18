#pragma once
#include <glad\glad.h>
#include <glm\glm.hpp>

#include <string>


//This is where we are putting together all the components of an entity, such as mesh, material, transform, etc.
class Shader;

class Entity
{
public:
    Entity(
        int id,
        const std::string& name
    );

    ~Entity();

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    Entity(Entity&& other) noexcept;
    Entity& operator=(Entity&& other) noexcept;

    bool CreateCube();

    void Render(
        const Shader& shader,
        const glm::mat4& view,
        const glm::mat4& projection
    );

    void Destroy();

    int GetID() const
    {
        return m_id;
    }

    const std::string& GetName() const
    {
        return m_name;
    }

    void SetPosition(const glm::vec3& position)
    {
        m_position = position;
    }

    void SetRotation(const glm::vec3& rotation)
    {
        m_rotation = rotation;
    }

    void SetScale(const glm::vec3& scale)
    {
        m_scale = scale;
    }

    const glm::vec3& GetPosition() const
    {
        return m_position;
    }

    const glm::vec3& GetRotation() const
    {
        return m_rotation;
    }

    const glm::vec3& GetScale() const
    {
        return m_scale;
    }

    void SetVisible(bool visible)
    {
        m_visible = visible;
    }

    bool IsVisible() const
    {
        return m_visible;
    }

private:
    glm::mat4 CalculateModelMatrix() const;

private:
    int m_id = -1;
    std::string m_name;

    glm::vec3 m_position{ 0.0f };
    glm::vec3 m_rotation{ 0.0f };
    glm::vec3 m_scale{ 1.0f };

    bool m_visible = true;

    GLuint m_vao = 0;
    GLuint m_vbo = 0;

    GLsizei m_vertexCount = 0;
};