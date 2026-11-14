#include "mesh/ParticleSystem/ParticleSystemRender.h"
#include <miniBoxLog.h>
#include <entity/Entity.h>
#include <mesh/MeshEditing.h>
#include <mesh/ParticleSystem/ParticleScatter.h>
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
#include <set>
#include <utility>
#include <array>




#include <cstddef>
#include <string>
#include <filesystem>
#include <system_error>


namespace
{
    bool MatricesChanged(
        const std::vector<glm::mat4>& current,
        const std::vector<glm::mat4>& cached)
    {
        if (current.size() != cached.size())
            return true;

        for (std::size_t i = 0; i < current.size(); ++i)
        {
            for (int column = 0; column < 4; ++column)
            {
                if (!glm::all(glm::equal(current[i][column],
                    cached[i][column])))
                    return true;
            }
        }
        return false;
    }
}

void ParticleSystemRender::RenderGrass(
    const std::vector<std::unique_ptr<Entity>>& entities,
    GrassBlade& grassBlade,
    GrassBlade& brokenGrassBlade,
    const GrassClump& grassClump,
    Shader& shader,
    const glm::mat4& view,
    const glm::mat4& projection)
{
    // These original shared meshes are still owned by BoxEngine.
    // The new per-variety meshes below are used for scattered grass.
    (void)grassBlade;
    (void)brokenGrassBlade;

    if (shader.ID() == 0)
        return;

    shader.Use();
    shader.setMat4("uView", view);
    shader.setMat4("uProjection", projection);
    shader.setVec3("uLightDirection",
        glm::normalize(glm::vec3(0.5f, 1.0f, 0.4f)));
    shader.setVec3("uLightColor", glm::vec3(1.0f));
    shader.SetUniformFloat("uAmbientStrength", 0.35f);

    const GLboolean cullingEnabled = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    for (auto& [key, variant] : m_grassVariants)
    {
        variant.pendingNormal.clear();
        variant.pendingBroken.clear();
    }

    std::set<GrassKey> activeKeys;

    for (const auto& entity : entities)
    {
        if (!entity)
            continue;

        const MeshEditing& mesh = entity->GetEditableMesh();
        const glm::mat4 entityModel = entity->GetModelMatrix();
        const auto& modifiers = entity->GetModifiers();

        for (std::size_t modifierIndex = 0;
            modifierIndex < modifiers.size(); ++modifierIndex)
        {
            const ModifierData& modifier = modifiers[modifierIndex];
            if (modifier.type != ModifierType::Scatter || !modifier.modEnabled)
                continue;

            const ParticleScatterData& scatter = modifier.scatter;
            
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

                    // A mesh pair belongs to this entity/modifier/entry.
                    const GrassKey key{
                        entity->GetID(), modifierIndex, entryIndex
                    };
                    GrassVariant& variant = m_grassVariants[key];
                    activeKeys.insert(key);

                    // Geometry is created once and rebuilt only if
                    // one of the four shape controls changes.
                    const std::array<float, 4> shape = {
                        entry.bladeCurvature,
                        entry.bladeConcavity,
                        entry.bladeBreakPosition,
                        entry.bladeBreakAngle
                    };

                    if (!variant.geometryReady || variant.shape != shape)
                    {
                        if (!variant.normal)
                            variant.normal = std::make_unique<GrassBlade>();
                        if (!variant.broken)
                            variant.broken = std::make_unique<GrassBlade>();

                        variant.normal->SetSize(0.01f, 0.20f);
                        variant.normal->SetCurvature(shape[0]);
                        variant.normal->SetConcavity(shape[1]);

                        variant.broken->SetSize(0.01f, 0.20f);
                        variant.broken->SetCurvature(shape[0]);
                        variant.broken->SetConcavity(shape[1]);
                        variant.broken->SetBreakPosition(shape[2]);
                        variant.broken->SetBreakAngle(shape[3]);

                        const bool normalOK = variant.normal->CreateGrassBlade();
                        const bool brokenOK = variant.broken->CreateBrokenGrassBlade();
                        variant.geometryReady = normalOK && brokenOK;
                        if (!variant.geometryReady)
                            continue;

                        variant.shape = shape;
                        variant.forceUpload = true;
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
                            1.0f, 0.0f, 0.0f, 0.0f);
                        // ##

// ============================================
// CHECK ROCK EXCLUSION ZONES
// ============================================

                        bool blockedByRock = false;

                        // Allow a little space for the grass blades
                        // to spread beyond the centre of the clump.
                        const float clumpClearance =
                            std::max(0.0f, entry.bladeSpread) *
                            std::max(1.0f, entry.maxScale);

                        for (const RockExclusionZone& zone :
                            m_rockExclusionZones)
                        {
                            // Only check rocks on the same object
                            // and the same selected face.
                            if (zone.entityID != entity->GetID())
                                continue;

                            if (zone.faceIndex != faceIndex)
                                continue;

                            // Vector from the rock centre
                            // to this grass clump.
                            glm::vec3 difference =
                                worldPosition - zone.position;

                            // Remove the component along the surface
                            // normal. This gives us distance across
                            // the surface, even on a slope.
                            difference -=
                                glm::dot(difference, worldNormal) *
                                worldNormal;

                            const float exclusionDistance =
                                zone.radius + clumpClearance;

                            if (glm::dot(difference, difference) <
                                exclusionDistance * exclusionDistance)
                            {
                                blockedByRock = true;
                                break;
                            }
                        }

                        // Skip this clump entirely if it would
                        // grow inside a rock.
                        if (blockedByRock)
                            continue;



                        // ##
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
                            variant.pendingNormal,
                            variant.pendingBroken
                        );

                    }
                }
            }
        }
    }


    // Upload only changed instance lists, then draw each shape batch.
    shader.Use();
    shader.SetUniformInt("uUseInstancing", 1);
    shader.setMat4("uModel", glm::mat4(1.0f));

    for (const GrassKey& key : activeKeys)
    {
        GrassVariant& variant = m_grassVariants.at(key);
        if (!variant.geometryReady)
            continue;

        if (variant.forceUpload || MatricesChanged(
            variant.pendingNormal, variant.cachedNormal))
        {
            variant.normal->UploadInstanceMatrices(variant.pendingNormal);
            variant.cachedNormal = std::move(variant.pendingNormal);
        }
        if (variant.forceUpload || MatricesChanged(
            variant.pendingBroken, variant.cachedBroken))
        {
            variant.broken->UploadInstanceMatrices(variant.pendingBroken);
            variant.cachedBroken = std::move(variant.pendingBroken);
        }

        variant.forceUpload = false;

        if (!variant.cachedNormal.empty())
            variant.normal->DrawCachedInstances();
        if (!variant.cachedBroken.empty())
            variant.broken->DrawCachedInstances();
    }

    shader.SetUniformInt("uUseInstancing", 0);

    // Removing/disabling an entry deletes its pair of OpenGL meshes and buffers.
    for (auto it = m_grassVariants.begin(); it != m_grassVariants.end(); )
    {
        if (activeKeys.find(it->first) == activeKeys.end())
            it = m_grassVariants.erase(it);
        else
            ++it;
    }

    if (cullingEnabled)
        glEnable(GL_CULL_FACE);
}

