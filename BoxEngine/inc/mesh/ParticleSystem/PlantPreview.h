#pragma once

#include <glm/glm.hpp>

class GrassBlade;
class GrassClump;
class Shader;

struct PlantPreviewSettings
{
    float bladeWidth = 0.010f;
    float bladeHeight = 0.200f;

    float curvature = 0.035f;
    float concavity = 0.0015f;

    float breakPosition = 0.80f;
    float breakAngle = 125.0f;

    float previewScale = 10.0f;
};


// =====================================================
// PLANT PREVIEW TYPES
// =====================================================

enum class PlantPreviewMode
{
    NormalBlade,
    BrokenBlade,
    GrassClump
};

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

    // Display one blade for close inspection.
    void RenderSingleBlade(GrassBlade& grassBlade, Shader& shader, const glm::mat4& view, const glm::mat4& projection);

    // Select which plant object to preview.
    void SetMode(PlantPreviewMode mode);

    // Return the currently selected preview type.
    PlantPreviewMode GetMode() const;

    PlantPreviewSettings& GetSettings();
    void ApplyPreviewScale();


private:

    bool m_visible = false;
    // Currently selected preview object.
    PlantPreviewMode m_mode = PlantPreviewMode::GrassClump;

    // Preview position.
    glm::vec3 m_position = glm::vec3(0.0f);

    // Enlarge small plants for easier inspection.
    float m_previewScale = 10.0f;

    PlantPreviewSettings m_settings;
};
