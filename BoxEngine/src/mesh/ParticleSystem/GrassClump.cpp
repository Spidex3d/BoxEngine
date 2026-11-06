
#include "mesh/ParticleSystem/GrassClump.h"

#include "mesh/ParticleSystem/GrassBlade.h"

#include <shader/Shader.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <random>
#include <algorithm>
#include <cmath>

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
// =====================================================
// GENERATE NATURAL OUTWARD-GROWING GRASS CLUMP
// =====================================================

void GrassClump::AppendInstanceMatrices(
    const glm::mat4& clumpModel,
    const GrassClumpSettings& settings,
    std::uint32_t seed,
    std::vector<glm::mat4>& matrices
) const
{
    // ================================================
    // VALIDATE SETTINGS
    // ================================================

    const int bladeCount =
        std::clamp(settings.bladeCount, 1, 100);

    const float spread =
        std::max(0.0f, settings.spread);

    const float minHeight =
        std::max(0.01f, settings.minHeight);

    const float maxHeight =
        std::max(minHeight, settings.maxHeight);

    const float minWidth =
        std::max(0.01f, settings.minWidth);

    const float maxWidth =
        std::max(minWidth, settings.maxWidth);

    const float minTilt =
        std::clamp(settings.minTilt, 0.0f, 90.0f);

    const float maxTilt =
        std::clamp(
            settings.maxTilt,
            minTilt,
            90.0f
        );

    // ================================================
    // RANDOM GENERATOR
    // ================================================

    std::mt19937 random(seed);

    std::uniform_real_distribution<float>
        random01(0.0f, 1.0f);

    std::uniform_real_distribution<float>
        randomAngle(0.0f, 360.0f);

    std::uniform_real_distribution<float>
        randomHeight(minHeight, maxHeight);

    std::uniform_real_distribution<float>
        randomWidth(minWidth, maxWidth);

    std::uniform_real_distribution<float>
        randomTilt(minTilt, maxTilt);

    // A small amount of directional variation.
    std::uniform_real_distribution<float>
        randomDirectionVariation(-8.0f, 8.0f);

    // ================================================
    // GENERATE BLADES
    // ================================================

    for (int i = 0; i < bladeCount; ++i)
    {
        // --------------------------------------------
        // Position within the clump
        // --------------------------------------------

        const float radialAngle =
            glm::radians(randomAngle(random));

        const float distance =
            std::sqrt(random01(random)) * spread;

        const float offsetX =
            std::cos(radialAngle) * distance;

        const float offsetZ =
            std::sin(radialAngle) * distance;

        // --------------------------------------------
        // Calculate outward direction
        // --------------------------------------------

        // Our grass blade curves towards local -Z.
        // Rotate that direction to point away from
        // the centre of the clump.

        const float outwardAngle =
            std::atan2(
                -std::cos(radialAngle),
                -std::sin(radialAngle)
            );

        // Small random variation to avoid perfection.
        const float directionVariation =
            glm::radians(
                randomDirectionVariation(random)
            );

        const float finalAngle =
            outwardAngle + directionVariation;

        // --------------------------------------------
        // Individual blade variation
        // --------------------------------------------

        const float heightScale =
            randomHeight(random);

        const float widthScale =
            randomWidth(random);

        const float tilt =
            glm::radians(randomTilt(random));

        // --------------------------------------------
        // Build blade transformation
        // --------------------------------------------

        glm::mat4 bladeModel = clumpModel;

        // Position the blade within the clump.
        bladeModel = glm::translate(
            bladeModel,
            glm::vec3(
                offsetX,
                0.0f,
                offsetZ
            )
        );

        // Rotate so the curved tip points outwards.
        bladeModel = glm::rotate(
            bladeModel,
            finalAngle,
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        // Tilt outwards in the same direction.
        bladeModel = glm::rotate(
            bladeModel,
            tilt,
            glm::vec3(1.0f, 0.0f, 0.0f)
        );

        // Apply blade dimensions.
        bladeModel = glm::scale(
            bladeModel,
            glm::vec3(
                widthScale,
                heightScale,
                widthScale
            )
        );

        // Add to GPU instance collection.
        matrices.push_back(bladeModel);
    }
}

//// =====================================================
//// GENERATE CONFIGURABLE GRASS CLUMP
//// =====================================================
//
//void GrassClump::AppendInstanceMatrices(
//    const glm::mat4& clumpModel,
//    const GrassClumpSettings& settings,
//    std::uint32_t seed,
//    std::vector<glm::mat4>& matrices
//) const
//{
//    // ================================================
//    // VALIDATE SETTINGS
//    // ================================================
//
//    const int bladeCount =
//        std::clamp(settings.bladeCount, 1, 100);
//
//    const float spread =
//        std::max(0.0f, settings.spread);
//
//    const float minHeight =
//        std::max(0.01f, settings.minHeight);
//
//    const float maxHeight =
//        std::max(minHeight, settings.maxHeight);
//
//    const float minWidth =
//        std::max(0.01f, settings.minWidth);
//
//    const float maxWidth =
//        std::max(minWidth, settings.maxWidth);
//
//    const float minTilt =
//        std::clamp(settings.minTilt, 0.0f, 90.0f);
//
//    const float maxTilt =
//        std::clamp(
//            settings.maxTilt,
//            minTilt,
//            90.0f
//        );
//
//    // ================================================
//    // RANDOM GENERATOR
//    // ================================================
//
//    std::mt19937 random(seed);
//
//    std::uniform_real_distribution<float>
//        random01(0.0f, 1.0f);
//
//    std::uniform_real_distribution<float>
//        randomAngle(0.0f, 360.0f);
//
//    std::uniform_real_distribution<float>
//        randomHeight(minHeight, maxHeight);
//
//    std::uniform_real_distribution<float>
//        randomWidth(minWidth, maxWidth);
//
//    std::uniform_real_distribution<float>
//        randomTilt(minTilt, maxTilt);
//
//    // ================================================
//    // GENERATE INDIVIDUAL BLADES
//    // ================================================
//
//    for (int i = 0; i < bladeCount; ++i)
//    {
//        // --------------------------------------------
//        // Random position within a circular clump
//        // --------------------------------------------
//
//        const float angle =
//            glm::radians(randomAngle(random));
//
//        // Square root gives an even distribution
//        // over the area of a circle.
//        const float distance =
//            std::sqrt(random01(random)) * spread;
//
//        const float offsetX =
//            std::cos(angle) * distance;
//
//        const float offsetZ =
//            std::sin(angle) * distance;
//
//        // --------------------------------------------
//        // Random blade variations
//        // --------------------------------------------
//
//        const float heightScale =
//            randomHeight(random);
//
//        const float widthScale =
//            randomWidth(random);
//
//        const float tilt =
//            glm::radians(randomTilt(random));
//
//        // --------------------------------------------
//        // Build transformation
//        // --------------------------------------------
//
//        glm::mat4 bladeModel = clumpModel;
//
//        // Position blade within the clump.
//        bladeModel = glm::translate(
//            bladeModel,
//            glm::vec3(
//                offsetX,
//                0.0f,
//                offsetZ
//            )
//        );
//
//        // Rotate around vertical axis.
//        bladeModel = glm::rotate(
//            bladeModel,
//            angle,
//            glm::vec3(0.0f, 1.0f, 0.0f)
//        );
//
//        // Tilt blade outwards.
//        bladeModel = glm::rotate(
//            bladeModel,
//            tilt,
//            glm::vec3(1.0f, 0.0f, 0.0f)
//        );
//
//        // Apply height and width variation.
//        bladeModel = glm::scale(
//            bladeModel,
//            glm::vec3(
//                widthScale,
//                heightScale,
//                widthScale
//            )
//        );
//
//        // Store transformation for GPU instancing.
//        matrices.push_back(bladeModel);
//    }
//}