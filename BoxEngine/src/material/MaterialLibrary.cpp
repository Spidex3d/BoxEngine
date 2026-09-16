#include "material/MaterialLibrary.h"
#include <miniBoxLog.h>
#include "Helpers.h"
#include <algorithm>

std::filesystem::path
MaterialLibrary::GetCategoryFolder(MaterialCategory category)
{
    switch (category)
    {
    case MaterialCategory::Glass:
        return "glass";

    case MaterialCategory::Metal:
        return "metal";

    case MaterialCategory::Plastic:
        return "plastic";

    case MaterialCategory::Wood:
        return "wood";

    case MaterialCategory::StoneBrickTile:
        return "stone_brick_tile";

    case MaterialCategory::PlantSoil:
        return "plant_soil";

    case MaterialCategory::Fabric:
        return "fabric";

    case MaterialCategory::Misc:
    default:
        return "misc";
    }
}


std::filesystem::path
MaterialLibrary::GetCategoryPath(MaterialCategory category)
{
    Helpers helpers;

    std::filesystem::path basePath = helpers.GetAssetPath("assets/materials");

    return basePath / GetCategoryFolder(category);
}

//std::vector<std::filesystem::path>
//MaterialLibrary::GetMaterials(
//    MaterialCategory category)
//{
//    namespace fs = std::filesystem;
//
//    std::vector<fs::path> materials;
//
//    const fs::path directory =
//        GetCategoryPath(category);
//
//    if (!fs::exists(directory))
//    {
//        return materials;
//    }
//
//    if (!fs::is_directory(directory))
//    {
//        return materials;
//    }
//
//    for (const auto& entry :
//        fs::directory_iterator(directory))
//    {
//        if (!entry.is_regular_file())
//        {
//            continue;
//        }
//
//        const fs::path& path =
//            entry.path();
//
//        if (path.extension() == ".mbmat")
//        {
//            materials.push_back(path);
//        }
//    }
//
//    std::sort(
//        materials.begin(),
//        materials.end(),
//        [](const fs::path& a,
//            const fs::path& b)
//    {
//        return
//            a.filename().string() <
//            b.filename().string();
//    }
//    );
//
//
//
//    return materials;
//}

std::vector<std::filesystem::path>
MaterialLibrary::GetMaterials(
    MaterialCategory category)
{
    namespace fs = std::filesystem;

    std::vector<fs::path> materials;

    const fs::path directory =
        GetCategoryPath(category);

    if (!fs::exists(directory))
    {
        return materials;
    }

    if (!fs::is_directory(directory))
    {
        return materials;
    }

    for (const auto& entry :
        fs::directory_iterator(directory))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }

        const fs::path path =
            entry.path();

        if (path.extension() == ".mbmat")
        {
            materials.push_back(path);
        }
    }

    std::sort(
        materials.begin(),
        materials.end()
    );

    return materials;
}