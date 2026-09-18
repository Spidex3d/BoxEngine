#include "mesh/ecosystem/RockGenerator.h"
#include <mesh/MeshEditing.h>
#include <cstdint>
#include <miniBoxLog.h>

RockGenerator::~RockGenerator() = default;


bool RockGenerator::Generate(
    MeshEditing& mesh,
    float radius,
    int subdivisions,
    float roughness,
    std::uint32_t seed)
{
    if (radius <= 0.0f)
        return false;

    roughness =
        glm::clamp(roughness, 0.0f, 1.0f);

    const int sectors =
        8 + subdivisions * 4;

    const int stacks =
        4 + subdivisions * 2;

    return mesh.CreateRock(sectors, stacks, radius, roughness, seed);
}




    float RockGenerator::Random01(
        std::uint32_t seed,
        std::uint32_t index) const
    {
        std::uint32_t value =
            seed ^ (index * 0x9E3779B9u);

        value ^= value >> 16;
        value *= 0x7FEB352Du;
        value ^= value >> 15;
        value *= 0x846CA68Bu;
        value ^= value >> 16;

        return static_cast<float>(value) /
            static_cast<float>(UINT32_MAX);
    }