// Call during BoxEngine shutdown BEFORE destroying the OpenGL context.
void ParticleSystemRender::ClearGrassCache()
{
    m_grassVariants.clear();
}



bool ParticleSystemRender::LoadSmallRock()
{
    // Already loaded? Nothing else to do.
    if (m_smallRockLoaded)
    {
        return true;
    }

    // ============================================
    // LOAD OUR FIRST ROCK ASSET
    // ============================================

    const std::filesystem::path rockPath = "assets/rocks/rock_01.mbx";

    mbxManager manager;

    MBXImportData importedRock;

    if (!manager.ImportMBX(rockPath, importedRock))
    {
        BOX_LOG_ERROR(
            "ParticleSystemRender: Failed to load "
            << rockPath.string()
        );

        return false;
    }

    // ============================================
    // CHECK THE MESH
    // ============================================

    if (!importedRock.mesh.IsValid())
    {
        BOX_LOG_ERROR(
            "ParticleSystemRender: Invalid rock mesh"
        );

        return false;
    }

    // ============================================
    // KEEP THE ROCK IN MEMORY
    // ============================================

    m_smallRockAsset = std::move(importedRock);

    m_smallRockLoaded = true;

    // ============================================
    // DEBUG INFORMATION
    // ============================================

    BOX_LOG_INFO(
        "Rock Asset Loaded: "
        << m_smallRockAsset.objectName
    );

    BOX_LOG_INFO(
        "Rock Vertices: "
        << m_smallRockAsset.mesh.vertices.size()
    );

    BOX_LOG_INFO(
        "Rock Materials: "
        << m_smallRockAsset.materials.size()
    );

    BOX_LOG_INFO(
        "Rock Scale: "
        << m_smallRockAsset.scale.x << ", "
        << m_smallRockAsset.scale.y << ", "
        << m_smallRockAsset.scale.z
    );

    return true;
}


