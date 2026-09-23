#include <mesh\UV\UVMapping.h>
#include <mesh\MeshEditing.h>
#include <algorithm>
#include <cmath>

bool UVMapping::Box(MeshEditing& mesh)
{
    // -------------------------------------------------
    // Cube UV cross layout.
    //
    //          [ Top ]
    // [Left] [Front] [Right] [Back]
    //          [Bottom]
    //
    // Atlas = 4 columns x 3 rows.
    // -------------------------------------------------

    if (mesh.GetFaceCount() != 6)
    {
        return false;
    }

    const float cellWidth =
        1.0f / 4.0f;

    const float cellHeight =
        1.0f / 3.0f;


    // -------------------------------------------------
    // Helper for assigning one atlas cell to one face.
    // -------------------------------------------------

    auto SetFaceUV =
        [&](std::size_t faceIndex,
            int column,
            int row)
    {
        EditFace& face =
            mesh.GetFace(faceIndex);

        if (face.vertices.size() != 4)
        {
            return;
        }

        const float u0 =
            column * cellWidth;

        const float v0 =
            row * cellHeight;

        const float u1 =
            u0 + cellWidth;

        const float v1 =
            v0 + cellHeight;

        face.uvs =
        {
            glm::vec2(u0, v0),
            glm::vec2(u1, v0),
            glm::vec2(u1, v1),
            glm::vec2(u0, v1)
        };
    };


    // -------------------------------------------------
    // MeshEditing::CreateCube() face order:
    //
    // 0 Front
    // 1 Back
    // 2 Left
    // 3 Right
    // 4 Top
    // 5 Bottom
    // -------------------------------------------------

    SetFaceUV(2, 0, 1); // Left
    SetFaceUV(0, 1, 1); // Front
    SetFaceUV(3, 2, 1); // Right
    SetFaceUV(1, 3, 1); // Back

    SetFaceUV(4, 1, 2); // Top
    SetFaceUV(5, 1, 0); // Bottom

    return true;
}

bool UVMapping::Spherical(MeshEditing& mesh)
{
    const auto& vertices =
        mesh.GetVertices();

    if (vertices.empty() ||
        mesh.GetFaceCount() == 0)
    {
        return false;
    }

    constexpr float PI =
        3.14159265358979323846f;

    // -------------------------------------------------
    // Generate spherical UV coordinates.
    //
    // U = longitude around the object.
    // V = latitude from bottom to top.
    //
    // UVs are stored per face-corner.
    // -------------------------------------------------

    for (std::size_t faceIndex = 0;
        faceIndex < mesh.GetFaceCount();
        ++faceIndex)
    {
        EditFace& face =
            mesh.GetFace(faceIndex);

        face.uvs.clear();
        face.uvs.reserve(face.vertices.size());

        for (std::size_t vertexIndex :
        face.vertices)
        {
            if (vertexIndex >= vertices.size())
            {
                return false;
            }

            const glm::vec3 position =
                vertices[vertexIndex].position;

            const float length =
                glm::length(position);

            if (length <= 0.000001f)
            {
                face.uvs.emplace_back(
                    0.5f,
                    0.5f
                );

                continue;
            }

            const glm::vec3 direction =
                position / length;

            float u =
                0.5f +
                std::atan2(
                    direction.z,
                    direction.x
                ) / (2.0f * PI);

            float v =
                0.5f -
                std::asin(
                    glm::clamp(
                        direction.y,
                        -1.0f,
                        1.0f
                    )
                ) / PI;

            face.uvs.emplace_back(u, v);
        }

        // -------------------------------------------------
        // Correct triangles/faces crossing the U seam.
        //
        // Example:
        //
        // Before:
        //     0.98, 0.02, 0.01
        //
        // After:
        //     0.98, 1.02, 1.01
        //
        // This prevents interpolation across almost the
        // entire texture.
        // -------------------------------------------------

        if (!face.uvs.empty())
        {
            float minU = face.uvs[0].x;
            float maxU = face.uvs[0].x;

            for (const glm::vec2& uv :
                face.uvs)
            {
                minU = std::min(minU, uv.x);
                maxU = std::max(maxU, uv.x);
            }

            // If the U range is greater than half the
            // texture width, this face crosses the seam.
            if ((maxU - minU) > 0.5f)
            {
                for (glm::vec2& uv :
                    face.uvs)
                {
                    if (uv.x < 0.5f)
                    {
                        uv.x += 1.0f;
                    }
                }
            }
        }

        // -------------------------------------------------
        // Correct spherical pole distortion.
        //
        // Near the north/south pole, longitude becomes
        // ambiguous. Give the pole corner a U coordinate
        // based on the other corners of the face instead.
        // -------------------------------------------------

        if (face.uvs.size() == face.vertices.size() &&
            face.vertices.size() >= 3)
        {
            constexpr float poleThreshold = 0.90f;

            std::size_t poleCorner =
                static_cast<std::size_t>(-1);

            float highestPoleAmount = 0.0f;

            // ---------------------------------------------
            // Find the corner closest to either pole.
            // ---------------------------------------------

            for (std::size_t corner = 0;
                corner < face.vertices.size();
                ++corner)
            {
                const std::size_t vertexIndex =
                    face.vertices[corner];

                if (vertexIndex >= vertices.size())
                {
                    continue;
                }

                const glm::vec3 position =
                    vertices[vertexIndex].position;

                const float length =
                    glm::length(position);

                if (length <= 0.000001f)
                {
                    continue;
                }

                const glm::vec3 direction =
                    position / length;

                const float poleAmount =
                    std::abs(direction.y);

                if (poleAmount > highestPoleAmount)
                {
                    highestPoleAmount =
                        poleAmount;

                    poleCorner =
                        corner;
                }
            }

            // ---------------------------------------------
            // Correct only the most pole-like corner.
            // ---------------------------------------------

            if (poleCorner !=
                static_cast<std::size_t>(-1) &&
                highestPoleAmount >= poleThreshold)
            {
                float averageU = 0.0f;
                std::size_t count = 0;

                for (std::size_t corner = 0;
                    corner < face.uvs.size();
                    ++corner)
                {
                    if (corner == poleCorner)
                    {
                        continue;
                    }

                    averageU +=
                        face.uvs[corner].x;

                    ++count;
                }

                if (count > 0)
                {
                    averageU /=
                        static_cast<float>(count);

                    face.uvs[poleCorner].x =
                        averageU;
                }
            }

        }
       
    }

    return true;
}