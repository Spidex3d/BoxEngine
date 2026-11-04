#include "mesh/ParticleSystem/ParticleScatter.h"
#include "mesh/MeshEditing.h"

#include <glm/glm.hpp>

#include <map>
#include <utility>
#include <random>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

// ============================================
// Generate points on a selected mesh face
// ============================================

std::vector<ScatterPoint>
ParticleScatter::GenerateOnFace(
    const MeshEditing& mesh,
    std::size_t faceIndex,
    int count,
    std::uint32_t seed)
{
    std::vector<ScatterPoint> points;

    if (count <= 0)
        return points;

    if (faceIndex >= mesh.GetFaceCount())
        return points;

    const EditFace& face =
        mesh.GetFace(faceIndex);

    if (face.vertices.size() < 3)
        return points;

    // ========================================
    // CONVERT FACE INTO TRIANGLES
    // ========================================

    struct ScatterTriangle
    {
        glm::vec3 a;
        glm::vec3 b;
        glm::vec3 c;

        glm::vec3 normal;

        float area;
    };

    std::vector<ScatterTriangle> triangles;

    float totalArea = 0.0f;

    // Triangle fan from the first face vertex.
    // Suitable for our initial convex-face test.
    const std::size_t firstIndex =
        face.vertices[0];

    if (firstIndex >= mesh.GetVertexCount())
        return points;

    const glm::vec3 firstPosition =
        mesh.GetVertex(firstIndex).position;

    for (std::size_t i = 1;
        i + 1 < face.vertices.size();
        ++i)
    {
        const std::size_t indexB =
            face.vertices[i];

        const std::size_t indexC =
            face.vertices[i + 1];

        if (indexB >= mesh.GetVertexCount() ||
            indexC >= mesh.GetVertexCount())
        {
            continue;
        }

        const glm::vec3 b =
            mesh.GetVertex(indexB).position;

        const glm::vec3 c =
            mesh.GetVertex(indexC).position;

        const glm::vec3 crossProduct =
            glm::cross(
                b - firstPosition,
                c - firstPosition
            );

        const float crossLength =
            glm::length(crossProduct);

        // Ignore degenerate triangles.
        if (crossLength <= 0.000001f)
            continue;

        ScatterTriangle triangle;

        triangle.a = firstPosition;
        triangle.b = b;
        triangle.c = c;

        triangle.area = crossLength * 0.5f;

        triangle.normal =
            crossProduct / crossLength;

        triangles.push_back(triangle);

        totalArea += triangle.area;
    }

    if (triangles.empty() || totalArea <= 0.0f)
        return points;

    // ========================================
    // RANDOM GENERATOR
    // ========================================

    std::mt19937 random(seed);

    std::uniform_real_distribution<float>
        random01(0.0f, 1.0f);

    std::uniform_real_distribution<float>
        randomArea(0.0f, totalArea);

    points.reserve(static_cast<std::size_t>(count));

    // ========================================
    // GENERATE PARTICLES
    // ========================================

    for (int i = 0; i < count; ++i)
    {
        // ------------------------------------
        // Choose triangle based on its area.
        // Larger triangles receive more points.
        // ------------------------------------

        const float areaChoice =
            randomArea(random);

        float accumulatedArea = 0.0f;

        const ScatterTriangle* selectedTriangle =
            &triangles.back();

        for (const ScatterTriangle& triangle : triangles)
        {
            accumulatedArea += triangle.area;

            if (areaChoice <= accumulatedArea)
            {
                selectedTriangle = &triangle;
                break;
            }
        }

        // ------------------------------------
        // Generate random barycentric coordinates.
        // ------------------------------------

        const float r1 = random01(random);
        const float r2 = random01(random);

        const float sqrtR1 = std::sqrt(r1);

        const float u = 1.0f - sqrtR1;

        const float v =
            sqrtR1 * (1.0f - r2);

        const float w =
            sqrtR1 * r2;

        // ------------------------------------
        // Calculate position within triangle.
        // ------------------------------------

        ScatterPoint point;

        point.position =
            selectedTriangle->a * u +
            selectedTriangle->b * v +
            selectedTriangle->c * w;

        point.normal =
            selectedTriangle->normal;

        points.push_back(point);
    }

    return points;
}