bool ParticleSystemRender::CreateSmallRockBuffers()
{
    if (m_rockBuffersReady)
        return true;

    if (!m_smallRockLoaded)
        return false;

    const MeshData& mesh = m_smallRockAsset.mesh;

    if (!mesh.IsValid())
        return false;

    // =========================================
    // CREATE GPU BUFFERS
    // =========================================

    glGenVertexArrays(1, &m_rockVAO);
    glGenBuffers(1, &m_rockVBO);
    glGenBuffers(1, &m_rockInstanceVBO);

    glBindVertexArray(m_rockVAO);

    // -----------------------------------------
    // Upload the rock geometry
    // -----------------------------------------

    glBindBuffer(GL_ARRAY_BUFFER, m_rockVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        mesh.vertices.size() * sizeof(MeshVertex),
        mesh.vertices.data(),
        GL_STATIC_DRAW
    );

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        sizeof(MeshVertex),
        (void*)offsetof(MeshVertex, position)
    );

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        sizeof(MeshVertex),
        (void*)offsetof(MeshVertex, normal)
    );

    // UV
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE,
        sizeof(MeshVertex),
        (void*)offsetof(MeshVertex, uv)
    );

    // Material index
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(
        3, 1, GL_UNSIGNED_INT,
        sizeof(MeshVertex),
        (void*)offsetof(MeshVertex, materialIndex)
    );

    // Tangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(
        4, 3, GL_FLOAT, GL_FALSE,
        sizeof(MeshVertex),
        (void*)offsetof(MeshVertex, tangent)
    );

    // =========================================
    // INSTANCE MATRICES
    // =========================================

    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_rockInstanceVBO
    );

    // Allocated properly when instances are uploaded.
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(glm::mat4),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    // A mat4 uses four vec4 attributes.
    for (GLuint column = 0; column < 4; ++column)
    {
        const GLuint location = 5 + column;

        glEnableVertexAttribArray(location);

        glVertexAttribPointer(
            location,
            4,
            GL_FLOAT,
            GL_FALSE,
            sizeof(glm::mat4),
            (void*)(sizeof(glm::vec4) * column)
        );

        // Advance once for each rock instance.
        glVertexAttribDivisor(location, 1);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_rockVertexCount = static_cast<GLsizei>(
        mesh.vertices.size()
        );

    m_rockBuffersReady = true;

    BOX_LOG_INFO(
        "Small Rock GPU buffers created. Vertices: "
        << m_rockVertexCount
    );



    return true;
}


