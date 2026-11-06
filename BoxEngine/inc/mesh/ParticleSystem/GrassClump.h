#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <cstdint>

class GrassBlade;
class Shader;


// =====================================================
// GRASS CLUMP SETTINGS
// =====================================================

struct GrassClumpSettings
{
    // Number of blades in one clump
    int bladeCount = 7;

    // Maximum distance from clump centre (metres)
    float spread = 0.035f;

    // Blade height variation
    float minHeight = 0.8f;
    float maxHeight = 1.2f;

    // Blade width variation
    float minWidth = 0.8f;
    float maxWidth = 1.2f;

    // Outward tilt in degrees
    float minTilt = 5.0f;
    float maxTilt = 25.0f;
};


// =====================================================
// GRASS CLUMP
// =====================================================

class GrassClump
{
public:
    GrassClump() = default;
    ~GrassClump() = default;

    // Original drawing function
    void Draw(
        const GrassBlade& grassBlade,
        Shader& shader,
        const glm::mat4& model
    ) const;

    // Original seven-blade arrangement
    void AppendInstanceMatrices(
        const glm::mat4& clumpModel,
        std::vector<glm::mat4>& matrices
    ) const;

    // NEW: Configurable grass clump
    void AppendInstanceMatrices(
        const glm::mat4& clumpModel,
        const GrassClumpSettings& settings,
        std::uint32_t seed,
        std::vector<glm::mat4>& matrices
    ) const;
};



//#pragma once
//#include <glm\glm.hpp>
//#include <vector>
//
//class GrassBlade;
//class Shader;
//
//class GrassClump
//{
//public:
//    GrassClump() = default;
//    ~GrassClump() = default;
//
//    void Draw(const GrassBlade& grassBlade, Shader& shader,
//        const glm::mat4& model) const;
//
//    // NEW: Generate blade transformations for instancing
//    void AppendInstanceMatrices(const glm::mat4& clumpModel,
//        std::vector<glm::mat4>& matrices) const;
//
//private:
//    // Individual blade transformations
//};