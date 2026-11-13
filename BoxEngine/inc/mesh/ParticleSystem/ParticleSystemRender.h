#pragma once

#include <glm/glm.hpp>
#include <array>
#include <cstddef>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include <mesh/ParticleSystem/GrassBlade.h>

class Entity;
class Shader;
class GrassClump;

// Each scatter entry owns its own two grass meshes and GPU instance buffers.
class ParticleSystemRender
{
public:
    ParticleSystemRender() = default;
    ~ParticleSystemRender() = default;

    // Existing interface retained: BoxEngine call does not change.
    void RenderGrass(
        const std::vector<std::unique_ptr<Entity>>& entities,
        GrassBlade& grassBlade,
        GrassBlade& brokenGrassBlade,
        const GrassClump& grassClump,
        Shader& shader,
        const glm::mat4& view,
        const glm::mat4& projection
    );

    void ClearGrassCache();

private:
    using GrassKey = std::tuple<int, std::size_t, std::size_t>;

    struct GrassVariant
    {
        std::unique_ptr<GrassBlade> normal;
        std::unique_ptr<GrassBlade> broken;
        std::array<float, 4> shape{};
        bool geometryReady = false;
        bool forceUpload = true;

        // Last matrices retained on the GPU.
        std::vector<glm::mat4> cachedNormal;
        std::vector<glm::mat4> cachedBroken;

        // Rebuilt each frame for comparison, not uploaded unless changed.
        std::vector<glm::mat4> pendingNormal;
        std::vector<glm::mat4> pendingBroken;
    };

    std::map<GrassKey, GrassVariant> m_grassVariants;
};

