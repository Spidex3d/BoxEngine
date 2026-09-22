#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <glm/glm.hpp>	

// version 0.1 22/09/2026 start date


class Entity;

struct OBJMaterialData
{
	
};

struct OBJImportData
{
	int versionMajor = 0;
	int versionMinor = 0;

	std::string objectName;

};

class objManager
{
public:
	objManager() = default;
	~objManager();

	bool ExportOBJ(const Entity& entity, const std::filesystem::path& objFilePath);

	bool ImportOBJ(const std::filesystem::path& objFilePath, OBJImportData& outData);

	
private:	

	bool WriteMTL(const Entity& entity, const std::filesystem::path& mtlFilePath);

};
