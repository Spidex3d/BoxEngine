#include <fileManager\mbxManager.h>
#include <entity/Entity.h>
#include <rendering/Material.h>
#include <miniBoxLog.h>

#include <fstream>
#include <iomanip>
#include <sstream>
#include <system_error>
#include <filesystem>

// version 0.1

namespace fs = std::filesystem;

bool mbxManager::ExportMBX(const Entity& entity, const std::filesystem::path& mbxFilePath)
{
	
    if (!entity.GetMeshData().IsValid())
    {
        BOX_LOG_ERROR("Cannot export entity with no mesh data");

        return false;
    }

    fs::path finalPath = mbxFilePath;

    if (finalPath.extension() != ".mbx")
    {
        finalPath.replace_extension(
            ".mbx"
        );
    }

    const fs::path outputDirectory =
        finalPath.parent_path();

    std::error_code error;

    fs::create_directories(
        outputDirectory,
        error
    );

    if (error)
    {
        BOX_LOG_ERROR(
            "Failed to create export directory: "
            << error.message()
        );

        return false;
    }

    std::string copiedTextureName;

    if (!CopyMaterialTextures(
        entity,
        outputDirectory,
        copiedTextureName))
    {
        return false;
    }

    const std::string mbxText =
        BuildMBX(
            entity,
            copiedTextureName
        );

    std::ofstream file(
        finalPath,
        std::ios::out |
        std::ios::trunc
    );

    if (!file.is_open())
    {
        BOX_LOG_ERROR(
            "Failed to create MBX file: "
            << finalPath.string()
        );

        return false;
    }

    file << mbxText;

    if (!file.good())
    {
        BOX_LOG_ERROR(
            "Failed while writing MBX file"
        );

        return false;
    }

    BOX_LOG_INFO(
        "Exported MBX: "
        << finalPath.string()
    );

    return true;

}

bool mbxManager::ImportMBX(const std::filesystem::path& mbxFilePath)
{
	// This will come later after we get export working.
	// import the mbx file from the specified directory
	// load the textures used in the mbx file from the same directory as the mbx file
	return false;
}

std::string mbxManager::BuildMBX(const Entity& entity, const std::string& copiedTextureName) const
{
	// export the mbx file to the specified directory
	// include the textures used in the mbx file to the same directory as the mbx file
    const MeshData& mesh = entity.GetMeshData();

    const Material& material =
        entity.GetMaterial();

    std::ostringstream output;

    output << std::fixed
        << std::setprecision(6);

    output
        << "# BoxEditor MBX 0.1\n"
        << "# Single-object mesh file\n\n";

    output
        << "o "
        << MakeSafeName(entity.GetName())
        << "\n\n";

    /*
     * Each MeshVertex stores a complete
     * position/normal/UV combination.
     */
    for (const MeshVertex& vertex :
        mesh.vertices)
    {
        output
            << "v "
            << vertex.position.x << ' '
            << vertex.position.y << ' '
            << vertex.position.z << '\n';
    }

    output << '\n';

    for (const MeshVertex& vertex :
        mesh.vertices)
    {
        output
            << "vn "
            << vertex.normal.x << ' '
            << vertex.normal.y << ' '
            << vertex.normal.z << '\n';
    }

    output << '\n';

    for (const MeshVertex& vertex :
        mesh.vertices)
    {
        output
            << "vuv "
            << vertex.uv.x << ' '
            << vertex.uv.y << '\n';
    }

    output << '\n';

    /*
     * MBX uses 1-based indices, following
     * the familiar OBJ convention.
     */
    if (mesh.HasIndices())
    {
        for (std::size_t index = 0;
            index + 2 < mesh.indices.size();
            index += 3)
        {
            const std::uint32_t a =
                mesh.indices[index] + 1;

            const std::uint32_t b =
                mesh.indices[index + 1] + 1;

            const std::uint32_t c =
                mesh.indices[index + 2] + 1;

            output
                << "f "
                << a << '/' << a << '/' << a << ' '
                << b << '/' << b << '/' << b << ' '
                << c << '/' << c << '/' << c
                << '\n';
        }
    }
    else
    {
        for (std::size_t vertex = 0;
            vertex + 2 < mesh.vertices.size();
            vertex += 3)
        {
            const std::size_t a =
                vertex + 1;

            const std::size_t b =
                vertex + 2;

            const std::size_t c =
                vertex + 3;

            output
                << "f "
                << a << '/' << a << '/' << a << ' '
                << b << '/' << b << '/' << b << ' '
                << c << '/' << c << '/' << c
                << '\n';
        }
    }

    output
        << "\nmaterial\n";

    const glm::vec4 baseColor =
        material.GetBaseColor();

    output
        << "base_color "
        << baseColor.r << ' '
        << baseColor.g << ' '
        << baseColor.b << ' '
        << material.GetAlpha()
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

    const glm::vec3 emissionColor =
        material.GetEmissionColor();

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

    if (!copiedTextureName.empty())
    {
        output
            << "base_color_map "
            << copiedTextureName
            << '\n';
    }

    output
        << "endmaterial\n";

    return output.str();
}

bool mbxManager::CopyMaterialTextures(const Entity& entity, const std::filesystem::path& outputDirectory, std::string& outBaseColorTextureName) const
{
    outBaseColorTextureName.clear();

    const Material& material = entity.GetMaterial();

    if (!material.UsesBaseColorTexture())
    {
        return true;
    }

    const std::string& sourcePathString = material.GetBaseColorTexturePath();

    if (sourcePathString.empty())
    {
        BOX_LOG_ERROR("Material has a texture ID but no source path ");

        return false;
    }

    const fs::path sourcePath = sourcePathString;

    if (!fs::exists(sourcePath))
    {
        BOX_LOG_ERROR("Texture source does not exist: " << sourcePath.string());

        return false;
    }

    const fs::path destinationPath = outputDirectory / sourcePath.filename();

    std::error_code error;

    /*
     * Do not copy a file onto itself.
     */
    if (fs::weakly_canonical(sourcePath, error) != fs::weakly_canonical(destinationPath, error))
    {
        fs::copy_file(sourcePath, destinationPath, fs::copy_options::overwrite_existing, error);

        if (error)
        {
            BOX_LOG_ERROR("Failed to copy texture: " << error.message());

            return false;
        }
    }

    outBaseColorTextureName = destinationPath.filename().string();

    return true;
}

std::string mbxManager::MakeSafeName(const std::string& name)
{
    std::string result =
        name;

    for (char& character : result)
    {
        if (character == ' ' ||
            character == '\t')
        {
            character = '_';
        }
    }

    if (result.empty())
    {
        result = "UnnamedObject";
    }

    return result;
}
