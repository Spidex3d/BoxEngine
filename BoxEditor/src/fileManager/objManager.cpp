#include "fileManager/objManager.h"
#include <fstream>
#include <entity/Entity.h>

objManager::~objManager() = default;

bool objManager::ExportOBJ(
    const Entity& entity,
    const std::filesystem::path& objFilePath)
{
    std::ofstream file(objFilePath);

    if (!file.is_open())
    {
        return false;
    }

    // ---------------------------------------------------------
    // OBJ header
    // ---------------------------------------------------------

    file << "# BoxEditor OBJ Export\n";
    file << "# Object: " << entity.GetName() << "\n\n";

    file << "o " << entity.GetName() << "\n\n";

    // ---------------------------------------------------------
    // Get the final render mesh
    // ---------------------------------------------------------

    const MeshData& mesh =
        entity.GetMeshData();

    // ---------------------------------------------------------
    // Vertex positions
    // ---------------------------------------------------------

    for (const auto& vertex : mesh.vertices)
    {
        file
            << "v "
            << vertex.position.x << " "
            << vertex.position.y << " "
            << vertex.position.z << "\n";
    }

    file << "\n";

    // ---------------------------------------------------------
    // Texture coordinates
    // ---------------------------------------------------------

    for (const auto& vertex : mesh.vertices)
    {
        file
            << "vt "
            << vertex.uv.x << " "
            << vertex.uv.y << "\n";
    }

    file << "\n";

    // ---------------------------------------------------------
    // Vertex normals
    // ---------------------------------------------------------

    for (const auto& vertex : mesh.vertices)
    {
        file
            << "vn "
            << vertex.normal.x << " "
            << vertex.normal.y << " "
            << vertex.normal.z << "\n";
    }

    file << "\n";

    // ---------------------------------------------------------
    // Faces
    // ---------------------------------------------------------

    for (std::size_t i = 0;
        i + 2 < mesh.indices.size();
        i += 3)
    {
        // OBJ indices start at 1, not 0.
        const unsigned int i0 =
            mesh.indices[i] + 1;

        const unsigned int i1 =
            mesh.indices[i + 1] + 1;

        const unsigned int i2 =
            mesh.indices[i + 2] + 1;

        file
            << "f "
            << i0 << "/" << i0 << "/" << i0 << " "
            << i1 << "/" << i1 << "/" << i1 << " "
            << i2 << "/" << i2 << "/" << i2 << "\n";
    }

    file.close();

    return true;
}
