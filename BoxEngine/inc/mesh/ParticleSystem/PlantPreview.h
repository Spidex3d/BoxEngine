#pragma once

#include <glm/glm.hpp>

class GrassBlade;
class GrassClump;
class Shader;

// =====================================================
// PLANT PREVIEW
//
// Used for developing and inspecting plant geometry.
// =====================================================

class PlantPreview
{
public:

    PlantPreview() = default;
    ~PlantPreview() = default;

    void Show();
    void Hide();
    void Toggle();

    bool IsVisible() const;

    // Render the preview object.
    void Render(GrassBlade& grassBlade, const GrassClump& grassClump, Shader& shader, const glm::mat4& view, const glm::mat4& projection);

private:

    bool m_visible = false;

    // Preview position.
    glm::vec3 m_position =
        glm::vec3(0.0f);

    // Enlarge small plants for easier inspection.
    float m_previewScale = 10.0f;
};
