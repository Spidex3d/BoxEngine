#include "mesh/ParticleSystem/ParticleSystemRender.h"

#include <entity/Entity.h>
#include <mesh/MeshEditing.h>
#include <mesh/ParticleSystem/ParticleScatter.h>
#include <mesh/ParticleSystem/GrassBlade.h>
#include <mesh/ParticleSystem/GrassClump.h>
#include <mesh/modifiers/ModifierData.h>
#include <shader/Shader.h>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <utility>

// =====================================================
// PARTICLE SYSTEM - GRASS RENDERING
// =====================================================

//void ParticleSystemRender::RenderGrass(
//    const std::vector<std::unique_ptr<Entity>>& entities,
//    GrassBlade& grassBlade,
//    const GrassClump& grassClump,
//    Shader& shader,
//    const glm::mat4& view,
//    const glm::mat4& projection)
void ParticleSystemRender::RenderGrass(
    const std::vector<std::unique_ptr<Entity>>& entities,
    GrassBlade& grassBlade,
    GrassBlade& brokenGrassBlade,
    const GrassClump& grassClump,
    Shader& shader,
    const glm::mat4& view,
    const glm::mat4& projection)
{
    if (shader.ID() == 0)
        return;

    shader.Use();

    shader.setMat4("uView", view);
    shader.setMat4("uProjection", projection);

    shader.setVec3(
        "uLightDirection",
        glm::normalize(glm::vec3(0.5f, 1.0f, 0.4f))
    );

    shader.setVec3("uLightColor", glm::vec3(1.0f));
    shader.SetUniformFloat("uAmbientStrength", 0.35f);

    const GLboolean cullingEnabled =
        glIsEnabled(GL_CULL_FACE);

    glDisable(GL_CULL_FACE);

    std::vector<glm::mat4> allGrassMatrices;
    std::vector<glm::mat4> allBrokenGrassMatrices;

    // =================================================
    // CHECK ALL ENTITIES
    // =================================================

    for (const auto& entity : entities)
    {
        if (!entity)
            continue;

        const MeshEditing& mesh =
            entity->GetEditableMesh();

        const glm::mat4 entityModel =
            entity->GetModelMatrix();

        const auto& modifiers =
            entity->GetModifiers();

        // =============================================
        // CHECK PARTICLE MODIFIERS
        // =============================================

        for (std::size_t modifierIndex = 0;
            modifierIndex < modifiers.size();
            ++modifierIndex)
        {
            const ModifierData& modifier =
                modifiers[modifierIndex];

            if (modifier.type != ModifierType::Scatter ||
                !modifier.modEnabled)
            {
                continue;
            }

            const ParticleScatterData& scatter =
                modifier.scatter;

            // =========================================
            // EACH ASSIGNED FACE
            // =========================================

            for (std::size_t faceIndex : scatter.faceIndices)
            {
                if (faceIndex >= mesh.GetFaceCount())
                    continue;

                const float faceArea =
                    ParticleScatter::CalculateFaceArea(
                        mesh,
                        faceIndex,
                        entityModel
                    );

                if (faceArea <= 0.0f)
                    continue;

                // =====================================
                // EACH GRASS VARIETY
                // =====================================

                for (std::size_t entryIndex = 0;
                    entryIndex < scatter.entries.size();
                    ++entryIndex)
                {
                    const ScatterEntry& entry =
                        scatter.entries[entryIndex];

                    if (!entry.enabled ||
                        entry.type != ParticleScatterType::Grass)
                    {
                        continue;
                    }

                    // =================================
                    // DENSITY
                    // =================================

                    constexpr int MAX_CLUMPS = 10000;

                    const double requestedCount =
                        static_cast<double>(faceArea) *
                        static_cast<double>(
                            std::max(0.0f, entry.density)
                            );

                    const int clumpCount =
                        static_cast<int>(
                            std::clamp(
                                requestedCount,
                                0.0,
                                static_cast<double>(MAX_CLUMPS)
                            )
                            );

                    if (clumpCount <= 0)
                        continue;

                    // =================================
                    // INDEPENDENT SEED
                    // =================================

                    std::uint32_t entrySeed = scatter.seed;

                    entrySeed ^= entry.seedOffset +
                        0x9e3779b9u +
                        (entrySeed << 6) +
                        (entrySeed >> 2);

                    const std::uint32_t faceSeed =
                        static_cast<std::uint32_t>(faceIndex);

                    entrySeed ^= faceSeed +
                        0x85ebca6bu +
                        (entrySeed << 6) +
                        (entrySeed >> 2);

                    // =================================
                    // CACHED SCATTER POSITIONS
                    // =================================

                    const auto& points =
                        ParticleScatter::GetCachedOnFace(
                            entity->GetID(),
                            modifierIndex,
                            entryIndex,
                            mesh,
                            faceIndex,
                            clumpCount,
                            entrySeed
                        );

                    // =================================
                    // RANDOM VARIATION
                    // =================================

                    std::mt19937 random(
                        entrySeed ^ 0xA511E9B3u
                    );

                    const float minScale =
                        std::max(0.01f, entry.minScale);

                    const float maxScale =
                        std::max(minScale, entry.maxScale);

                    std::uniform_real_distribution<float>
                        randomScale(minScale, maxScale);

                    const float minRotation =
                        std::clamp(
                            entry.minRotation, 0.0f, 360.0f
                        );

                    const float maxRotation =
                        std::clamp(
                            entry.maxRotation,
                            minRotation,
                            360.0f
                        );

                    std::uniform_real_distribution<float>
                        randomRotation(minRotation, maxRotation);

                    // =================================
                    // CLUMP SETTINGS
                    // =================================

                    GrassClumpSettings settings;

                    settings.bladeCount = entry.bladeCount;
                    settings.spread = entry.bladeSpread;

                    settings.minHeight = entry.minBladeHeight;
                    settings.maxHeight = entry.maxBladeHeight;

                    settings.minWidth = entry.minBladeWidth;
                    settings.maxWidth = entry.maxBladeWidth;

                    settings.minTilt = entry.minBladeTilt;
                    settings.maxTilt = entry.maxBladeTilt;

                    // Temporary setting until we connect the panel.
                    //settings.brokenBladePercentage = 15.0f;

                    // Broken blade percentage from the particle settings.
                    settings.brokenBladePercentage =
                        entry.brokenBladePercentage;

                    // =================================
                    // GENERATE CLUMPS
                    // =================================

                    for (std::size_t pointIndex = 0;
                        pointIndex < points.size();
                        ++pointIndex)
                    {
                        const ScatterPoint& point =
                            points[pointIndex];

                        const glm::vec3 worldPosition =
                            glm::vec3(
                                entityModel *
                                glm::vec4(point.position, 1.0f)
                            );

                        // World-space surface normal.
                        const glm::mat3 normalMatrix =
                            glm::transpose(
                                glm::inverse(
                                    glm::mat3(entityModel)
                                )
                            );

                        glm::vec3 worldNormal =
                            normalMatrix * point.normal;

                        if (glm::dot(worldNormal, worldNormal) <
                            0.000001f)
                        {
                            worldNormal =
                                glm::vec3(0.0f, 1.0f, 0.0f);
                        }
                        else
                        {
                            worldNormal =
                                glm::normalize(worldNormal);
                        }

                        const glm::vec3 up(0.0f, 1.0f, 0.0f);

                        const float dotProduct =
                            glm::clamp(
                                glm::dot(up, worldNormal),
                                -1.0f,
                                1.0f
                            );

                        glm::quat surfaceRotation;

                        if (dotProduct > 0.9999f)
                        {
                            surfaceRotation =
                                glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
                        }
                        else if (dotProduct < -0.9999f)
                        {
                            surfaceRotation = glm::angleAxis(
                                glm::radians(180.0f),
                                glm::vec3(1.0f, 0.0f, 0.0f)
                            );
                        }
                        else
                        {
                            const glm::vec3 axis =
                                glm::normalize(
                                    glm::cross(up, worldNormal)
                                );

                            surfaceRotation =
                                glm::angleAxis(
                                    std::acos(dotProduct),
                                    axis
                                );
                        }

                        const glm::quat verticalRotation(
                            1.0f, 0.0f, 0.0f, 0.0f
                        );

                        const glm::quat finalRotation =
                            glm::normalize(
                                glm::slerp(
                                    verticalRotation,
                                    surfaceRotation,
                                    std::clamp(
                                        entry.surfaceAlignment,
                                        0.0f,
                                        1.0f
                                    )
                                )
                            );

                        glm::mat4 model(1.0f);

                        model = glm::translate(
                            model,
                            worldPosition
                        );

                        model *= glm::mat4_cast(finalRotation);

                        model = glm::rotate(
                            model,
                            glm::radians(randomRotation(random)),
                            up
                        );

                        model = glm::scale(
                            model,
                            glm::vec3(randomScale(random))
                        );

                        // Unique blade arrangement for each clump.
                        std::uint32_t clumpSeed = entrySeed;

                        const std::uint32_t indexSeed =
                            static_cast<std::uint32_t>(pointIndex);

                        clumpSeed ^= indexSeed +
                            0x9e3779b9u +
                            (clumpSeed << 6) +
                            (clumpSeed >> 2);

                        grassClump.AppendMixedInstanceMatrices(
                            model,
                            settings,
                            clumpSeed,
                            allGrassMatrices,
                            allBrokenGrassMatrices
                        );

                        /*grassClump.AppendInstanceMatrices(
                            model,
                            settings,
                            clumpSeed,
                            allGrassMatrices
                        );*/
                    }
                }
            }
        }
    }

    // =================================================
    // GPU INSTANCE BUFFER CACHING
    // =================================================

    // =================================================
// GPU INSTANCE BUFFER CACHING
// =================================================

// Compare two collections of instance matrices.
    auto matricesChanged = [](
        const std::vector<glm::mat4>& current,
        const std::vector<glm::mat4>& cached) -> bool
    {
        if (current.size() != cached.size())
            return true;

        for (std::size_t i = 0; i < current.size(); ++i)
        {
            for (int column = 0; column < 4; ++column)
            {
                if (!glm::all(glm::equal(
                    current[i][column],
                    cached[i][column])))
                {
                    return true;
                }
            }
        }

        return false;
    };

    // -------------------------------------------------
    // Normal grass cache
    // -------------------------------------------------

    if (matricesChanged(
        allGrassMatrices,
        m_cachedGrassMatrices))
    {
        grassBlade.UploadInstanceMatrices(
            allGrassMatrices
        );

        m_cachedGrassMatrices =
            std::move(allGrassMatrices);
    }

    // -------------------------------------------------
    // Broken grass cache
    // -------------------------------------------------

    if (matricesChanged(
        allBrokenGrassMatrices,
        m_cachedBrokenGrassMatrices))
    {
        brokenGrassBlade.UploadInstanceMatrices(
            allBrokenGrassMatrices
        );

        m_cachedBrokenGrassMatrices =
            std::move(allBrokenGrassMatrices);
    }

    // =================================================
    // DRAW INSTANCED GRASS
    // =================================================

    shader.Use();

    shader.SetUniformInt(
        "uUseInstancing",
        1
    );

    shader.setMat4(
        "uModel",
        glm::mat4(1.0f)
    );

    // Draw all normal blades.
    if (!m_cachedGrassMatrices.empty())
    {
        grassBlade.DrawCachedInstances();
    }

    // Draw all broken blades.
    if (!m_cachedBrokenGrassMatrices.empty())
    {
        brokenGrassBlade.DrawCachedInstances();
    }

    shader.SetUniformInt(
        "uUseInstancing",
        0
    );

    //bool grassChanged =
    //    allGrassMatrices.size() !=
    //    m_cachedGrassMatrices.size();

    //if (!grassChanged)
    //{
    //    for (std::size_t i = 0;
    //        i < allGrassMatrices.size();
    //        ++i)
    //    {
    //        for (int column = 0; column < 4; ++column)
    //        {
    //            if (!glm::all(glm::equal(
    //                allGrassMatrices[i][column],
    //                m_cachedGrassMatrices[i][column])))
    //            {
    //                grassChanged = true;
    //                break;
    //            }
    //        }

    //        if (grassChanged)
    //            break;
    //    }
    //}

    //if (grassChanged)
    //{
    //    grassBlade.UploadInstanceMatrices(
    //        allGrassMatrices
    //    );

    //    m_cachedGrassMatrices =
    //        std::move(allGrassMatrices);
    //}

    //// =================================================
    //// DRAW CACHED GRASS
    //// =================================================

    //if (!m_cachedGrassMatrices.empty())
    //{
    //    shader.Use();

    //    shader.SetUniformInt("uUseInstancing", 1);

    //    shader.setMat4(
    //        "uModel",
    //        glm::mat4(1.0f)
    //    );

    //    grassBlade.DrawCachedInstances();

    //    shader.SetUniformInt("uUseInstancing", 0);
    //}

    // Restore the previous face-culling state.
    if (cullingEnabled)
    {
        glEnable(GL_CULL_FACE);
    }
}

// =====================================================
// CLEAR GRASS CACHE
// =====================================================
void ParticleSystemRender::ClearGrassCache()
{
    m_cachedGrassMatrices.clear();
    m_cachedBrokenGrassMatrices.clear();
}


//void ParticleSystemRender::ClearGrassCache()
//{
//    m_cachedGrassMatrices.clear();
//}
