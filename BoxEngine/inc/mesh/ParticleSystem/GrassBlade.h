#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class GrassBlade
{
public:
	GrassBlade();
	~GrassBlade();

	GrassBlade(const GrassBlade&) = delete;

	GrassBlade& operator=(const GrassBlade&) = delete;

	bool CreateGrassBlade();

	// Draw many copies of the same grass mesh.
	void DrawInstanced(const std::vector<glm::mat4>& instanceMatrices) const;

	void Draw() const;

	void SetPosition(const glm::vec3& position);
	void SetSize(float width, float height);
	const glm::vec3& GetPosition() const;
	float GetWidth() const;
	float GetHeight() const;

	void SetCurvature(float curvature);
	void SetConcavity(float concavity);

	const glm::vec4& GetColor() const;
	void SetColor(const glm::vec4& color);

	void Destroy();

private:
	GLuint m_vao = 0;
	GLuint m_vbo = 0;
	GLuint m_ebo = 0;

	GLuint m_instanceVbo = 0;
	GLsizei m_indexCount = 0;

	glm::vec3 m_position = glm::vec3(0.0f);

	// Measurements in metres, assuming 1 world unit = 1 metre
	float m_width = 0.01f;   // 1 cm
	float m_height = 0.20f;  // 20 cm

	// How much the blade bends towards its tip
	float m_curvature = 0.035f;

	// Depth of the central groove
	float m_concavity = 0.0015f;

	glm::vec4 m_color = glm::vec4(1.0f);


};
