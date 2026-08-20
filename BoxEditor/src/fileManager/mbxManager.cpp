#include <fileManager\mbxManager.h>
#include <entity/Entity.h>
#include <rendering/Material.h>
#include <miniBoxLog.h>

#include <fstream>
#include <iomanip>
#include <sstream>
#include <system_error>
#include <filesystem>
#include <algorithm>
#include <vector>

namespace fs = std::filesystem;
// version 0.1




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

bool mbxManager::ImportMBX(const std::filesystem::path& mbxFilePath, MBXImportData& outData)
{
    std::ifstream file(mbxFilePath);

    if (!file.is_open())
    {
        BOX_LOG_ERROR("Failed to open MBX file: " << mbxFilePath.string());

        return false;
    }

    outData = MBXImportData{};

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> textureCoordinates;

    std::vector<std::uint32_t> positionIndices;

    std::vector<std::uint32_t> textureIndices;

    std::vector<std::uint32_t> normalIndices;

    bool readingMaterial = false;

    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty() ||
            line[0] == '#')
        {
            continue;
        }

        std::istringstream stream(line);

        std::string command;
        stream >> command;

        if (command == "o")
        {
            stream >> outData.objectName;
        }
        else if (command == "v")
        {
            glm::vec3 position;

            stream
                >> position.x
                >> position.y
                >> position.z;

            positions.push_back(position);
        }
        else if (command == "vn")
        {
            glm::vec3 normal;

            stream
                >> normal.x
                >> normal.y
                >> normal.z;

            normals.push_back(normal);
        }
        else if (command == "vuv")
        {
            glm::vec2 uv;

            stream
                >> uv.x
                >> uv.y;

            textureCoordinates.push_back(uv);
        }
        else if (command == "f")
        {
            std::string faceVertex;

            /*
             * MBX 0.1 currently exports
             * triangles, so read three entries.
             */
            for (int vertex = 0;
                vertex < 3;
                ++vertex)
            {
                if (!(stream >> faceVertex))
                {
                    BOX_LOG_ERROR("Invalid MBX face line: " << line);

                    return false;
                }

                std::replace(
                    faceVertex.begin(),
                    faceVertex.end(),
                    '/',
                    ' '
                );

                std::istringstream faceStream(faceVertex);

                std::uint32_t positionIndex = 0;
                std::uint32_t uvIndex = 0;
                std::uint32_t normalIndex = 0;

                faceStream
                    >> positionIndex
                    >> uvIndex
                    >> normalIndex;

                /*
                 * Convert MBX's 1-based indices
                 * into C++ 0-based indices.
                 */
                if (positionIndex == 0 ||
                    uvIndex == 0 ||
                    normalIndex == 0)
                {
                    BOX_LOG_ERROR("Invalid zero index in MBX face");

                    return false;
                }

                positionIndices.push_back(positionIndex - 1);

                textureIndices.push_back(uvIndex - 1);

                normalIndices.push_back(normalIndex - 1);
            }
        }
        else if (command == "material")
        {
            readingMaterial = true;
        }
        else if (command == "endmaterial")
        {
            readingMaterial = false;
        }
        else if (readingMaterial)
        {
            if (command == "base_color")
            {
                stream
                    >> outData.baseColor.r
                    >> outData.baseColor.g
                    >> outData.baseColor.b
                    >> outData.baseColor.a;
            }
            else if (command == "metallic")
            {
                stream >> outData.metallic;
            }
            else if (command == "roughness")
            {
                stream >> outData.roughness;
            }
            else if (command == "alpha")
            {
                stream >> outData.alpha;
            }
            else if (
                command == "emission_color")
            {
                stream
                    >> outData.emissionColor.r
                    >> outData.emissionColor.g
                    >> outData.emissionColor.b;
            }
            else if (
                command == "emission_strength")
            {
                stream
                    >> outData.emissionStrength;
            }
            else if (
                command == "base_color_map")
            {
                std::string textureFilename;

                stream >> textureFilename;

                outData.baseColorTexturePath =
                    mbxFilePath.parent_path() /
                    textureFilename;
            }
        }
    }

    if (positionIndices.empty())
    {
        BOX_LOG_ERROR("MBX file contains no faces");

        return false;
    }

    if (positionIndices.size() !=
        textureIndices.size() ||
        positionIndices.size() !=
        normalIndices.size())
    {
        BOX_LOG_ERROR("MBX face index arrays do not match");

        return false;
    }

    /*
     * Rebuild complete MeshVertex entries.
     *
     * This creates one vertex for every face
     * corner. That is simple and preserves
     * different UVs or normals at seams.
     */
    outData.mesh.vertices.reserve(
        positionIndices.size()
    );

    for (std::size_t index = 0;
        index < positionIndices.size();
        ++index)
    {
        const std::uint32_t positionIndex =
            positionIndices[index];

        const std::uint32_t uvIndex =
            textureIndices[index];

        const std::uint32_t normalIndex =
            normalIndices[index];

        if (positionIndex >= positions.size() ||
            uvIndex >= textureCoordinates.size() ||
            normalIndex >= normals.size())
        {
            BOX_LOG_ERROR(
                "MBX face index is outside its array"
            );

            return false;
        }

        MeshVertex vertex;

        vertex.position = positions[positionIndex];

        vertex.normal = normals[normalIndex];

        vertex.uv = textureCoordinates[uvIndex];

        outData.mesh.vertices.push_back(
            vertex
        );
    }

    /*
     * This first importer expands all faces
     * into non-indexed triangle vertices.
     */
    outData.mesh.indices.clear();

    if (outData.objectName.empty())
    {
        outData.objectName =
            mbxFilePath.stem().string();
    }

    BOX_LOG_INFO(
        "Imported MBX data: "
        << outData.objectName
        << " Vertices="
        << outData.mesh.vertices.size()
    );

    return outData.mesh.IsValid();

}

