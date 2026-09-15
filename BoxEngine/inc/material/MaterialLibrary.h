#pragma once

#include <filesystem>
//#include <vector>
#include "Material.h"


class MaterialLibrary
{
public:

    // Returns the folder name used on disk.
    static std::filesystem::path GetCategoryFolder(MaterialCategory category);

    // Returns the complete material directory
    // Example:
    // assets/materials/glass
    static std::filesystem::path GetCategoryPath(MaterialCategory category);

	

    static std::vector<std::filesystem::path>
        GetMaterials(
            MaterialCategory category
        );

};
