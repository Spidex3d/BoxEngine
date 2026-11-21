#include "mesh/modifiers/FaceExtrude.h"
#include <entity/Entity.h>

#include <mesh/MeshEditing.h>
#include <miniBoxLog.h>

#include <glm/glm.hpp>

#include <map>
#include <queue>
#include <utility>
#include <vector>
#include <limits>
#include <unordered_set>
#include <algorithm>

//bool FaceExtrude::Use(Entity& entity, std::size_t faceIndex, float distance)
bool FaceExtrude::Use(MeshEditing& mesh, std::size_t faceIndex, const glm::vec3& direction, float distance)
{
    // Implementation of face extrusion goes here
    if (faceIndex >=
        mesh.GetFaceCount())
    {
        BOX_LOG_ERROR(
            "FaceExtrude: Invalid face index"
        );

        return false;
    }

    const EditFace originalFace = mesh.GetFace(faceIndex);

    if (originalFace.vertices.size() < 3)
    {
        BOX_LOG_ERROR(
            "FaceExtrude: Face has fewer than 3 vertices"
        );

        return false;
    }

    // ------------------------------------------
    // Validate direction
    // ------------------------------------------

    const float directionLength =
        glm::length(direction);

    if (directionLength <= 0.000001f)
    {
        BOX_LOG_ERROR(
            "FaceExtrude: Invalid extrusion direction"
        );

        return false;
    }

    const glm::vec3 extrusionDirection =
        glm::normalize(direction);

    // ------------------------------------------
    // Create duplicated outer face vertices
    // ------------------------------------------

    std::vector<std::size_t>
        newFaceVertices;

    newFaceVertices.reserve(
        originalFace.vertices.size()
    );

    for (const std::size_t oldIndex :
    originalFace.vertices)
    {
        const glm::vec3 oldPosition =
            mesh.GetVertex(
                oldIndex
            ).position;

        const glm::vec3 newPosition =
            oldPosition +
            extrusionDirection *
            distance;

        const std::size_t newIndex =
            mesh.AddVertex(
                newPosition
            );

        newFaceVertices.push_back(
            newIndex
        );
    }

    // ------------------------------------------
    // Replace original face with new outer face
    // ------------------------------------------

    if (!mesh.SetFace(
        faceIndex,
        newFaceVertices))
    {
        BOX_LOG_ERROR(
            "FaceExtrude: Failed to replace face"
        );

        return false;
    }

    // ------------------------------------------
    // Create side faces
    // ------------------------------------------

    const std::size_t vertexCount =
        originalFace.vertices.size();

    for (std::size_t i = 0;
        i < vertexCount;
        ++i)
    {
        const std::size_t next =
            (i + 1) %
            vertexCount;

        const std::size_t oldA =
            originalFace.vertices[i];

        const std::size_t oldB =
            originalFace.vertices[next];

        const std::size_t newA =
            newFaceVertices[i];

        const std::size_t newB =
            newFaceVertices[next];

        mesh.AddFace(
            {
                oldA,
                oldB,
                newB,
                newA
            }
        );
    }

    // ------------------------------------------
    // Rebuild modelling edges
    // ------------------------------------------

    mesh.RebuildEdges();

    BOX_LOG_INFO(
        "Face extruded. "
        << "Vertices="
        << mesh.GetVertexCount()
        << " Edges="
        << mesh.GetEdgeCount()
        << " Faces="
        << mesh.GetFaceCount()
    );

    return true;
}


// ============================================
// MULTI-FACE EXTRUSION
// ============================================