std::string mbxManager::BuildMBX(const Entity& entity, const std::string& copiedTextureName) const
{
	// export the mbx file to the specified directory
	// include the textures used in the mbx file to the same directory as the mbx file
    const MeshData& mesh = entity.GetMeshData();

    const Material& material =
        entity.GetMaterial();

    std::ostringstream output;

    output << "# BoxEditor MBX\n"
        << "version "
        << MBX_VERSION_MAJOR
        << ' '
        << MBX_VERSION_MINOR
        << "\n\n";

    output << std::fixed
        << std::setprecision(6);

    

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

            const std::uint32_t materialIndex =
                mesh.vertices[
                    mesh.indices[index]
                ].materialIndex;
            

            // we need to include the vertex.materialIndex = 2;
			// f 1/1/1 2/2/2 3/3/3/ m 2 triangle 1
			// f 1/1/1 3/3/3 4/4/4/ m 2 triangle 2 which makes a face with 4 vertices and 2 triangles
            
            output
                << "f "
                << a << '/' << a << '/' << a << ' '
                << b << '/' << b << '/' << b << ' '
                << c << '/' << c << '/' << c
                << " m "
                << materialIndex
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

            const std::uint32_t materialIndex =
                mesh.vertices[vertex].materialIndex;

            output
                << "f "
                << a << '/' << a << '/' << a << ' '
                << b << '/' << b << '/' << b << ' '
                << c << '/' << c << '/' << c
                << " m "
                << materialIndex
                << '\n';
        }
    }

    output
        << "\nmaterials "
        << entity.GetMaterialSlotCount()
        << '\n';

    for (std::size_t index = 0;
        index < entity.GetMaterialSlotCount();
        ++index)
    {
        const Material& material =
            entity.GetMaterialSlot(index);

        output
            << "\nmaterial "
            << index
            << '\n';

        output
            << "name "
            << MakeSafeName(
                material.GetName()
            )
            << '\n';

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

        if (material.UsesBaseColorTexture())
        {
            const std::filesystem::path texturePath =
                material.GetBaseColorTexturePath();

            if (!texturePath.empty())
            {
                output
                    << "base_color_map "
                    << texturePath.filename().string()
                    << '\n';
            }
        }

        output
            << "endmaterial\n";
    }

    /*output
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
        << "endmaterial\n";*/


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
        BOX_LOG_ERROR("Entity '" << entity.GetName() << "' has texture ID "
            << material.GetBaseColorTexture() << " but no texture source path");

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
        const fs::path absoluteSource = fs::absolute(sourcePath);

        const fs::path absoluteDestination = fs::absolute(destinationPath);

        if (absoluteSource != absoluteDestination)
        {
            fs::copy_file(
                sourcePath,
                destinationPath,
                fs::copy_options::overwrite_existing,
                error
            );

            if (error)
            {
                BOX_LOG_ERROR("Failed to copy texture: " << error.message());

                return false;
            }
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
