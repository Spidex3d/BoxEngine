
#include "mesh/ParticleSystem/GrassClump.h"

#include "mesh/ParticleSystem/GrassBlade.h"

#include <shader/Shader.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>


// =====================================================
// Grass Clump
//
// Draw seven blades using one shared GrassBlade mesh.
// =====================================================

void GrassClump::Draw(
    const GrassBlade& grassBlade,
    Shader& shader,
    const glm::mat4& model) const
{
    // ---------------------------------------------
    // Individual blade settings
    // ---------------------------------------------

    struct GrassBladeTransform
    {
        float angle;
        float heightScale;
        float widthScale;
        float tilt;
    };

    // Our seven-blade arrangement.
    const std::array<GrassBladeTransform, 7> blades =
    { {
            // Angle, Height, Width, Tilt

            {   0.0f, 1.00f, 1.00f,  0.0f },
            {  52.0f, 0.85f, 0.90f, 15.0f },
            { 105.0f, 1.15f, 1.05f, 20.0f },
            { 158.0f, 0.90f, 0.85f, 12.0f },
            { 210.0f, 1.20f, 1.10f, 18.0f },
            { 265.0f, 0.80f, 0.95f, 23.0f },
            { 315.0f, 1.05f, 1.00f, 14.0f }
        } };

    // =============================================
    // Draw all seven blades
    // =============================================

    for (const GrassBladeTransform& blade : blades)
    {
        glm::mat4 bladeModel = model;

        // -----------------------------------------
        // Rotate around the vertical axis
        // -----------------------------------------

        bladeModel = glm::rotate(
            bladeModel,
            glm::radians(blade.angle),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        // -----------------------------------------
        // Tilt outwards
        // -----------------------------------------

        bladeModel = glm::rotate(
            bladeModel,
            glm::radians(blade.tilt),
            glm::vec3(1.0f, 0.0f, 0.0f)
        );

        // -----------------------------------------
        // Height and width variation
        // -----------------------------------------

        bladeModel = glm::scale(
            bladeModel,
            glm::vec3(
                blade.widthScale,
                blade.heightScale,
                blade.widthScale
            )
        );

        // -----------------------------------------
        // Send transformation to grass shader
        // -----------------------------------------

        shader.setMat4(
            "uModel",
            bladeModel
        );

        // -----------------------------------------
        // Draw the shared grass mesh
        // -----------------------------------------

        grassBlade.Draw();
    }
}

void GrassClump::AppendInstanceMatrices(
    const glm::mat4& clumpModel,
    std::vector<glm::mat4>& matrices
) const
{
    struct BladeTransform
    {
        float angle;
        float heightScale;
        float widthScale;
        float tilt;
    };

    // Seven blades forming a natural grass clump
    const BladeTransform blades[] =
    {
        {   0.0f, 1.00f, 1.00f,  0.0f },
        {  52.0f, 0.85f, 0.90f, 15.0f },
        { 105.0f, 1.15f, 1.05f, 20.0f },
        { 158.0f, 0.90f, 0.85f, 12.0f },
        { 210.0f, 1.20f, 1.10f, 18.0f },
        { 265.0f, 0.80f, 0.95f, 23.0f },
        { 315.0f, 1.05f, 1.00f, 14.0f }
    };

    for (const BladeTransform& blade : blades)
    {
        glm::mat4 bladeModel = clumpModel;

        // Rotate blade around the centre of the clump
        bladeModel = glm::rotate(
            bladeModel,
            glm::radians(blade.angle),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        // Tilt blade outwards
        bladeModel = glm::rotate(
            bladeModel,
            glm::radians(blade.tilt),
            glm::vec3(1.0f, 0.0f, 0.0f)
        );

        // Vary blade width and height
        bladeModel = glm::scale(
            bladeModel,
            glm::vec3(
                blade.widthScale,
                blade.heightScale,
                blade.widthScale
            )
        );

        matrices.push_back(bladeModel);
    }
}