bool FaceExtrude::UseMultiple(
    MeshEditing& mesh,
    const std::vector<std::size_t>& faceIndices,
    const std::vector<glm::vec3>& directions,
    float distance)
{
    // ========================================
    // VALIDATE INPUT
    // ========================================

    if (faceIndices.empty())
    {
        BOX_LOG_WARNING(
            "Multi-face extrusion: no faces selected"
        );

        return false;
    }

    if (faceIndices.size() != directions.size())
    {
        BOX_LOG_ERROR(
            "Multi-face extrusion: direction count mismatch"
        );

        return false;
    }

    // Save the original mesh in case an
    // operation fails partway through.
    const MeshEditing originalMesh = mesh;

    const std::size_t originalFaceCount =
        mesh.GetFaceCount();

    std::unordered_set<std::size_t> processedFaces;

    // ========================================
    // EXTRUDE ALL SELECTED FACES
    // ========================================

    for (std::size_t i = 0;
        i < faceIndices.size();
        ++i)
    {
        const std::size_t faceIndex =
            faceIndices[i];

        if (faceIndex >= originalFaceCount)
        {
            BOX_LOG_ERROR(
                "Multi-face extrusion: invalid face "
                << faceIndex
            );

            mesh = originalMesh;
            return false;
        }

        // Don't extrude the same face twice.
        if (!processedFaces.insert(faceIndex).second)
        {
            continue;
        }

        const glm::vec3 direction =
            directions[i];

        if (glm::length(direction) < 0.000001f)
        {
            BOX_LOG_ERROR(
                "Multi-face extrusion: invalid direction"
            );

            mesh = originalMesh;
            return false;
        }

        // Reuse our existing single-face
        // extrusion operation.
        if (!Use(
            mesh,
            faceIndex,
            direction,
            distance))
        {
            // Restore everything if one
            // extrusion fails.
            mesh = originalMesh;

            BOX_LOG_ERROR(
                "Multi-face extrusion failed"
            );

            return false;
        }
    }

    BOX_LOG_INFO(
        "Multi-face extrusion completed. Faces="
        << processedFaces.size()
        << " Distance="
        << distance
    );

    return true;
}


