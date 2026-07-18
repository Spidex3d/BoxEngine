#pragma once
#include <glad/glad.h>
#include <memory>
#include <shader\Shader.h>
#include <vector>

class Grid
{
	public:
        Grid();
        ~Grid();

		Grid(const Grid&) = delete;
		Grid& operator=(const Grid&) = delete;

		Grid(Grid&& other) noexcept;
		Grid& operator=(Grid&& other) noexcept;
		

		bool Create(int halfSize, float spacing);
        void Render(const Shader& shader, const glm::mat4& view, const glm::mat4& projection) const;
        
		void Destroy();

        void SetPosition(const glm::vec3& position)
        {
            m_position = position;
        }

        

        const glm::vec3& GetPosition() const
        {
            return m_position;
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

	GLuint m_vao = 0;
	GLuint m_vbo = 0;

    GLsizei m_vertexCount = 0;

	int m_halfSize = 10;
	float m_spacing = 1.0f;

	glm::vec3 m_position{ 0.0f };

	bool m_visible = true;

};