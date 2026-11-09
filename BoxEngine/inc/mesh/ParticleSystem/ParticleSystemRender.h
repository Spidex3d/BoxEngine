#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class Entity;
class Shader;
class GrassBlade;
class GrassClump;

// =====================================================
// PARTICLE SYSTEM RENDERER
// =====================================================

class ParticleSystemRender
{
public:

    ParticleSystemRender() = default;
    ~ParticleSystemRender() = default;

    // =================================================
    // GRASS
    // =================================================

    void RenderGrass(
        const std::vector<std::unique_ptr<Entity>>& entities,
        GrassBlade& grassBlade,
        GrassBlade& brokenGrassBlade,
        const GrassClump& grassClump,
        Shader& shader,
        const glm::mat4& view,
        const glm::mat4& projection
    );

    /*void RenderGrass(
        const std::vector<std::unique_ptr<Entity>>& entities,
        GrassBlade& grassBlade,
        const GrassClump& grassClump,
        Shader& shader,
        const glm::mat4& view,
        const glm::mat4& projection
    );*/

    // =================================================
    // FUTURE PARTICLE TYPES
    // =================================================

    void RenderRocks();
    void RenderPlants();
    void RenderTrees();
    void RenderFire();

    // =================================================
    // CACHE MANAGEMENT
    // =================================================

    void ClearGrassCache();

private:

    // Last grass transformations uploaded to the GPU.
    std::vector<glm::mat4> m_cachedGrassMatrices;
    // GPU cache for broken grass blades.
    std::vector<glm::mat4> m_cachedBrokenGrassMatrices;
};