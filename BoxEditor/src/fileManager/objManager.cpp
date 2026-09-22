#include "fileManager/objManager.h"
#include <fstream>
#include <entity/Entity.h>
#include <material/Material.h>

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

    // ----------------- mtl -----------------------------------
    file << "mtllib " << objFilePath.stem().string() << ".mtl\n\n";

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
        // ---------------------------------------------------------
        // Material used by this triangle
        // ---------------------------------------------------------
        const std::size_t materialIndex =
            mesh.vertices[mesh.indices[i]].materialIndex;

        if (materialIndex <
            entity.GetMaterialSlotCount())
        {
            const Material& material =
                entity.GetMaterialSlot(
                    materialIndex
                );

            file
                << "usemtl "
                << material.GetName()
                << "\n";
        }

        // ---------------------------------------------------------
        // Triangle
        // ---------------------------------------------------------
        file
            << "f "
            << i0 << "/" << i0 << "/" << i0 << " "
            << i1 << "/" << i1 << "/" << i1 << " "
            << i2 << "/" << i2 << "/" << i2 << "\n";
    }

    file.close();
   
    // ---------------------------------------------------------
    // Write companion MTL file
    // ---------------------------------------------------------

    std::filesystem::path mtlFilePath =
        objFilePath;

    mtlFilePath.replace_extension(".mtl");

    if (!WriteMTL(
        entity,
        mtlFilePath))
    {
        return false;
    }


    return true;
}

bool objManager::WriteMTL(const Entity& entity, const std::filesystem::path& mtlFilePath)
{
    std::ofstream file(mtlFilePath);

    if (!file.is_open())
    {
        return false;
    }

    file << "# BoxEditor Material File\n\n";

    const std::size_t materialCount =
        entity.GetMaterialSlotCount();

    for (std::size_t i = 0;
        i < materialCount;
        ++i)
    {
        const Material& material =
            entity.GetMaterialSlot(i);

        const glm::vec4& color =
            material.GetBaseColor();

        file
            << "newmtl "
            << material.GetName()
            << "\n";

        // Diffuse / base colour
        file
            << "Kd "
            << color.r << " "
            << color.g << " "
            << color.b << "\n";

        // Alpha
        file
            << "d "
            << material.GetAlpha()
            << "\n";

        file << "\n";
    }

    file.close();

    return true;
}