void ParticleSystemRender::RenderRocks(
    const std::vector<std::unique_ptr<Entity>>& entities,
    Shader& shader,
    const glm::mat4& view,
    const glm::mat4& projection,
    const glm::vec3& cameraPosition,
    GLuint rockAlbedoTexture)
{
    if (shader.ID() == 0 || m_rockLibrary.empty())
        return;

    // Discard exclusion zones from the previous frame.
    m_rockExclusionZones.clear();

    // ============================================
    // PREPARE THE SCENE SHADER
    // ============================================

    shader.Use();

    shader.setMat4("uView", view);
    shader.setMat4("uProjection", projection);
    shader.setMat4("uModel", glm::mat4(1.0f));

    shader.setVec3(
        "uCameraPosition",
        cameraPosition
    );

    shader.setVec3(
        "uLightPosition",
        glm::vec3(3.0f, 4.0f, 3.0f)
    );

    shader.setVec3(
        "uLightColor",
        glm::vec3(1.0f)
    );

    shader.SetUniformInt("uUseInstancing", 1);

    const GLboolean cullingEnabled =
        glIsEnabled(GL_CULL_FACE);

    glDisable(GL_CULL_FACE);

    // ============================================
    // LOOP THROUGH EVERY ROCK MODEL
    // ============================================

    for (std::size_t rockIndex = 0;
        rockIndex < m_rockLibrary.size();
        ++rockIndex)
    {
        RockAsset& rock = m_rockLibrary[rockIndex];

        if (!rock.gpuReady)
            continue;

        rock.instanceMatrices.clear();

        // ============================================
        // CALCULATE THIS ROCK'S FOOTPRINT RADIUS
        // ============================================

        float rockRadius = 0.0f;

        for (const MeshVertex& vertex :
            rock.data.mesh.vertices)
        {
            const float distance =
                glm::length(
                    glm::vec2(
                        vertex.position.x,
                        vertex.position.z
                    )
                );

            rockRadius = std::max(rockRadius, distance);
        }




        // ========================================
        // FIND ALL SURFACE SCATTER MODIFIERS
        // ========================================

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

            const glm::mat3 normalMatrix =
                glm::transpose(
                    glm::inverse(
                        glm::mat3(entityModel)
                    )
                );

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

                const ParticleScatterData& scatter = modifier.scatter;

                // ##

// ============================================
// FIND SETTINGS FOR THIS ROCK MODEL
// ============================================

                const ScatterEntry* rockEntry = nullptr;

                for (const ScatterEntry& entry : scatter.entries)
                {
                    if (entry.type != ParticleScatterType::Rocks)
                        continue;

                    if (entry.assetID != static_cast<int>(rockIndex))
                        continue;

                    if (!entry.enabled)
                        continue;

                    rockEntry = &entry;
                    break;
                }

                // This Surface Scatter modifier does not
                // contain an enabled entry for this rock.
                if (!rockEntry)
                    continue;

                // Each shape has its own independent count.
                const int rocksPerFace =
                    std::clamp(rockEntry->rockCount, 0, 1000);

                if (rocksPerFace == 0)
                    continue;



                //##


                // ====================================
                // EACH SELECTED FACE
                // ====================================

                for (std::size_t faceIndex :
                scatter.faceIndices)
                {
                    if (faceIndex >= mesh.GetFaceCount())
                        continue;

                    // Independent seed for each model,
                    // entity, modifier and selected face.
                    std::uint32_t seed = scatter.seed;

                  

                    auto MixSeed = [&seed](std::uint32_t value)
                    {
                        seed ^= value +
                            0x9e3779b9u +
                            (seed << 6) +
                            (seed >> 2);
                    };

                    MixSeed(
                        static_cast<std::uint32_t>(
                            entity->GetID()
                            )
                    );

                    MixSeed(
                        static_cast<std::uint32_t>(
                            modifierIndex
                            )
                    );

                    MixSeed(
                        static_cast<std::uint32_t>(
                            rockIndex
                            )
                    );

                    MixSeed(
                        static_cast<std::uint32_t>(
                            faceIndex
                            )
                    );

                    MixSeed(rockEntry->seedOffset);

                    // ====================================
                    // GENERATE SCATTER POINTS
                    // ====================================

                    const auto points =
                        ParticleScatter::GenerateOnFace(
                            mesh,
                            faceIndex,
                            rocksPerFace,
                            seed
                        );

                    std::mt19937 random(
                        seed ^ 0xA511E9B3u
                    );

                    // ============================================
                    // SHARED ROCK VARIATION SETTINGS
                    // ============================================

                    const float minScale =
                        std::max(0.01f, scatter.minScale);

                    const float maxScale =
                        std::max(minScale, scatter.maxScale);

                    const float maxRotation =
                        std::clamp(
                            scatter.randomRotation,
                            0.0f,
                            360.0f
                        );

                    std::uniform_real_distribution<float>
                        randomRotation(0.0f, maxRotation);

                    std::uniform_real_distribution<float>
                        randomScale(minScale, maxScale);

                   /* std::uniform_real_distribution<float>
                        randomRotation(0.0f, 360.0f);

                    std::uniform_real_distribution<float>
                        randomScale(0.8f, 1.2f);*/

                    // ====================================
                    // BUILD INSTANCE MATRICES
                    // ====================================

                    for (const ScatterPoint& point : points)
                    {
                        const glm::vec3 worldPosition =
                            glm::vec3(
                                entityModel *
                                glm::vec4(
                                    point.position,
                                    1.0f
                                )
                            );

                        glm::vec3 worldNormal = normalMatrix * point.normal;

                        // ============================================
                        // APPLY SURFACE OFFSET
                        // ============================================

                        const glm::vec3 offsetPosition =
                            worldPosition +
                            worldNormal * scatter.rockSurfaceOffset;

                        if (glm::dot(
                            worldNormal,
                            worldNormal) < 0.000001f)
                        {
                            worldNormal =
                                glm::vec3(0, 1, 0);
                        }
                        else
                        {
                            worldNormal =
                                glm::normalize(worldNormal);
                        }

                        const glm::vec3 up(0, 1, 0);

                        const float dotProduct =
                            glm::clamp(
                                glm::dot(up, worldNormal),
                                -1.0f,
                                1.0f
                            );

                        glm::quat surfaceRotation(
                            1.0f, 0.0f, 0.0f, 0.0f
                        );

                        if (dotProduct < -0.9999f)
                        {
                            surfaceRotation =
                                glm::angleAxis(
                                    glm::radians(180.0f),
                                    glm::vec3(1, 0, 0)
                                );
                        }
                        else if (dotProduct < 0.9999f)
                        {
                            const glm::vec3 axis =
                                glm::normalize(
                                    glm::cross(
                                        up,
                                        worldNormal
                                    )
                                );

                            surfaceRotation =
                                glm::angleAxis(
                                    std::acos(dotProduct),
                                    axis
                                );
                        }

                        // --------------------------------
                        // MODEL MATRIX
                        // --------------------------------

                        glm::mat4 model(1.0f);

                        model = glm::translate(
                            model,
                            offsetPosition
                        );

                        model *= glm::mat4_cast(
                            surfaceRotation
                        );

                        model = glm::rotate(
                            model,
                            glm::radians(
                                randomRotation(random)
                            ),
                            up
                        );

                        const float variation =
                            randomScale(random);

                        // Preserve this rock's own
                        // saved MBX scale.
                        model = glm::scale(
                            model,
                            rock.data.scale * variation
                        );

                        rock.instanceMatrices.push_back(model);


                        // ============================================
                        // CREATE GRASS EXCLUSION ZONE
                        // ============================================

                        RockExclusionZone zone;

                        zone.entityID = entity->GetID();
                        zone.faceIndex = faceIndex;

                        // Use the actual world-space position of
                        // this rock, including Surface Offset.
                        zone.position = glm::vec3(model[3]);

                        // The instance matrix already contains the
                        // MBX scale and random size variation.
                        const float scaleX =
                            glm::length(glm::vec3(model[0]));

                        const float scaleZ =
                            glm::length(glm::vec3(model[2]));

                        const float instanceScale =
                            std::max(scaleX, scaleZ);

                        // A small extra margin around each rock.
                        
                        // ============================================
                        // ADJUSTABLE GRASS CLEARANCE
                        // ============================================

                        const float extraClearance =
                            std::max(0.0f, scatter.grassClearance);

                        zone.radius =
                            rockRadius * instanceScale +
                            extraClearance;
                        
                        /*constexpr float extraClearance = 0.03f;

                        zone.radius =
                            rockRadius * instanceScale +
                            extraClearance;*/

                        m_rockExclusionZones.push_back(zone);




                    }
                }
            }
        }

        // No instances for this rock?
        if (rock.instanceMatrices.empty())
            continue;

        // ========================================
        // UPLOAD THIS ROCK'S INSTANCE MATRICES
        // ========================================

        glBindBuffer(
            GL_ARRAY_BUFFER,
            rock.instanceVBO
        );

        glBufferData(
            GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(
                rock.instanceMatrices.size() *
                sizeof(glm::mat4)
                ),
            rock.instanceMatrices.data(),
            GL_DYNAMIC_DRAW
        );

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // ========================================
        // SET MATERIALS FOR THIS ROCK
        // ========================================

        shader.Use();

        for (int slot = 0; slot < 8; ++slot)
        {
            const std::string suffix =
                "[" + std::to_string(slot) + "]";

            glm::vec4 color(0.75f, 0.75f, 0.75f, 1.0f);
            float metallic = 0.0f;
            float roughness = 0.5f;

            bool useAlbedo = false;

            if (static_cast<std::size_t>(slot) <
                rock.data.materials.size())
            {
                const MBXMaterialData& material =
                    rock.data.materials[slot];

                color = material.baseColor;
                metallic = material.metallic;
                roughness = material.roughness;

                useAlbedo =
                    material.useBaseColorTexture &&
                    rockAlbedoTexture != 0;
            }

            shader.setVec4(
                ("uMaterialColors" + suffix).c_str(),
                color
            );

            shader.SetUniformFloat(
                ("uMaterialMetallic" + suffix).c_str(),
                metallic
            );

            shader.SetUniformFloat(
                ("uMaterialRoughness" + suffix).c_str(),
                roughness
            );

            shader.SetUniformInt(
                ("uMaterialUsesTexture" + suffix).c_str(),
                useAlbedo ? 1 : 0
            );

            shader.SetUniformInt(
                ("uMaterialTextures" + suffix).c_str(),
                slot
            );

            shader.SetUniformInt(
                ("uMaterialUsesNormalTexture" + suffix).c_str(),
                0
            );

            shader.SetUniformInt(
                ("uMaterialUsesRoughnessTexture" + suffix).c_str(),
                0
            );

            shader.SetUniformInt(
                ("uMaterialUsesAOTexture" + suffix).c_str(),
                0
            );

            shader.SetUniformInt(
                ("uMaterialType" + suffix).c_str(),
                0
            );

            shader.SetUniformFloat(
                ("uMaterialTransmission" + suffix).c_str(),
                0.0f
            );

            shader.SetUniformFloat(
                ("uMaterialIOR" + suffix).c_str(),
                1.5f
            );

            shader.setVec3(
                ("uMaterialEmissionColor" + suffix).c_str(),
                glm::vec3(0.0f)
            );

            shader.SetUniformFloat(
                ("uMaterialEmissionStrength" + suffix).c_str(),
                0.0f
            );

            // All five rocks currently share the
            // same Stone_albedo.png texture.
            glActiveTexture(GL_TEXTURE0 + slot);

            glBindTexture(
                GL_TEXTURE_2D,
                useAlbedo ? rockAlbedoTexture : 0
            );
        }

        // ========================================
        // DRAW THIS ROCK TYPE
        // ========================================

        glBindVertexArray(rock.VAO);

        glDrawArraysInstanced(
            GL_TRIANGLES,
            0,
            rock.vertexCount,
            static_cast<GLsizei>(
                rock.instanceMatrices.size()
                )
        );

        glBindVertexArray(0);
    }

    // ============================================
    // CLEAN UP RENDER STATE
    // ============================================

    for (GLuint slot = 0; slot < 8; ++slot)
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glActiveTexture(GL_TEXTURE0);

    if (cullingEnabled)
        glEnable(GL_CULL_FACE);

    shader.SetUniformInt("uUseInstancing", 0);
}