bool FaceExtrude::UseRegion(
    MeshEditing& mesh,
    const std::vector<std::size_t>& faceIndices,
    const std::vector<glm::vec3>& directions,
    float distance)
{
    // ============================================
    // VALIDATE THE SELECTION
    // ============================================

    if (faceIndices.empty() ||
        faceIndices.size() != directions.size())
    {
        BOX_LOG_ERROR("Region Extrude: invalid selection");
        return false;
    }

    const std::size_t originalFaceCount =
        mesh.GetFaceCount();

    const std::size_t originalVertexCount =
        mesh.GetVertexCount();

    const std::size_t InvalidIndex =
        std::numeric_limits<std::size_t>::max();

    using EdgeKey =
        std::pair<std::size_t, std::size_t>;

    // Each undirected edge maps to the selected
    // faces that use it.
    std::map<EdgeKey, std::vector<std::size_t>>
        edgeOwners;

    std::vector<bool> seen(
        originalFaceCount, false
    );

    for (std::size_t i = 0; i < faceIndices.size(); ++i)
    {
        const std::size_t faceIndex = faceIndices[i];

        if (faceIndex >= originalFaceCount ||
            seen[faceIndex])
        {
            BOX_LOG_ERROR(
                "Region Extrude: invalid or duplicate face"
            );
            return false;
        }

        seen[faceIndex] = true;

        if (glm::length(directions[i]) < 0.000001f)
            return false;

        const EditFace& face = mesh.GetFace(faceIndex);

        if (face.vertices.size() < 3)
            return false;

        for (std::size_t j = 0;
            j < face.vertices.size(); ++j)
        {
            const std::size_t a = face.vertices[j];
            const std::size_t b =
                face.vertices[(j + 1) % face.vertices.size()];

            if (a >= originalVertexCount ||
                b >= originalVertexCount ||
                a == b)
            {
                return false;
            }

            const EdgeKey key = std::minmax(a, b);

            // i is the position within faceIndices.
            edgeOwners[key].push_back(i);
        }
    }

    // ============================================
    // FIND CONNECTED SELECTED REGIONS
    // ============================================

    std::vector<std::vector<std::size_t>>
        neighbours(faceIndices.size());

    for (const auto& entry : edgeOwners)
    {
        const auto& owners = entry.second;

        // More than two selected faces sharing
        // one edge is non-manifold.
        if (owners.size() > 2)
        {
            BOX_LOG_ERROR(
                "Region Extrude: non-manifold selected edge"
            );
            return false;
        }

        if (owners.size() == 2)
        {
            neighbours[owners[0]].push_back(owners[1]);
            neighbours[owners[1]].push_back(owners[0]);
        }
    }

    std::vector<int> regionOf(
        faceIndices.size(), -1
    );

    std::vector<std::vector<std::size_t>> regions;

    for (std::size_t start = 0;
        start < faceIndices.size(); ++start)
    {
        if (regionOf[start] != -1)
            continue;

        const int regionID =
            static_cast<int>(regions.size());

        regions.emplace_back();

        std::queue<std::size_t> pending;
        pending.push(start);
        regionOf[start] = regionID;

        while (!pending.empty())
        {
            const std::size_t current = pending.front();
            pending.pop();

            regions.back().push_back(current);

            for (std::size_t next : neighbours[current])
            {
                if (regionOf[next] != -1)
                    continue;

                regionOf[next] = regionID;
                pending.push(next);
            }
        }
    }

    // ============================================
    // CALCULATE ONE DIRECTION PER REGION
    // ============================================

    std::vector<glm::vec3> regionDirections(
        regions.size(), glm::vec3(0.0f)
    );

    for (std::size_t r = 0; r < regions.size(); ++r)
    {
        for (std::size_t selectedIndex : regions[r])
        {
            regionDirections[r] +=
                glm::normalize(directions[selectedIndex]);
        }

        const float length =
            glm::length(regionDirections[r]);

        if (length < 0.000001f)
        {
            BOX_LOG_ERROR(
                "Region Extrude: conflicting directions"
            );
            return false;
        }

        regionDirections[r] /= length;
    }

    // ============================================
    // CREATE THE EXTRUDED REGIONS
    // ============================================

    const MeshEditing originalMesh = mesh;

    for (std::size_t r = 0; r < regions.size(); ++r)
    {
        // All faces in this connected region
        // share a single old-to-new vertex map.
        std::vector<std::size_t> newVertex(
            originalVertexCount, InvalidIndex
        );

        const glm::vec3 offset =
            regionDirections[r] * distance;

        // ----------------------------------------
        // CREATE THE TOP FACES
        // ----------------------------------------

        for (std::size_t selectedIndex : regions[r])
        {
            const std::size_t faceIndex =
                faceIndices[selectedIndex];

            const EditFace originalFace =
                originalMesh.GetFace(faceIndex);

            std::vector<std::size_t> topVertices;
            topVertices.reserve(originalFace.vertices.size());

            for (std::size_t oldIndex :
            originalFace.vertices)
            {
                if (newVertex[oldIndex] == InvalidIndex)
                {
                    const glm::vec3 position =
                        originalMesh.GetVertex(
                            oldIndex
                        ).position;

                    newVertex[oldIndex] =
                        mesh.AddVertex(position + offset);
                }

                topVertices.push_back(newVertex[oldIndex]);
            }

            // Replaces the selected original face
            // without changing its face index.
            if (!mesh.SetFace(faceIndex, topVertices))
            {
                mesh = originalMesh;
                return false;
            }
        }

        // ----------------------------------------
        // CREATE WALLS ON BOUNDARY EDGES ONLY
        // ----------------------------------------

        for (std::size_t selectedIndex : regions[r])
        {
            const EditFace& originalFace =
                originalMesh.GetFace(
                    faceIndices[selectedIndex]
                );

            const std::size_t count =
                originalFace.vertices.size();

            for (std::size_t j = 0; j < count; ++j)
            {
                const std::size_t oldA =
                    originalFace.vertices[j];

                const std::size_t oldB =
                    originalFace.vertices[(j + 1) % count];

                const EdgeKey key = std::minmax(oldA, oldB);

                // Two selected faces share this edge:
                // it is internal, so no wall is needed.
                if (edgeOwners.at(key).size() != 1)
                    continue;

                const std::size_t newA = newVertex[oldA];
                const std::size_t newB = newVertex[oldB];

                const std::size_t sideIndex =
                    mesh.AddFace({
                        oldA, oldB, newB, newA
                        });

                // Preserve the source material.
                mesh.GetFace(sideIndex).materialIndex =
                    originalFace.materialIndex;
            }
        }
    }

    // Rebuild modelling edges only once.
    mesh.RebuildEdges();

    BOX_LOG_INFO(
        "Region Extrude: "
        << faceIndices.size()
        << " faces in "
        << regions.size()
        << " region(s)"
    );

    return true;
}
