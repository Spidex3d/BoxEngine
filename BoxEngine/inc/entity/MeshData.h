#pragma once
#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

struct MeshVertex
{
    glm::vec3 position{ 0.0f };
    glm::vec3 normal{ 0.0f };
    glm::vec2 uv{ 0.0f };
};

struct MeshData
{
    std::vector<MeshVertex> vertices;
    std::vector<std::uint32_t> indices;

    bool HasIndices() const
    {
        return !indices.empty();
    }

    bool IsValid() const
    {
        return !vertices.empty();
    }
};
