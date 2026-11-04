#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cstddef>
#include <cstdint>

class MeshEditing;

// A generated point on the surface of a mesh
struct ScatterPoint
{
    glm::vec3 position{ 0.0f };

    glm::vec3 normal{ 0.0f, 1.0f, 0.0f };
};

class ParticleScatter
{
public:

    // Generate points across one selected face.
    // Positions are in the mesh's local coordinate space.
    static std::vector<ScatterPoint> GenerateOnFace(
        const MeshEditing& mesh,
        std::size_t faceIndex,
        int count,
        std::uint32_t seed
    );

    // Returns stored points, regenerating only when needed.
    static const std::vector<ScatterPoint>& GetCachedOnFace(
        int entityID,
        std::size_t modifierIndex,
        const MeshEditing& mesh,
        std::size_t faceIndex,
        int count,
        std::uint32_t seed
    );

 // =====================================================
// MULTI-PARTICLE SCATTER CACHE
// =====================================================

// Each modifier can contain multiple particle entries,
// and each entry can scatter across multiple faces.

    static const std::vector<ScatterPoint>& GetCachedOnFace(
        int entityID,
        std::size_t modifierIndex,
        std::size_t entryIndex,
        const MeshEditing& mesh,
        std::size_t faceIndex,
        int count,
        std::uint32_t seed
    );

    // =====================================================
    // CALCULATE SURFACE AREA
    // =====================================================

    // Returns the area of a mesh face in square metres.
    // Includes object scaling through the model matrix.
    static float CalculateFaceArea(const MeshEditing& mesh, std::size_t faceIndex,
        const glm::mat4& modelMatrix);

    // Force this modifier's points to regenerate next time.
    static void InvalidateCache(int entityID, std::size_t modifierIndex);



    
    // Remove all cached scatter data for one entity.
    static void ClearEntityCache(int entityID);


private:
};