// =====================================================
// PARTICLE SCATTER CACHE
// =====================================================

namespace
{
    struct ScatterCacheEntry
    {
        std::vector<ScatterPoint> points;

        std::size_t faceIndex = 0;

        int count = 0;
        std::uint32_t seed = 0;

        std::size_t vertexCount = 0;
        std::size_t faceCount = 0;

        // Remember the actual geometry of the target face.
        std::vector<std::size_t> vertexIndices;
        std::vector<glm::vec3> vertexPositions;

        bool generated = false;
    };

    using CacheKey = std::pair<int, std::size_t>;

    std::map<CacheKey, ScatterCacheEntry> scatterCache;

    bool SamePosition(
        const glm::vec3& a,
        const glm::vec3& b)
    {
        return a.x == b.x &&
            a.y == b.y &&
            a.z == b.z;
    }
}


// =====================================================
// GET CACHED SCATTER POINTS
// =====================================================

const std::vector<ScatterPoint>&
ParticleScatter::GetCachedOnFace(
    int entityID,
    std::size_t modifierIndex,
    const MeshEditing& mesh,
    std::size_t faceIndex,
    int count,
    std::uint32_t seed)
{
    const CacheKey key{ entityID, modifierIndex };

    ScatterCacheEntry& cache = scatterCache[key];

    // Check whether any generation settings changed.
    bool needsUpdate =
        cache.faceIndex != faceIndex ||
        cache.count != count ||
        cache.seed != seed ||
        cache.vertexCount != mesh.GetVertexCount() ||
        cache.faceCount != mesh.GetFaceCount();

    // Collect the current target face's geometry.
    std::vector<std::size_t> currentIndices;
    std::vector<glm::vec3> currentPositions;

    if (faceIndex < mesh.GetFaceCount())
    {
        const EditFace& face = mesh.GetFace(faceIndex);

        currentIndices = face.vertices;

        for (std::size_t index : face.vertices)
        {
            if (index < mesh.GetVertexCount())
            {
                currentPositions.push_back(
                    mesh.GetVertex(index).position
                );
            }
        }
    }

    // Check whether the face's vertex indices changed.
    if (cache.vertexIndices != currentIndices)
    {
        needsUpdate = true;
    }

    // Check whether any vertex positions changed.
    if (cache.vertexPositions.size() !=
        currentPositions.size())
    {
        needsUpdate = true;
    }
    else
    {
        for (std::size_t i = 0;
            i < currentPositions.size();
            ++i)
        {
            if (!SamePosition(
                cache.vertexPositions[i],
                currentPositions[i]))
            {
                needsUpdate = true;
                break;
            }
        }
    }

    // Also generate on the first request.
    // The map creates a new entry with empty geometry.
    
    if (!cache.generated)
    {
        needsUpdate = true;
    }
    
    

    if (needsUpdate)
    {
        cache.points = GenerateOnFace(
            mesh,
            faceIndex,
            count,
            seed
        );

        cache.generated = true;

        cache.faceIndex = faceIndex;
        cache.count = count;
        cache.seed = seed;

        cache.vertexCount = mesh.GetVertexCount();
        cache.faceCount = mesh.GetFaceCount();

        cache.vertexIndices = std::move(currentIndices);
        cache.vertexPositions = std::move(currentPositions);

        // Temporary test output
        /*std::cout
            << "Scatter regenerated: Entity "
            << entityID
            << ", Modifier "
            << modifierIndex
            << ", Points "
            << cache.points.size()
            << std::endl;*/
    }

    return cache.points;
}


// =====================================================
// INVALIDATE A SPECIFIC MODIFIER CACHE
// =====================================================

void ParticleScatter::InvalidateCache(
    int entityID,
    std::size_t modifierIndex)
{
    const CacheKey key{ entityID, modifierIndex };

    scatterCache.erase(key);
}

// =====================================================
// CLEAR ALL PARTICLE CACHES FOR AN ENTITY
// =====================================================

void ParticleScatter::ClearEntityCache(int entityID)
{
    for (auto it = scatterCache.begin();
        it != scatterCache.end();)
    {
        // Cache key:
        // first  = Entity ID
        // second = Modifier Index

        if (it->first.first == entityID)
        {
            it = scatterCache.erase(it);
        }
        else
        {
            ++it;
        }
    }
}