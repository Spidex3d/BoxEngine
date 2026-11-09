#include "mesh/ParticleSystem/PlantPreview.h"

#include <mesh/ParticleSystem/GrassBlade.h>
#include <mesh/ParticleSystem/GrassClump.h>

#include <shader/Shader.h>

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// =====================================================
// VISIBILITY
// =====================================================

void PlantPreview::Show()
{
    m_visible = true;
}

void PlantPreview::Hide()
{
    m_visible = false;
}

void PlantPreview::Toggle()
{
    m_visible = !m_visible;
}

bool PlantPreview::IsVisible() const
{
    return m_visible;
}


// =====================================================
// RENDER PLANT PREVIEW
// =====================================================

void PlantPreview::Render(
    GrassBlade& grassBlade,
    const GrassClump& grassClump,
    Shader& shader,
    const glm::mat4& view,
    const glm::mat4& projection)
{
    if (!m_visible || shader.ID() == 0)
        return;

    shader.Use();

    // ---------------------------------------------
    // Camera matrices
    // ---------------------------------------------

    shader.setMat4("uView", view);
    shader.setMat4("uProjection", projection);

    // ---------------------------------------------
    // Lighting
    // ---------------------------------------------

    shader.setVec3(
        "uLightDirection",
        glm::normalize(
            glm::vec3(0.5f, 1.0f, 0.4f)
        )
    );

    shader.setVec3(
        "uLightColor",
        glm::vec3(1.0f)
    );

    shader.SetUniformFloat(
        "uAmbientStrength",
        0.35f
    );

    // ---------------------------------------------
    // Preview transformation
    // ---------------------------------------------

    glm::mat4 model(1.0f);

    model = glm::translate(
        model,
        m_position
    );

    model = glm::scale(
        model,
        glm::vec3(m_previewScale)
    );

    // ---------------------------------------------
    // Generate clump matrices
    // ---------------------------------------------

    std::vector<glm::mat4> matrices;

    matrices.reserve(7);

    grassClump.AppendInstanceMatrices(
        model,
        matrices
    );

    // ---------------------------------------------
    // Draw both sides of the leaves
    // ---------------------------------------------

    const GLboolean cullingEnabled =
        glIsEnabled(GL_CULL_FACE);

    glDisable(GL_CULL_FACE);

    shader.SetUniformInt(
        "uUseInstancing",
        1
    );

    shader.setMat4(
        "uModel",
        glm::mat4(1.0f)
    );

    grassBlade.DrawInstanced(matrices);

    shader.SetUniformInt(
        "uUseInstancing",
        0
    );

    if (cullingEnabled)
    {
        glEnable(GL_CULL_FACE);
    }
}

// =====================================================
// RENDER SINGLE PLANT BLADE
// =====================================================

void PlantPreview::RenderSingleBlade(
    GrassBlade& grassBlade,
    Shader& shader,
    const glm::mat4& view,
    const glm::mat4& projection)
{
    if (!m_visible || shader.ID() == 0)
        return;

    shader.Use();

    // Camera matrices
    shader.setMat4("uView", view);
    shader.setMat4("uProjection", projection);

    // Lighting
    shader.setVec3(
        "uLightDirection",
        glm::normalize(
            glm::vec3(0.5f, 1.0f, 0.4f)
        )
    );

    shader.setVec3(
        "uLightColor",
        glm::vec3(1.0f)
    );

    shader.SetUniformFloat(
        "uAmbientStrength",
        0.35f
    );

    // Position and scale.
    glm::mat4 model(1.0f);

    model = glm::translate(
        model,
        m_position
    );

    model = glm::scale(
        model,
        glm::vec3(m_previewScale)
    );

    shader.SetUniformInt(
        "uUseInstancing",
        0
    );

    shader.setMat4(
        "uModel",
        model
    );

    // Render both sides of the leaf.
    const GLboolean cullingEnabled =
        glIsEnabled(GL_CULL_FACE);

    glDisable(GL_CULL_FACE);

    grassBlade.Draw();

    if (cullingEnabled)
    {
        glEnable(GL_CULL_FACE);
    }
}
void PlantPreview::SetMode(PlantPreviewMode mode)
{
    m_mode = mode;
}

PlantPreviewMode PlantPreview::GetMode() const
{
    return m_mode;
}

// =====================================================
// PLANT PREVIEW SETTINGS
// =====================================================

PlantPreviewSettings& PlantPreview::GetSettings()
{
    return m_settings;
}

void PlantPreview::ApplyPreviewScale()
{
    m_previewScale = glm::max(
        m_settings.previewScale,
        0.01f
    );
}