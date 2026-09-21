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

	//wright a function to move the texture file to the same directory as the obj file
	// and update the mtl file to point to the new texture file path

	// wright a function to write the mtl file for the obj file, with the correct texture file path


private:	

};