void ParticleSystemRender::ClearRockCache()
{
    if (m_rockInstanceVBO != 0)
    {
        glDeleteBuffers(1, &m_rockInstanceVBO);
        m_rockInstanceVBO = 0;
    }

    if (m_rockVBO != 0)
    {
        glDeleteBuffers(1, &m_rockVBO);
        m_rockVBO = 0;
    }

    if (m_rockVAO != 0)
    {
        glDeleteVertexArrays(1, &m_rockVAO);
        m_rockVAO = 0;
    }

    m_rockVertexCount = 0;
    m_rockBuffersReady = false;

    // ============================================
    // CLEAR ROCK LIBRARY GPU BUFFERS
    // ============================================

    for (RockAsset& rock : m_rockLibrary)
    {
        if (rock.instanceVBO != 0)
        {
            glDeleteBuffers(1, &rock.instanceVBO);
            rock.instanceVBO = 0;
        }

        if (rock.VBO != 0)
        {
            glDeleteBuffers(1, &rock.VBO);
            rock.VBO = 0;
        }

        if (rock.VAO != 0)
        {
            glDeleteVertexArrays(1, &rock.VAO);
            rock.VAO = 0;
        }

        rock.vertexCount = 0;
        rock.gpuReady = false;

        rock.instanceMatrices.clear();
    }

    m_rockLibrary.clear();
    m_rockExclusionZones.clear();
}


