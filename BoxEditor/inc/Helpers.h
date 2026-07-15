#pragma once
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class Helpers
{
public:

	std::string GetAssetPath(const std::string& relativePath);

	static fs::path FindAssetsRoot();


private:

};