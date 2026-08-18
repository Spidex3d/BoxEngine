#pragma once
#include <glm/glm.hpp>

#include <cstdint>
#include <vector>
// vertex structure for mesh data
struct MeshVertex
{
    glm::vec3 position{ 0.0f };
    glm::vec3 normal{ 0.0f };
    glm::vec2 uv{ 0.0f };

    std::uint32_t materialIndex = 0;
};

// logical edge structure for mesh data
// Face structure for mesh data
struct LogicalFace
{
    std::vector<std::size_t> vertexIndices;
    std::vector<std::size_t> edgeIndices;
};


struct MeshData
{
    std::vector<MeshVertex> vertices;
   
	std::vector<LogicalFace> faces;
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
