#pragma once
#include <mesh/MeshData.h>
#include <cstdint>

class MeshEditing;

class RockGenerator
{
public:
    RockGenerator() = default;
    ~RockGenerator();

    bool Generate(
        MeshEditing& mesh,
        float radius,
        int subdivisions,
        float roughness,
        std::uint32_t seed,
        float rockFlattening,
        float rockElongation = 1.0f,
        float rockAngularity = 0.0f,
        int flatFaceCount = 0,
        float cutDepth = 0.0f,
        bool randomFaceDirection = false
    );

private:
    float Random01(
        std::uint32_t seed,
        std::uint32_t index
    ) const;
};