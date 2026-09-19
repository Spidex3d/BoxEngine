#include <fileManager\SceneSerializer.h>
#include <entity/Entity.h>
#include <miniBoxLog.h>

#include <fstream>
#include <iomanip>

SceneSerializer::~SceneSerializer() = default;

bool SceneSerializer::SerializeScene(
    const std::filesystem::path& filePath,
    const std::vector<std::unique_ptr<Entity>>& entities)
{
    std::filesystem::path finalPath =
        filePath;

    // ---------------------------------------------
    // Make sure this is a BoxEditor scene file.
    // ---------------------------------------------
    if (finalPath.extension() != ".mbs")
    {
        finalPath.replace_extension(".mbs");
    }

    // ---------------------------------------------
    // Create parent directory if required.
    // ---------------------------------------------
    const std::filesystem::path directory =
        finalPath.parent_path();

    if (!directory.empty())
    {
        std::error_code error;

        std::filesystem::create_directories(
            directory,
            error
        );

        if (error)
        {
            BOX_LOG_ERROR(
                "Failed to create scene directory: "
                << error.message()
            );

            return false;
        }
    }

    // ---------------------------------------------
    // Open scene file.
    // ---------------------------------------------
    std::ofstream file(
        finalPath,
        std::ios::out |
        std::ios::trunc
    );

    if (!file.is_open())
    {
        BOX_LOG_ERROR(
            "Failed to create scene file: "
            << finalPath.string()
        );

        return false;
    }

    file << std::fixed
        << std::setprecision(6);

    // ---------------------------------------------
    // Scene header.
    // ---------------------------------------------
    file << "# BoxEditor Scene\n";
    file << "version 0 1\n\n";

    file << "entities "
        << entities.size()
        << "\n\n";

    // ---------------------------------------------
    // Serialize every entity.
    // ---------------------------------------------
    for (const auto& entity : entities)
    {
        if (!entity)
        {
            continue;
        }

        if (!SerializeEntity(
            file,
            *entity))
        {
            BOX_LOG_ERROR(
                "Failed to serialize entity"
            );

            return false;
        }
    }

    if (!file.good())
    {
        BOX_LOG_ERROR(
            "Failed while writing scene file"
        );

        return false;
    }

    BOX_LOG_INFO(
        "Scene saved: "
        << finalPath.string()
    );

    return true;
}


bool SceneSerializer::SerializeEntity(
    std::ostream& output,
    const Entity& entity)
{
    const glm::vec3& position =
        entity.GetPosition();

    const glm::vec3& rotation =
        entity.GetRotation();

    const glm::vec3& scale =
        entity.GetScale();

    output << "entity\n";

    output
        << "id "
        << entity.GetID()
        << '\n';

    output
        << "name "
        << entity.GetName()
        << '\n';

    output
        << "primitive "
        << static_cast<int>(
            entity.GetPrimitiveType()
            )
        << '\n';

    output
        << "visible "
        << (entity.IsVisible() ? 1 : 0)
        << '\n';

    output
        << "position "
        << position.x << ' '
        << position.y << ' '
        << position.z
        << '\n';

    output
        << "rotation "
        << rotation.x << ' '
        << rotation.y << ' '
        << rotation.z
        << '\n';

    output
        << "scale "
        << scale.x << ' '
        << scale.y << ' '
        << scale.z
        << '\n';

    // =================================================
    // EDITABLE MESH
    // =================================================

    const MeshEditing& mesh =
        entity.GetEditableMesh();

    output
        << "shading "
        << static_cast<int>(
            mesh.GetShadingMode()
            )
        << '\n';


    // -------------------------------------------------
    // Vertices
    // -------------------------------------------------

    output
        << "vertices "
        << mesh.GetVertexCount()
        << '\n';

    for (const EditVertex& vertex :
        mesh.GetVertices())
    {
        output
            << "v "
            << vertex.position.x << ' '
            << vertex.position.y << ' '
            << vertex.position.z
            << '\n';
    }


    // -------------------------------------------------
    // Faces
    // -------------------------------------------------

    output
        << "faces "
        << mesh.GetFaceCount()
        << '\n';

    for (const EditFace& face :
        mesh.GetFaces())
    {
        output
            << "f "
            << face.vertices.size();

        for (std::size_t vertexIndex :
        face.vertices)
        {
            output
                << ' '
                << vertexIndex;
        }

        // Save the material assigned to this face.
        output
            << " m "
            << face.materialIndex;

        output << '\n';
    }


    // -------------------------------------------------
    // Loose edges
    // -------------------------------------------------

    std::size_t looseEdgeCount = 0;

    for (const EditEdge& edge :
        mesh.GetEdges())
    {
        if (edge.loose)
        {
            ++looseEdgeCount;
        }
    }

    output
        << "loose_edges "
        << looseEdgeCount
        << '\n';

    for (const EditEdge& edge :
        mesh.GetEdges())
    {
        if (!edge.loose)
        {
            continue;
        }

        output
            << "e "
            << edge.vertexA << ' '
            << edge.vertexB
            << '\n';
    }

    // =================================================
// MATERIAL SLOTS
// =================================================

    output
        << "materials "
        << entity.GetMaterialSlotCount()
        << '\n';

    for (std::size_t i = 0;
        i < entity.GetMaterialSlotCount();
        ++i)
    {
        const Material& material =
            entity.GetMaterialSlot(i);

        const glm::vec4& baseColor =
            material.GetBaseColor();

        const glm::vec3& emissionColor =
            material.GetEmissionColor();

        output
            << "material "
            << i
            << '\n';

        output
            << "material_name "
            << material.GetName()
            << '\n';

        output
            << "material_type "
            << static_cast<int>(
                material.GetType()
                )
            << '\n';

        output
            << "base_color "
            << baseColor.r << ' '
            << baseColor.g << ' '
            << baseColor.b << ' '
            << baseColor.a
            << '\n';

        output
            << "metallic "
            << material.GetMetallic()
            << '\n';

        output
            << "roughness "
            << material.GetRoughness()
            << '\n';

        output
            << "alpha "
            << material.GetAlpha()
            << '\n';

        output
            << "emission_color "
            << emissionColor.r << ' '
            << emissionColor.g << ' '
            << emissionColor.b
            << '\n';

        output
            << "emission_strength "
            << material.GetEmissionStrength()
            << '\n';

        output
            << "transmission "
            << material.GetTransmission()
            << '\n';

        output
            << "ior "
            << material.GetIOR()
            << '\n';

        // ---------------------------------------------
        // Base color texture
        // ---------------------------------------------

        output
            << "use_base_texture "
            << (material.UsesBaseColorTexture() ? 1 : 0)
            << '\n';

        output
            << "base_texture "
            << material.GetBaseColorTexturePath()
            << '\n';

        // ---------------------------------------------
        // Normal map
        // ---------------------------------------------

        output
            << "use_normal_texture "
            << (material.UsesNormalTexture() ? 1 : 0)
            << '\n';

        output
            << "normal_texture "
            << material.GetNormalTexturePath()
            << '\n';

        output
            << "normal_strength "
            << material.GetNormalStrength()
            << '\n';

        output << "endmaterial\n";
    }


    output << "endentity\n\n";

    return output.good();
}