std::filesystem::path
ParticleSystemRender::GetSmallRockAlbedoPath() const
{
    if (!m_smallRockLoaded)
        return {};

    for (const MBXMaterialData& material :
        m_smallRockAsset.materials)
    {
        if (material.useBaseColorTexture &&
            !material.baseColorTexturePath.empty())
        {
            return material.baseColorTexturePath;
        }
    }

    return {};
}


bool ParticleSystemRender::LoadRockLibrary(const std::filesystem::path& folderPath)
{
    namespace fs = std::filesystem;

    // ============================================
    // CHECK FOLDER
    // ============================================

    std::error_code error;

    if (!fs::is_directory(folderPath, error))
    {
        BOX_LOG_ERROR(
            "Rock Library folder not found: "
            << folderPath.string()
        );

        return false;
    }

    // ============================================
    // FIND ALL MBX FILES
    // ============================================

    std::vector<fs::path> rockFiles;

    fs::directory_iterator iterator(
        folderPath,
        fs::directory_options::skip_permission_denied,
        error
    );

    if (error)
    {
        BOX_LOG_ERROR(
            "Failed to read Rock Library: "
            << error.message()
        );

        return false;
    }

    const fs::directory_iterator end;

    while (iterator != end)
    {
        if (iterator->is_regular_file(error))
        {
            fs::path path = iterator->path();

            if (path.extension() == ".mbx")
            {
                rockFiles.push_back(path);
            }
        }

        if (error)
        {
            BOX_LOG_ERROR(
                "Rock Library read error: "
                << error.message()
            );

            return false;
        }

        iterator.increment(error);

        if (error)
        {
            BOX_LOG_ERROR(
                "Rock Library iteration error: "
                << error.message()
            );

            return false;
        }
    }

    // Keep asset indices consistent.
    std::sort(
        rockFiles.begin(),
        rockFiles.end()
    );

    // ============================================
    // LOAD ALL ROCK MODELS
    // ============================================

    std::vector<RockAsset> loadedAssets;

    mbxManager manager;

    for (const fs::path& rockPath : rockFiles)
    {
        MBXImportData importedRock;

        if (!manager.ImportMBX(
            rockPath,
            importedRock))
        {
            BOX_LOG_ERROR(
                "Failed to import rock: "
                << rockPath.string()
            );

            continue;
        }

        if (!importedRock.mesh.IsValid())
        {
            BOX_LOG_ERROR(
                "Invalid rock mesh: "
                << rockPath.string()
            );

            continue;
        }

        RockAsset asset;

        asset.name =
            rockPath.stem().string();

        asset.filePath = rockPath;

        asset.data =
            std::move(importedRock);

        BOX_LOG_INFO(
            "Rock Library: "
            << asset.name
            << " Vertices: "
            << asset.data.mesh.vertices.size()
        );

        loadedAssets.push_back(
            std::move(asset)
        );
    }

    if (loadedAssets.empty())
    {
        BOX_LOG_ERROR(
            "No valid MBX rocks found in: "
            << folderPath.string()
        );

        return false;
    }

    // Replace the previous library only
    // after successfully loading assets.
    m_rockLibrary = std::move(loadedAssets);

    BOX_LOG_INFO(
        "Rock Library loaded "
        << m_rockLibrary.size()
        << " models"
    );

    return true;
}


