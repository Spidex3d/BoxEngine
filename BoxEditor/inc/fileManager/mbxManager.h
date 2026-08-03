#pragma once
#include <filesystem>
#include <string>

// Export mbx files to a specified directory
// Import mbx files from a specified directory
// version 0.1
class Entity;

class mbxManager
{
public:
	mbxManager() = default;

	mbxManager(const mbxManager&) = delete;

	mbxManager& operator=(const mbxManager&) = delete;

	// Export mbx files to a specified directory assets/models/mbx/cube.mbx
	// we also need to export all the textures used in the mbx file to the same directory as the mbx file.
	bool ExportMBX(const Entity& entity, const std::filesystem::path& mbxFilePath);

	bool ImportMBX(const std::filesystem::path& mbxFilePath); // Import mbx files from a specified directory

private:
	// Build the mbx file then call ExportMBX to export it to a specified directory
	std::string BuildMBX(const Entity& entity, const std::string& copiedTextureName) const;

	bool CopyMaterialTextures(const Entity& entity, const std::filesystem::path& outputDirectory,
		std::string& outBaseColorTextureName
	) const;

	static std::string MakeSafeName(const std::string& name);

};
