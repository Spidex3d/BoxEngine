#include "material/MaterialSerializer.h"
#include "material/Material.h"

#include <fstream>
#include <filesystem>

#include <sstream>
#include <string>


bool MaterialSerializer::Save(const Material& material, const std::string& filePath)
{
    namespace fs = std::filesystem;

    const fs::path path(filePath);

    // Create the material directory if needed.
    if (path.has_parent_path())
    {
        std::error_code error;

        fs::create_directories(path.parent_path(), error);

        if (error)
        {
            return false;
        }
    }


    std::ofstream file(path);

    if (!file.is_open())
    {
        return false;
    }


    const glm::vec4 baseColor = material.GetBaseColor();

    const glm::vec3 emissionColor = material.GetEmissionColor();


    // ================================================
    // MiniBox Material Format
    // Version 1
    // ================================================

    file << "MBMAT 1\n";

    file << "Name "
        << material.GetName()
        << '\n';

    file << "Type "
        << static_cast<int>(
            material.GetType()
            )
        << '\n';


    file << "BaseColor "
        << baseColor.r << ' '
        << baseColor.g << ' '
        << baseColor.b << '\n';


    file << "Metallic "
        << material.GetMetallic()
        << '\n';

    file << "Roughness "
        << material.GetRoughness()
        << '\n';

    file << "Alpha "
        << material.GetAlpha()
        << '\n';


    file << "Transmission "
        << material.GetTransmission()
        << '\n';

    file << "IOR "
        << material.GetIOR()
        << '\n';


    file << "EmissionColor "
        << emissionColor.r << ' '
        << emissionColor.g << ' '
        << emissionColor.b << '\n';

    file << "EmissionStrength "
        << material.GetEmissionStrength()
        << '\n';


    file << "BaseColorTexture "
        << (
            material.GetBaseColorTexturePath().empty()
            ? "none"
            : material.GetBaseColorTexturePath()
            )
        << '\n';


    file << "NormalTexture "
        << (
            material.GetNormalTexturePath().empty()
            ? "none"
            : material.GetNormalTexturePath()
            )
        << '\n';

    file << "NormalStrength "
        << material.GetNormalStrength()
        << '\n';


    return file.good();
}


bool MaterialSerializer::Load(
    const std::string& filePath,
    Material& material)
{
    std::ifstream file(filePath);

    if (!file.is_open())
    {
        return false;
    }


    // ================================================
    // Check file header
    // ================================================

    std::string line;

    if (!std::getline(file, line))
    {
        return false;
    }

    if (line != "MBMAT 1")
    {
        return false;
    }


    // ================================================
    // Read material properties
    // ================================================

    while (std::getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }

        std::istringstream stream(line);

        std::string key;

        stream >> key;


        // --------------------------------------------
        // Name
        // --------------------------------------------

        if (key == "Name")
        {
            std::string name;

            std::getline(stream, name);

            // Remove the space left after "Name"
            if (!name.empty() &&
                name.front() == ' ')
            {
                name.erase(
                    name.begin()
                );
            }

            material.SetName(name);
        }


        // --------------------------------------------
        // Material Type
        // --------------------------------------------

        else if (key == "Type")
        {
            int type = 0;

            stream >> type;

            material.SetType(
                static_cast<MaterialType>(
                    type
                    )
            );
        }


        // --------------------------------------------
        // Base Color
        // --------------------------------------------

        else if (key == "BaseColor")
        {
            float r = 1.0f;
            float g = 1.0f;
            float b = 1.0f;

            stream >> r >> g >> b;

            material.SetBaseColor(
                glm::vec4(
                    r,
                    g,
                    b,
                    1.0f
                )
            );
        }


        // --------------------------------------------
        // Metallic
        // --------------------------------------------

        else if (key == "Metallic")
        {
            float value = 0.0f;

            stream >> value;

            material.SetMetallic(value);
        }


        // --------------------------------------------
        // Roughness
        // --------------------------------------------

        else if (key == "Roughness")
        {
            float value = 0.5f;

            stream >> value;

            material.SetRoughness(value);
        }


        // --------------------------------------------
        // Alpha
        // --------------------------------------------

        else if (key == "Alpha")
        {
            float value = 1.0f;

            stream >> value;

            material.SetAlpha(value);
        }


        // --------------------------------------------
        // Transmission
        // --------------------------------------------

        else if (key == "Transmission")
        {
            float value = 0.0f;

            stream >> value;

            material.SetTransmission(value);
        }


        // --------------------------------------------
        // IOR
        // --------------------------------------------

        else if (key == "IOR")
        {
            float value = 1.5f;

            stream >> value;

            material.SetIOR(value);
        }


        // --------------------------------------------
        // Emission Color
        // --------------------------------------------

        else if (key == "EmissionColor")
        {
            float r = 0.0f;
            float g = 0.0f;
            float b = 0.0f;

            stream >> r >> g >> b;

            material.SetEmissionColor(
                glm::vec3(
                    r,
                    g,
                    b
                )
            );
        }


        // --------------------------------------------
        // Emission Strength
        // --------------------------------------------

        else if (key == "EmissionStrength")
        {
            float value = 0.0f;

            stream >> value;

            material.SetEmissionStrength(
                value
            );
        }


        // --------------------------------------------
        // Base Color Texture
        // --------------------------------------------

        else if (key == "BaseColorTexture")
        {
            std::string texturePath;

            std::getline(
                stream,
                texturePath
            );

            if (!texturePath.empty() &&
                texturePath.front() == ' ')
            {
                texturePath.erase(
                    texturePath.begin()
                );
            }

            if (texturePath != "none")
            {
                material.SetBaseColorTexturePath(
                    texturePath
                );
            }
            }

        // --------------------------------------------
        // Normal Texture
        // --------------------------------------------
        // void SetNormalTexture(GLuint textureID, const std::string& sourcePath);
        else if (key == "NormalTexture")
        {
            std::string texturePath;

            std::getline(
                stream,
                texturePath
            );

            if (!texturePath.empty() &&
                texturePath.front() == ' ')
            {
                texturePath.erase(
                    texturePath.begin()
                );
            }

            if (texturePath != "none")
            {
                material.SetNormalTexturePath(
                    texturePath
                );
            }
            }

        // --------------------------------------------
        // Normal Strength
        // --------------------------------------------

        else if (key == "NormalStrength")
        {
            float value = 1.0f;

            stream >> value;

            material.SetNormalStrength(
                value
            );
        }
    }


    return true;
}