bool ParticleSystemRender::CreateRockLibraryBuffers()
{
    if (m_rockLibrary.empty())
    {
        BOX_LOG_ERROR("Rock Library is empty");
        return false;
    }

    // ============================================
    // CREATE GPU BUFFERS FOR EVERY ROCK
    // ============================================

    for (RockAsset& rock : m_rockLibrary)
    {
        // Already uploaded?
        if (rock.gpuReady)
            continue;

        const MeshData& mesh = rock.data.mesh;

        if (!mesh.IsValid())
        {
            BOX_LOG_ERROR(
                "Invalid mesh: " << rock.name
            );

            return false;
        }

        // ========================================
        // CREATE OPENGL OBJECTS
        // ========================================

        glGenVertexArrays(1, &rock.VAO);
        glGenBuffers(1, &rock.VBO);
        glGenBuffers(1, &rock.instanceVBO);

        if (rock.VAO == 0 ||
            rock.VBO == 0 ||
            rock.instanceVBO == 0)
        {
            BOX_LOG_ERROR(
                "Failed to create buffers: "
                << rock.name
            );

            return false;
        }

        glBindVertexArray(rock.VAO);

        // ========================================
        // UPLOAD ROCK GEOMETRY
        // ========================================

        glBindBuffer(GL_ARRAY_BUFFER, rock.VBO);

        glBufferData(
            GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(
                mesh.vertices.size() * sizeof(MeshVertex)
                ),
            mesh.vertices.data(),
            GL_STATIC_DRAW
        );

        // Position - location 0
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE,
            sizeof(MeshVertex),
            (void*)offsetof(MeshVertex, position)
        );

        // Normal - location 1
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE,
            sizeof(MeshVertex),
            (void*)offsetof(MeshVertex, normal)
        );

        // UV - location 2
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(
            2, 2, GL_FLOAT, GL_FALSE,
            sizeof(MeshVertex),
            (void*)offsetof(MeshVertex, uv)
        );

        // Material index - location 3
        glEnableVertexAttribArray(3);
        glVertexAttribIPointer(
            3, 1, GL_UNSIGNED_INT,
            sizeof(MeshVertex),
            (void*)offsetof(MeshVertex, materialIndex)
        );

        // Tangent - location 4
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(
            4, 3, GL_FLOAT, GL_FALSE,
            sizeof(MeshVertex),
            (void*)offsetof(MeshVertex, tangent)
        );

        // ========================================
        // INSTANCE MATRIX BUFFER
        // ========================================

        glBindBuffer(
            GL_ARRAY_BUFFER,
            rock.instanceVBO
        );

        // Allocate space for an initial matrix.
        // We'll upload the actual matrices later.
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(glm::mat4),
            nullptr,
            GL_DYNAMIC_DRAW
        );

        for (GLuint column = 0; column < 4; ++column)
        {
            const GLuint location = 5 + column;

            glEnableVertexAttribArray(location);

            glVertexAttribPointer(
                location,
                4,
                GL_FLOAT,
                GL_FALSE,
                sizeof(glm::mat4),
                (void*)(sizeof(glm::vec4) * column)
            );

            // One matrix per rock instance.
            glVertexAttribDivisor(location, 1);
        }

        // ========================================
        // FINISH THIS ROCK
        // ========================================

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        rock.vertexCount =
            static_cast<GLsizei>(
                mesh.vertices.size()
                );

        rock.gpuReady = true;

        BOX_LOG_INFO(
            "Rock GPU Ready: "
            << rock.name
            << " Vertices: "
            << rock.vertexCount
        );
    }

    BOX_LOG_INFO(
        "Rock Library GPU buffers initialized"
    );

    return true;
}
