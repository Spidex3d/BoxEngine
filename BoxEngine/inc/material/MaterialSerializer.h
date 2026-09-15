#pragma once

#include <string>

class Material;

class MaterialSerializer
{
public:

    static bool Save(
        const Material& material,
        const std::string& filePath
    );

    static bool Load(
        const std::string& filePath,
        Material& material
    );
};
